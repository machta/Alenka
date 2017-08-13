#include "common.h"
#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <memory>

using namespace std;
using namespace boost::filesystem;

namespace {

path copyToTmp(const string &pathName, const string &sufix) {
  path tmpPath = unique_path(temp_directory_path().string() +
                             "/%%%%_%%%%_%%%%_%%%%." + sufix);
  copy_file(pathName, tmpPath);

  return tmpPath;
}

void testDataModel(const DataModel *dataModel) {
  ASSERT_EQ(dataModel->montageTable()->rowCount(), 2);

  Montage m1 = dataModel->montageTable()->row(0);
  EXPECT_EQ(m1.name, "Montage 0");
  EXPECT_EQ(m1.save, true);

  Montage m2 = dataModel->montageTable()->row(1);
  EXPECT_EQ(m2.name, "Montage 1");
  EXPECT_EQ(m2.save, false);

  const AbstractTrackTable *trackTable =
      dataModel->montageTable()->trackTable(0);
  ASSERT_EQ(trackTable->rowCount(), 2);

  Track t1 = trackTable->row(0);
  EXPECT_EQ(t1.label, "T 0");
  EXPECT_EQ(t1.color[0], 0);
  EXPECT_EQ(t1.color[1], 0);
  EXPECT_EQ(t1.color[2], 0);
  EXPECT_EQ(t1.amplitude, 0.000001);
  EXPECT_EQ(t1.hidden, false);
  EXPECT_EQ(t1.code, "out = in(0) + in(1);");

  Track t2 = trackTable->row(1);
  EXPECT_EQ(t2.label, "T 1");
  EXPECT_EQ(t2.color[0], 0);
  EXPECT_EQ(t2.color[1], 0);
  EXPECT_EQ(t2.color[2], 0);
  EXPECT_EQ(t2.amplitude, 0.000001);
  EXPECT_EQ(t2.hidden, false);
  EXPECT_EQ(t2.code, "out = sum(0, 9);");

  const AbstractEventTable *eventTable =
      dataModel->montageTable()->eventTable(0);
  ASSERT_EQ(eventTable->rowCount(), 2);

  Event e1 = eventTable->row(0);
  EXPECT_EQ(e1.label, "Event 00");
  EXPECT_EQ(e1.type, 0);
  EXPECT_EQ(e1.position, 0);
  EXPECT_EQ(e1.duration, 200);
  EXPECT_EQ(e1.channel, 0);
  EXPECT_EQ(e1.description, "bla bla");

  Event e2 = eventTable->row(1);
  EXPECT_EQ(e2.label, "Event 01");
  EXPECT_EQ(e2.type, 1);
  EXPECT_EQ(e2.position, 2860);
  EXPECT_EQ(e2.duration, 608);
  EXPECT_EQ(e2.channel, -1);
  EXPECT_EQ(e2.description, "");

  trackTable = dataModel->montageTable()->trackTable(1);
  ASSERT_EQ(trackTable->rowCount(), 1);

  Track t3 = trackTable->row(0);
  EXPECT_EQ(t3.label, "T 0");
  EXPECT_EQ(t3.color[0], 0);
  EXPECT_EQ(t3.color[1], 0);
  EXPECT_EQ(t3.color[2], 0);
  EXPECT_EQ(t3.amplitude, 0.00003);
  EXPECT_EQ(t3.hidden, false);
  EXPECT_EQ(t3.code, "out = in(0);");

  ASSERT_EQ(dataModel->eventTypeTable()->rowCount(), 2);

  EventType et1 = dataModel->eventTypeTable()->row(0);
  EXPECT_EQ(et1.id, 1);
  EXPECT_EQ(et1.name, "Type 1");
  EXPECT_EQ(et1.opacity, 0.25);
  EXPECT_EQ(et1.color[0], 255);
  EXPECT_EQ(et1.color[1], 0);
  EXPECT_EQ(et1.color[2], 0);
  EXPECT_EQ(et1.hidden, false);

  EventType et2 = dataModel->eventTypeTable()->row(1);
  EXPECT_EQ(et2.id, 5);
  EXPECT_EQ(et2.name, "Type 5");
  EXPECT_EQ(et2.opacity, 0.5);
  EXPECT_EQ(et2.color[0], 255);
  EXPECT_EQ(et2.color[1], 255);
  EXPECT_EQ(et2.color[2], 0);
  EXPECT_EQ(et2.hidden, false);
}

unique_ptr<DataModel> makeDataModel() {
  auto dataModel = make_unique<DataModel>(make_unique<EventTypeTable>(),
                                          make_unique<MontageTable>());

  dataModel->montageTable()->insertRows(0, 2);

  Montage m1 = dataModel->montageTable()->row(0);
  m1.name = "Montage 0";
  m1.save = true;
  dataModel->montageTable()->row(0, m1);

  Montage m2 = dataModel->montageTable()->row(1);
  m2.name = "Montage 1";
  m2.save = false;
  dataModel->montageTable()->row(1, m2);

  AbstractTrackTable *trackTable = dataModel->montageTable()->trackTable(0);
  trackTable->insertRows(0, 2);

  Track t1 = trackTable->row(0);
  t1.label = "T 0";
  t1.color[0] = t1.color[1] = t1.color[2] = 0;
  t1.amplitude = 0.000001;
  t1.hidden = false;
  t1.code = "out = in(0) + in(1);";
  trackTable->row(0, t1);

  Track t2 = trackTable->row(1);
  t2.label = "T 1";
  t2.color[0] = t2.color[1] = t2.color[2] = 0;
  t2.amplitude = 0.000001;
  t2.hidden = false;
  t2.code = "out = sum(0, 9);";
  trackTable->row(1, t2);

  AbstractEventTable *eventTable = dataModel->montageTable()->eventTable(0);
  eventTable->insertRows(0, 2);

  Event e1 = eventTable->row(0);
  e1.label = "Event 00";
  e1.type = 0;
  e1.position = 0;
  e1.duration = 200;
  e1.channel = 0;
  e1.description = "bla bla";
  eventTable->row(0, e1);

  Event e2 = eventTable->row(1);
  e2.label = "Event 01";
  e2.type = 1;
  e2.position = 2860;
  e2.duration = 608;
  e2.channel = -1;
  eventTable->row(1, e2);

  trackTable = dataModel->montageTable()->trackTable(1);
  trackTable->insertRows(0);

  Track t3 = trackTable->row(0);
  t3.label = "T 0";
  t3.color[0] = t3.color[1] = t3.color[2] = 0;
  t3.amplitude = 0.00003;
  t3.hidden = false;
  t3.code = "out = in(0);";
  trackTable->row(0, t3);

  dataModel->eventTypeTable()->insertRows(0, 2);

  EventType et1 = dataModel->eventTypeTable()->row(0);
  et1.id = 1;
  et1.name = "Type 1";
  et1.opacity = 0.25;
  et1.color[0] = 255;
  et1.color[1] = et1.color[2] = 0;
  et1.hidden = false;
  dataModel->eventTypeTable()->row(0, et1);

  EventType et2 = dataModel->eventTypeTable()->row(1);
  et2.id = 5;
  et2.name = "Type 5";
  et2.opacity = 0.5;
  et2.color[0] = et2.color[1] = 255;
  et2.color[2] = 0;
  et2.hidden = false;
  dataModel->eventTypeTable()->row(1, et2);

  testDataModel(dataModel.get());
  return dataModel;
}

void testMontages(const DataModel *dataModel) {
  EXPECT_EQ(dataModel->montageTable()->rowCount(), 1);

  Montage m1 = dataModel->montageTable()->row(0);
  EXPECT_EQ(m1.name, "Recording Montage");
  EXPECT_EQ(m1.save, false);

  const AbstractTrackTable *trackTable =
      dataModel->montageTable()->trackTable(0);
  const int trackCount = 19;
  ASSERT_EQ(trackTable->rowCount(), trackCount);

  vector<string> labels{"Fp1", "Fp2", "F3", "F4", "C3", "C4", "P3",
                        "P4",  "O1",  "O2", "F7", "F8", "T3", "T4",
                        "T5",  "T6",  "Fz", "Cz", "Pz"};

  for (int i = 0; i < trackCount; ++i) {
    Track t = trackTable->row(i);
    EXPECT_EQ(t.label.substr(0, labels[i].size()), labels[i]);
    EXPECT_EQ(t.color[0], 0);
    EXPECT_EQ(t.color[1], 0);
    EXPECT_EQ(t.color[2], 0);
    EXPECT_EQ(t.amplitude, 1);
    EXPECT_EQ(t.hidden, false);
    EXPECT_EQ(t.code, "out = in(INDEX);");
  }
}

void testEvents(const DataModel *dataModel, bool edf) {
  const AbstractEventTable *eventTable =
      dataModel->montageTable()->eventTable(0);
  ASSERT_EQ(eventTable->rowCount(), 2);

  Event e1 = eventTable->row(0);
  EXPECT_EQ(e1.type, 0);
  EXPECT_EQ(e1.position, 0);
  EXPECT_EQ(e1.duration, 200);
  EXPECT_EQ(e1.channel, 0);
  if (edf) {
    EXPECT_EQ(e1.label, "Event 00");
    EXPECT_EQ(e1.description, "bla bla");
  } else {
    EXPECT_EQ(e1.label, "Event 0");
    EXPECT_EQ(e1.description, "");
  }

  Event e2 = eventTable->row(1);
  EXPECT_EQ(e2.type, 1);
  EXPECT_EQ(e2.duration, 608);
  EXPECT_EQ(e2.position, 2860);
  EXPECT_EQ(e2.channel, -1);
  EXPECT_EQ(e2.description, "");
  if (edf) {
    EXPECT_EQ(e2.label, "Event 01");
  } else {
    EXPECT_EQ(e2.label, "Event 1");
  }
}

void testTypes(const DataModel *dataModel) {
  ASSERT_EQ(dataModel->eventTypeTable()->rowCount(), 2);

  EventType et1 = dataModel->eventTypeTable()->row(0);
  EXPECT_EQ(et1.id, 1);
  EXPECT_EQ(et1.name, "Type 1");
  EXPECT_EQ(et1.opacity, 0.25);
  EXPECT_EQ(et1.color[0], 255);
  EXPECT_EQ(et1.color[1], 0);
  EXPECT_EQ(et1.color[2], 0);
  EXPECT_EQ(et1.hidden, false);

  EventType et2 = dataModel->eventTypeTable()->row(1);
  EXPECT_EQ(et2.id, 5);
  EXPECT_EQ(et2.name, "Type 5");
  EXPECT_EQ(et2.opacity, 0.25);
  EXPECT_EQ(et2.color[0], 255);
  EXPECT_EQ(et2.color[1], 0);
  EXPECT_EQ(et2.color[2], 0);
  EXPECT_EQ(et2.hidden, false);
}

template <class T> void testMontFile(const string &fp, const string &suffix) {
  path p = copyToTmp(fp, suffix);

  {
    T file(p.string());

    unique_ptr<DataModel> dataModel = makeDataModel();
    file.setDataModel(dataModel.get());

    file.save();
  }

  {
    DataModel dataModel(make_unique<EventTypeTable>(),
                        make_unique<MontageTable>());

    T file(p.string());
    file.setDataModel(&dataModel);

    file.load();
    testDataModel(&dataModel);
  }

  remove(p);
  remove(p.string() + ".mont");
  remove(p.string() + ".backup");
}

template <class T>
unique_ptr<DataModel> testPrimary(const string &fp, const string &suffix) {
  path p = copyToTmp(fp, suffix);

  {
    T file(p.string());

    unique_ptr<DataModel> dataModel = makeDataModel();
    file.setDataModel(dataModel.get());

    file.save();
  }

  remove(p.string() + ".mont");
  auto dataModel = make_unique<DataModel>(make_unique<EventTypeTable>(),
                                          make_unique<MontageTable>());
  {
    T file(p.string());
    file.setDataModel(dataModel.get());

    file.load();
  }

  remove(p);
  remove(p.string() + ".backup");
  return dataModel;
}

} // namespace

