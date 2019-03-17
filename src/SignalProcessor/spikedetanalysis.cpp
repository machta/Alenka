#include "spikedetanalysis.h"

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../../Alenka-Signal/include/AlenkaSignal/montageprocessor.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"
#include "../myapplication.h"
#include "../signalfilebrowserwindow.h"
#include "../spikedetsettingsdialog.h"
#include "signalprocessor.h"

#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

#include <chrono>
#include <thread>

using namespace std;
using namespace AlenkaSignal;

namespace {

template <class T> class Loader : public AbstractSpikedetLoader<T> {
  const int BLOCK_LENGTH = 8 * 1024;

  AlenkaFile::DataFile *file;
  const vector<unique_ptr<Montage<T>>> &montage;
  vector<T> tmpData;
  int inChannels, outChannels;
  unique_ptr<MontageProcessor<T>> processor;
  cl_command_queue queue = nullptr;
  cl_mem inBuffer = nullptr, outBuffer = nullptr, xyzBuffer = nullptr;

public:
  Loader(AlenkaFile::DataFile *file,
         const vector<unique_ptr<Montage<T>>> &montage, OpenCLContext *context)
      : file(file), montage(montage), inChannels(file->getChannelCount()),
        outChannels(static_cast<int>(montage.size())),
        processor(new MontageProcessor<T>(BLOCK_LENGTH, inChannels)) {
    cl_int err;
    cl_mem_flags flags = CL_MEM_READ_WRITE;

    queue = clCreateCommandQueue(context->getCLContext(),
                                 context->getCLDevice(), 0, &err);
    checkClErrorCode(err, "clCreateCommandQueue");

    inBuffer =
        clCreateBuffer(context->getCLContext(), flags,
                       BLOCK_LENGTH * inChannels * sizeof(T), nullptr, &err);
    checkClErrorCode(err, "clCreateBuffer");

    outBuffer =
        clCreateBuffer(context->getCLContext(), flags,
                       BLOCK_LENGTH * outChannels * sizeof(T), nullptr, &err);
    checkClErrorCode(err, "clCreateBuffer");

    xyzBuffer = clCreateBuffer(context->getCLContext(), flags,
                               inChannels * 3 * sizeof(T), nullptr, &err);
    checkClErrorCode(err, "clCreateBuffer");

    auto montageTable = file->getDataModel()->montageTable();
    assert(0 < montageTable->rowCount());
    auto defaultTrackTable = montageTable->trackTable(0);

    SignalProcessor::updateXyzBuffer(queue, xyzBuffer, defaultTrackTable);

    tmpData.resize(BLOCK_LENGTH * inChannels);
  }

  ~Loader() override {
    cl_int err;

    if (queue) {
      err = clReleaseCommandQueue(queue);
      checkClErrorCode(err, "clReleaseCommandQueue()");
    }

    if (inBuffer) {
      err = clReleaseMemObject(inBuffer);
      checkClErrorCode(err, "clReleaseMemObject()");
    }

    if (outBuffer) {
      err = clReleaseMemObject(outBuffer);
      checkClErrorCode(err, "clReleaseMemObject()");
    }

    if (xyzBuffer) {
      err = clReleaseMemObject(xyzBuffer);
      checkClErrorCode(err, "clReleaseMemObject()");
    }
  }

  void readSignal(T *data, int64_t firstSample, int64_t lastSample) override {
    cl_int err;

    for (int64_t sample = firstSample; sample <= lastSample;
         sample += BLOCK_LENGTH) {
      // OpenCLContext::printBuffer("spikedet_loader.txt", data, (lastSample -
      // firstSample + 1)*outChannels);
      int len = min<int>(BLOCK_LENGTH, lastSample - sample + 1);
      assert(len >= 1);

      file->readSignal(tmpData.data(), sample, sample + len - 1);

      size_t origin[] = {0, 0, 0};
      size_t rowLen = len * sizeof(T);
      size_t inRegion[] = {rowLen, static_cast<size_t>(inChannels), 1};

      err = clEnqueueWriteBufferRect(queue, inBuffer, CL_TRUE, origin, origin,
                                     inRegion, BLOCK_LENGTH * sizeof(T), 0, 0,
                                     0, tmpData.data(), 0, nullptr, nullptr);
      checkClErrorCode(err, "clEnqueueWriteBufferRect()");

      processor->process(montage.begin(), montage.end(), inBuffer, outBuffer,
                         xyzBuffer, queue, BLOCK_LENGTH);
      // OpenCLContext::printBuffer("after_process.txt", outBuffer, queue);

      size_t outRegion[] = {rowLen, static_cast<size_t>(outChannels), 1};
      size_t dataOrigin[] = {
          static_cast<size_t>((sample - firstSample) * sizeof(T)), 0, 0};

      err =
          clEnqueueReadBufferRect(queue, outBuffer, CL_TRUE, origin, dataOrigin,
                                  outRegion, BLOCK_LENGTH * sizeof(T), 0,
                                  (lastSample - firstSample + 1) * sizeof(T), 0,
                                  data, 0, nullptr, nullptr);
      checkClErrorCode(err, "clEnqueueReadBufferRect()");
    }
    // OpenCLContext::printBuffer("spikedet_loader_done.txt", data, (lastSample
    // - firstSample + 1)*outChannels);
  }

  int64_t sampleCount() override { return file->getSamplesRecorded(); }
  int channelCount() override { return outChannels; }
};

template <class T>
auto makeMontage(OpenDataFile *file, OpenCLContext *context) {
  const AlenkaFile::AbstractTrackTable *trackTable =
      file->dataModel->montageTable()->trackTable(
          OpenDataFile::infoTable.getSelectedMontage());

  vector<pair<string, cl_int>> montageCode;
  for (int i = 0; i < trackTable->rowCount(); ++i)
    montageCode.emplace_back(trackTable->row(i).code, i);

  auto labels = SignalProcessor::collectLabels(
      file->file->getDataModel()->montageTable()->trackTable(0));
  string header =
      OpenDataFile::infoTable.getGlobalMontageHeader().toStdString();
  return SignalProcessor::makeMontage<T>(montageCode, context, header, labels);
}

void processOutput(OpenDataFile *file, SpikedetAnalysis *spikedetAnalysis,
                   double spikeDuration) {
  using namespace AlenkaFile;

  // Add three new event types for the different levels of spike events.
  file->undoFactory->beginMacro("run Spikedet");

  const AbstractEventTypeTable *eventTypeTable =
      file->dataModel->eventTypeTable();
  int index = eventTypeTable->rowCount();
  file->undoFactory->insertEventType(index, 3);

  QColor colors[3] = {QColor(0, 0, 255), QColor(0, 255, 0),
                      QColor(0, 255, 255)};
  for (int i = 0; i < 3; ++i) {
    EventType et = eventTypeTable->row(index + i);

    et.name = "Spikedet K" + to_string(i + 1);
    et.color = DataModel::color2colorArray(colors[i]);

    file->undoFactory->changeEventType(index + i, et);
  }

  // Process the output structure.
  const AbstractEventTable *eventTable =
      file->dataModel->montageTable()->eventTable(
          OpenDataFile::infoTable.getSelectedMontage());

  CDetectorOutput *out = spikedetAnalysis->getOutput();
  assert(out);
  int count = static_cast<int>(out->m_pos.size());

  if (count > 0) {
    assert(static_cast<int>(out->m_chan.size()) == count);

    int etIndex = eventTable->rowCount();
    file->undoFactory->insertEvent(OpenDataFile::infoTable.getSelectedMontage(),
                                   etIndex, count);

    for (int i = 0; i < count; ++i) {
      Event e = eventTable->row(etIndex + i);

      e.label = "Spike " + to_string(i);
      e.type =
          index +
          (out->m_con[i] == 0.5
               ? 1
               : 0); // TODO: Figure out what should be used as the third type.
      e.position = out->m_pos[i] * file->file->getSamplingFrequency();
      e.duration = file->file->getSamplingFrequency() * spikeDuration;
      // e.duration = out->m_dur[i]*file->getSamplingFrequency();
      e.channel = out->m_chan[i] - 1;

      file->undoFactory->changeEvent(
          OpenDataFile::infoTable.getSelectedMontage(), etIndex + i, e);
    }
  }

  file->undoFactory->endMacro();
}

const int SLEEP_FOR_MS = 1;

} // namespace

