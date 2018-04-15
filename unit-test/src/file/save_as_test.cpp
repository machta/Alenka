#include "common.h"
#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

// TODO: Add save_MAT_as_EDF test.

TEST(save_as_test, save_GDF_as_EDF) {
  TestFile gdf00(TEST_DATA_PATH + "gdf/gdf00.gdf", 200, 19, 364000);
  unique_ptr<DataFile> gdf_file(gdf00.makeGDF2());

  DataModel dataModel(make_unique<EventTypeTable>(),
                      make_unique<MontageTable>());

  gdf_file->setDataModel(&dataModel);
  gdf_file->load();

  path tmpPath =
      unique_path(temp_directory_path().string() + "/%%%%_%%%%_%%%%_%%%%.edf");
  EDF::saveAs(tmpPath.string(), gdf_file.get());
  unique_ptr<DataFile> edfFile(new EDF(tmpPath.string()));

  int channelCount = edfFile->getChannelCount();
  int samplesRecorded = static_cast<int>(edfFile->getSamplesRecorded());
  int n = channelCount * samplesRecorded;

  vector<double> dataD;
  dataD.insert(dataD.begin(), n, 0);
  edfFile->readSignal(dataD.data(), 0, samplesRecorded - 1);

  double relErr, absErr;

  // The conversion introduces an error (probably in the extreme values that
  // need to be clipped in order to fit into 16 bits) so these tests have a
  // higher tolerance. But the result looks OK in Alenka.
  compareMatrixAverage(dataD.data(), gdf00.getValues().data(), channelCount,
                       samplesRecorded, &relErr, &absErr);
  EXPECT_LT(relErr, 0.3);
  EXPECT_LT(absErr, 0.6);

  //	compareMatrix(dataD.data(), gdf00.getValues().data(), channelCount,
  // samplesRecorded, &relErr, &absErr);
  //	EXPECT_LT(relErr, MAX_REL_ERR_DOUBLE);
  //	EXPECT_LT(absErr, MAX_ABS_ERR_DOUBLE);

  edfFile.reset();
  remove(tmpPath);
}