TEST(data_model_test, test_mont_GDF2) {
  testMontFile<GDF2>(TEST_DATA_PATH + "gdf/gdf00.gdf", "gdf");
  testMontFile<GDF2>(TEST_DATA_PATH + "gdf/gdf01.gdf", "gdf");
}

TEST(data_model_test, test_mont_EDF) {
  testMontFile<EDF>(TEST_DATA_PATH + "edf/edf00.edf", "edf");
}

TEST(data_model_test, test_primary_GDF200) {
  unique_ptr<DataModel> dataModel =
      testPrimary<GDF2>(TEST_DATA_PATH + "gdf/gdf00.gdf", "gdf");

  testMontages(dataModel.get());
  testEvents(dataModel.get(), false);
  testTypes(dataModel.get());
}

TEST(data_model_test, test_primary_GDF201) {
  unique_ptr<DataModel> dataModel =
      testPrimary<GDF2>(TEST_DATA_PATH + "gdf/gdf01.gdf", "gdf");

  testEvents(dataModel.get(), false);
  testTypes(dataModel.get());
}

TEST(data_model_test, test_primary_EDF) {
  unique_ptr<DataModel> dataModel =
      testPrimary<EDF>(TEST_DATA_PATH + "edf/edf00.edf", "edf");

  testEvents(dataModel.get(), true);
}