void SpikedetAnalysis::runAnalysis(OpenDataFile *file, QWidget *parent) {
  assert(file);

  QProgressDialog progress("Running Spikedet analysis", "Abort", 0, 100,
                           parent);
  progress.setWindowModality(Qt::WindowModal);

  progress.setMinimumDuration(0); // This is to show the dialog immediately.
  progress.setValue(1);

  OpenCLContext *const context = globalContext.get();

  vector<unique_ptr<Montage<SIGNALTYPE>>> montage =
      makeMontage<SIGNALTYPE>(file, context);

  int Fs = static_cast<int>(round(file->file->getSamplingFrequency()));
  Spikedet spikedet(Fs, originalSpikedet(), settings);
  Loader<SIGNALTYPE> loader(file->file, montage, context);

  output = make_unique<CDetectorOutput>();
  discharges = make_unique<CDischarges>(loader.channelCount());

  thread t(
      [&]() { spikedet.runAnalysis(&loader, output.get(), discharges.get()); });

  while (1) {
    int percentage = spikedet.progressPercentage();
    progress.setValue(percentage);

    if (progress.wasCanceled()) {
      spikedet.cancel();
      break;
    }

    if (percentage == 100)
      break;

    this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_MS));
  }

  t.join();
  progress.setValue(100);

  processOutput(file, this, spikeDuration);
}

