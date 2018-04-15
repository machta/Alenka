#include "../include/AlenkaFile/datafile.h"

#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <type_traits>

#include <detailedexception.h>

using namespace std;
using namespace pugi;
using namespace AlenkaFile;

namespace {

template <typename T>
void fillWithZeroes(vector<T *> &dataChannels, uint64_t n) {
  for (auto &e : dataChannels) {
    for (uint64_t i = 0; i < n; ++i) {
      *e = 0;
      e++;
    }
  }
}

template <typename T>
void readSignalFloatDouble(DataFile *file, T *data, int64_t firstSample,
                           int64_t lastSample) {
  if (lastSample < firstSample)
    throwDetailed(invalid_argument(
        "'lastSample' must be greater than or equal to 'firstSample'"));

  int64_t len = lastSample - firstSample + 1;
  vector<T *> dataChannels(file->getChannelCount());
  for (unsigned int i = 0; i < file->getChannelCount(); ++i)
    dataChannels[i] = data + i * len;

  if (firstSample < 0) {
    fillWithZeroes(dataChannels, min(-firstSample, len));
    firstSample = 0;
  }

  int64_t lastInFile = min<int64_t>(file->getSamplesRecorded() - 1, lastSample);

  if (firstSample <= lastInFile) {
    file->readChannels(dataChannels, firstSample, lastInFile);

    for (auto &e : dataChannels)
      e += lastInFile - firstSample + 1;
  }

  if (lastInFile < lastSample)
    fillWithZeroes(dataChannels, min(lastSample - lastInFile, len));

#ifndef NDEBUG
  for (unsigned int i = 0; i < file->getChannelCount(); ++i)
    assert(dataChannels.at(i) == data + (i + 1) * len &&
           "Make sure that precisely the required number of samples was read.");
#endif
}

void appendTrack(xml_node node, const Track &t) {
  xml_node track = node.append_child("track");

  track.append_attribute("label").set_value(t.label.c_str());
  track.append_child("code")
      .append_child(node_pcdata)
      .set_value(t.code.c_str());
  track.append_attribute("color").set_value(
      DataModel::colorArray2str(t.color).c_str());
  track.append_attribute("amplitude").set_value(t.amplitude);
  track.append_attribute("hidden").set_value(t.hidden);
  track.append_attribute("x").set_value(t.x);
  track.append_attribute("y").set_value(t.y);
  track.append_attribute("z").set_value(t.z);
}

void appendTrackTable(xml_node node, AbstractTrackTable *tt) {
  xml_node trackTable = node.append_child("trackTable");
  for (int i = 0; i < tt->rowCount(); ++i)
    appendTrack(trackTable, tt->row(i));
}

void appendEvent(xml_node node, const Event &e) {
  xml_node event = node.append_child("event");

  event.append_attribute("label").set_value(e.label.c_str());
  event.append_attribute("type").set_value(e.type);
  event.append_attribute("position").set_value(e.position);
  event.append_attribute("duration").set_value(e.duration);
  event.append_attribute("channel").set_value(e.channel);
  event.append_child("description")
      .append_child(node_pcdata)
      .set_value(e.description.c_str());
}

void appendEventTable(xml_node node, AbstractEventTable *et) {
  xml_node eventTable = node.append_child("eventTable");
  for (int i = 0; i < et->rowCount(); ++i)
    appendEvent(eventTable, et->row(i));
}

void appendEventType(xml_node node, const EventType &et) {
  xml_node eventType = node.append_child("eventType");

  eventType.append_attribute("id").set_value(et.id);
  eventType.append_attribute("name").set_value(et.name.c_str());
  eventType.append_attribute("opacity").set_value(et.opacity);
  eventType.append_attribute("color").set_value(
      DataModel::colorArray2str(et.color).c_str());
  eventType.append_attribute("hidden").set_value(et.hidden);
}

void buildXML(xml_document &xml, DataModel *dataModel) {
  xml_node document = xml.append_child("document");

  xml_node montageTable = document.append_child("montageTable");
  for (int i = 0; i < dataModel->montageTable()->rowCount(); ++i) {
    Montage m = dataModel->montageTable()->row(i);
    xml_node montage = montageTable.append_child("montage");

    xml_attribute name = montage.append_attribute("name");
    name.set_value(m.name.c_str());

    xml_attribute save = montage.append_attribute("save");
    save.set_value(m.save);

    appendTrackTable(montage, dataModel->montageTable()->trackTable(i));
    appendEventTable(montage, dataModel->montageTable()->eventTable(i));
  }

  xml_node eventTypeTable = document.append_child("eventTypeTable");
  for (int i = 0; i < dataModel->eventTypeTable()->rowCount(); ++i)
    appendEventType(eventTypeTable, dataModel->eventTypeTable()->row(i));
}

void loadTrack(xml_node node, AbstractTrackTable *tt) {
  if (node) {
    int last = tt->rowCount();
    tt->insertRows(last);
    Track t = tt->row(last);

    t.label = node.attribute("label").as_string();
    t.code = node.child("code").text().as_string();
    t.color = DataModel::str2colorArray(node.attribute("color").as_string());
    t.amplitude = node.attribute("amplitude").as_double();
    t.hidden = node.attribute("hidden").as_bool();
    t.x = node.attribute("x").as_float();
    t.y = node.attribute("y").as_float();
    t.z = node.attribute("z").as_float();

    tt->row(last, t);
  }
}

void loadTrackTable(xml_node node, AbstractTrackTable *tt) {
  if (node) {
    xml_node track = node.child("track");

    while (track) {
      loadTrack(track, tt);
      track = track.next_sibling("track");
    }
  }
}

void loadEvent(xml_node node, AbstractEventTable *et) {
  if (node) {
    int last = et->rowCount();
    et->insertRows(last);
    Event e = et->row(last);

    e.label = node.attribute("label").as_string();
    e.type = node.attribute("type").as_int();
    e.position = node.attribute("position").as_int();
    e.duration = node.attribute("duration").as_int();
    e.channel = node.attribute("channel").as_int();
    e.description = node.child("description").text().as_string();

    et->row(last, e);
  }
}

void loadEventTable(xml_node node, AbstractEventTable *et) {
  if (node) {
    xml_node event = node.child("event");

    while (event) {
      loadEvent(event, et);
      event = event.next_sibling("event");
    }
  }
}

void loadMontage(xml_node node, AbstractMontageTable *mt) {
  if (node) {
    int last = mt->rowCount();
    mt->insertRows(last);
    Montage m = mt->row(last);

    m.name = node.attribute("name").as_string();
    m.save = node.attribute("save").as_bool();

    mt->row(last, m);

    loadTrackTable(node.child("trackTable"), mt->trackTable(last));
    loadEventTable(node.child("eventTable"), mt->eventTable(last));
  }
}

void loadMontageTable(xml_node node, AbstractMontageTable *mt) {
  if (node) {
    xml_node montage = node.child("montage");

    while (montage) {
      loadMontage(montage, mt);
      montage = montage.next_sibling("montage");
    }
  }
}

void loadEventType(xml_node node, AbstractEventTypeTable *ett) {
  if (node) {
    int last = ett->rowCount();
    ett->insertRows(last);
    EventType et = ett->row(last);

    et.id = node.attribute("id").as_int();
    et.name = node.attribute("name").as_string();
    et.opacity = node.attribute("opacity").as_float();
    et.color = DataModel::str2colorArray(node.attribute("color").as_string());
    et.hidden = node.attribute("hidden").as_bool();

    ett->row(last, et);
  }
}

void loadEventTypeTable(xml_node node, AbstractEventTypeTable *ett) {
  if (node) {
    xml_node eventType = node.child("eventType");

    while (eventType) {
      loadEventType(eventType, ett);
      eventType = eventType.next_sibling("eventType");
    }
  }
}

void loadXML(xml_node node, DataModel *dataModel) {
  if (node) {
    loadMontageTable(node.child("montageTable"), dataModel->montageTable());
    loadEventTypeTable(node.child("eventTypeTable"),
                       dataModel->eventTypeTable());
  }
}

} // namespace

