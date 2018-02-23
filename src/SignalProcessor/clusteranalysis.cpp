#include "clusteranalysis.h"

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

#include <algorithm>

using namespace std;
using namespace AlenkaSignal;
using namespace AlenkaFile;

namespace {

double resolvePosition(int channels, int index, vector<double> *positions) {
  double res = 0;
  bool empty = true;

  for (int i = 0; i < channels; ++i) {
    double pos = positions[i][index];

    if (isnan(pos))
      continue;

    if (empty) {
      empty = false;
      res = pos;
    } else {
      res = min(res, pos);
    }
  }

  return res;
}

} // namespace

void ClusterAnalysis::runAnalysis(OpenDataFile *file, QWidget * /*parent*/) {
  if (!cluster)
    cluster = make_unique<Cluster>();

  if (!discharges || !discharges->m_MA || !discharges->m_MW ||
      discharges->m_MA->size() == 0)
    return;

  int channels = discharges->GetCountChannels();
  int n = static_cast<int>(discharges->m_MA->size());

  vector<double> MA, MW;
  MA.reserve(n * channels);
  MW.reserve(n * channels);

  for (int i = 0; i < channels; ++i) {
    for (int j = 0; j < n; ++j) {
      MA.push_back(discharges->m_MA[i][j]);
      MW.push_back(discharges->m_MW[i][j]);
    }
  }

  cluster->process(n, channels, MA, MW, pcaCentering());
  auto classes = cluster->getClass();
  assert(static_cast<int>(classes.size()) == n);
  int nClasses = *max_element(classes.begin(), classes.end()) + 1;

  file->undoFactory->beginMacro("run Cluster");

  auto tt = file->dataModel->eventTypeTable();
  int typeIndex = tt->rowCount();
  file->undoFactory->insertEventType(typeIndex, nClasses);

  for (int i = 0; i < nClasses; ++i) {
    EventType et = tt->row(typeIndex + i);
    et.name = "Cluster Class " + to_string(i);
    file->undoFactory->changeEventType(typeIndex + i, et);
  }

  int selectedMontage = OpenDataFile::infoTable.getSelectedMontage();
  auto et = file->dataModel->montageTable()->eventTable(selectedMontage);
  int eventIndex = et->rowCount();
  file->undoFactory->insertEvent(selectedMontage, eventIndex, n);
  int fs = file->file->getSamplingFrequency();

  for (int i = 0; i < n; ++i) {
    Event e = et->row(eventIndex + i);

    e.type = typeIndex + classes[i];
    e.position = resolvePosition(channels, i, discharges->m_MP) * fs;
    e.duration = spikeDuration * fs;
    // e.duration = discharges->m_MD[0][i] * fs;
    e.label = "Cluster " + to_string(i);
    e.channel = -1;

    file->undoFactory->changeEvent(selectedMontage, eventIndex + i, e);
  }

  file->undoFactory->endMacro();
}