void SpikedetAnalysis::analyseCommandLineFile() {
  unique_ptr<AlenkaFile::DataFile> file;

  try {
    vector<string> fileNames;
    programOption("filename", fileNames);

    if (1 != fileNames.size())
      runtime_error("You must specify at least 1 input file");

    const QString fileName = QString::fromStdString(fileNames[0]);
    const vector<string> rest(fileNames.begin() + 1, fileNames.end());
    file = SignalFileBrowserWindow::dataFileBySuffix(fileName, rest);
  } catch (const runtime_error &e) {
    cerr << "Error while opening file: " << catchDetailed(e) << endl;
    MyApplication::mainExit(EXIT_FAILURE);
  }

  int Fs = static_cast<int>(round(file->getSamplingFrequency()));
  auto settings = Spikedet::defaultSettings();
  SpikedetSettingsDialog::resetSettings(&settings, nullptr);

  bool useOriginalDecimation;
  programOption("osd", useOriginalDecimation);

  Spikedet spikedet(Fs, useOriginalDecimation, settings);
  FileSpikedetLoader<SIGNALTYPE> loader(file.get());

  auto output = make_unique<CDetectorOutput>();
  auto discharges = make_unique<CDischarges>(loader.channelCount());

  thread t(
      [&]() { spikedet.runAnalysis(&loader, output.get(), discharges.get()); });

  int lastPercentage = -1;

  while (1) {
    int percentage = spikedet.progressPercentage();

    if (percentage < 100 && lastPercentage < percentage) {
      fprintf(stderr, "progress: %3d%%\n", percentage);
      lastPercentage = percentage;
    }

    if (percentage == 100)
      break;

    this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_MS));
  }

  t.join();

  string fileName;
  programOption("spikedet", fileName);

  CResultsModel::SaveResultsMAT(fileName.c_str(), output.get(),
                                discharges.get());
}