namespace AlenkaFile {

void DataFile::saveSecondaryFile(string montFilePath) {
  if (montFilePath == "")
    montFilePath = filePath + ".mont";

  xml_document doc;
  buildXML(doc, dataModel);

  if (!doc.save_file(montFilePath.c_str()))
    throwDetailed(runtime_error("Error writing " + montFilePath));
}

bool DataFile::loadSecondaryFile(string montFilePath) {
  if (montFilePath == "")
    montFilePath = filePath + ".mont";

  xml_document doc;
  xml_parse_result res = doc.load_file(montFilePath.c_str());

  if (res)
    loadXML(doc.child("document"), dataModel);

  return res;
}

void DataFile::readSignal(float *data, int64_t firstSample,
                          int64_t lastSample) {
  readSignalFloatDouble(this, data, firstSample, lastSample);
}

void DataFile::readSignal(double *data, int64_t firstSample,
                          int64_t lastSample) {
  readSignalFloatDouble(this, data, firstSample, lastSample);
}

double DataFile::getPhysicalMaximum(unsigned int channel) {
  if (physicalMax.empty())
    computePhysicalMinMax();

  return physicalMax[channel];
}

double DataFile::getPhysicalMinimum(unsigned int channel) {
  if (physicalMin.empty())
    computePhysicalMinMax();

  return physicalMin[channel];
}

vector<string> DataFile::getLabels() {
  std::vector<std::string> labels;
  labels.reserve(getChannelCount());

  for (unsigned int i = 0; i < getChannelCount(); ++i) {
    string l = getLabel(i);
    boost::trim(l);
    labels.push_back(l);
  }

  return labels;
}

void DataFile::fillDefaultMontage(int index) {
  assert(0 < getChannelCount());

  AbstractMontageTable *mt = getDataModel()->montageTable();
  Montage montage = mt->row(0);
  montage.name = "Recording Montage";
  mt->row(0, montage);

  AbstractTrackTable *tt = mt->trackTable(index);
  tt->insertRows(index, getChannelCount());

  auto labels = getLabels();
  assert(labels.size() == getChannelCount());

  for (unsigned int i = 0; i < getChannelCount(); ++i) {
    Track t = tt->row(i);
    t.label = labels[i];
    tt->row(i, t);
  }
}

void DataFile::computePhysicalMinMax() {
  assert(physicalMax.empty());
  assert(physicalMin.empty());

  const int channels = getChannelCount();
  physicalMax.reserve(channels);
  physicalMin.reserve(channels);

  for (int i = 0; i < channels; ++i) {
    physicalMax.push_back(getDigitalMinimum(i));
    physicalMin.push_back(getDigitalMaximum(i));
  }

  const int chunkSize = static_cast<int>(getSamplingFrequency());
  vector<double> tmpData(chunkSize * channels);
  const auto sampleCount = static_cast<int64_t>(getSamplesRecorded());

  for (int64_t i = 0; i < sampleCount; i += chunkSize) {
    int64_t to = i + chunkSize - 1;
    readSignal(tmpData.data(), i, to);

    int size = static_cast<int>(min<int64_t>(chunkSize, sampleCount - i));

    for (int j = 0; j < channels; ++j) {
      auto itB = tmpData.begin();
      advance(itB, j * size);

      auto itE = itB;
      advance(itE, size);

      auto minMax = minmax_element(itB, itE);

      if (*minMax.first < physicalMin[j])
        physicalMin[j] = *minMax.first;

      if (physicalMax[j] < *minMax.second)
        physicalMax[j] = *minMax.second;
    }
  }

  for (int i = 0; i < channels; ++i) {
    physicalMax[i] = min(physicalMax[i], getDigitalMaximum(i));
    physicalMin[i] = max(physicalMin[i], getDigitalMinimum(i));
  }
}

double DataFile::INVALID_DATE = -1000'1000'1000;

} // namespace AlenkaFile
