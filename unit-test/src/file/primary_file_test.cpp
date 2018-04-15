#include "common.h"
#include <gtest/gtest.h>

namespace {

void metaInfoTest(DataFile *file, TestFile *testFile) {
  EXPECT_DOUBLE_EQ(file->getSamplingFrequency(), testFile->sampleRate);
  EXPECT_EQ(file->getChannelCount(), testFile->channelCount);
  EXPECT_EQ(file->getSamplesRecorded(), testFile->samplesRecorded);
}

void outOfBoundsTest(DataFile *file, int nNormal, int nZero) {
  vector<double> a((nNormal + nZero) * file->getChannelCount());
  vector<double> b(nNormal * file->getChannelCount());
  vector<double> zero(nZero * file->getChannelCount(), 0);

  // Read partially before.
  fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));
  fillVector(b, static_cast<double>(0xBBBBBBBBBBBBBBBB));

  file->readSignal(a.data(), -nZero, nNormal - 1);
  file->readSignal(b.data(), 0, nNormal - 1);

  double relErr, absErr;

  compareMatrix(a.data(), zero.data(), file->getChannelCount(), nZero,
                nNormal + nZero, nZero, &relErr, &absErr);
  EXPECT_DOUBLE_EQ(relErr, 0);
  EXPECT_DOUBLE_EQ(absErr, 0);

  compareMatrix(a.data() + nZero, b.data(), file->getChannelCount(), nNormal,
                nNormal + nZero, nNormal, &relErr, &absErr);
  EXPECT_DOUBLE_EQ(relErr, 0);
  EXPECT_DOUBLE_EQ(absErr, 0);

  // Read partially after.
  fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));
  fillVector(b, static_cast<double>(0xBBBBBBBBBBBBBBBB));

  size_t last = file->getSamplesRecorded() - 1;
  file->readSignal(a.data(), last - nNormal + 1, last + nZero);
  file->readSignal(b.data(), last - nNormal + 1, last);

  compareMatrix(a.data() + nNormal, zero.data(), file->getChannelCount(), nZero,
                nNormal + nZero, nZero, &relErr, &absErr);
  EXPECT_DOUBLE_EQ(relErr, 0);
  EXPECT_DOUBLE_EQ(absErr, 0);

  compareMatrix(a.data(), b.data(), file->getChannelCount(), nNormal,
                nNormal + nZero, nNormal, &relErr, &absErr);
  EXPECT_DOUBLE_EQ(relErr, 0);
  EXPECT_DOUBLE_EQ(absErr, 0);

  // Read only before.
  if (nZero > 0) {
    fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));

    file->readSignal(a.data(), -1000 - nZero - nNormal + 1, -1000 - nNormal);

    compareMatrix(a.data(), zero.data(), file->getChannelCount(), nZero,
                  &relErr, &absErr);
    EXPECT_DOUBLE_EQ(relErr, 0);
    EXPECT_DOUBLE_EQ(absErr, 0);
  }

  // Read only after.
  if (nZero > 0) {
    fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));

    auto end = 1000 + file->getSamplesRecorded();
    file->readSignal(a.data(), end + nNormal, end + nNormal + nZero - 1);

    compareMatrix(a.data(), zero.data(), file->getChannelCount(), nZero,
                  &relErr, &absErr);
    EXPECT_DOUBLE_EQ(relErr, 0);
    EXPECT_DOUBLE_EQ(absErr, 0);
  }
}

void outOfBoundsTest(DataFile *file) {
  outOfBoundsTest(file, 1, 0);
  outOfBoundsTest(file, 100, 0);

  vector<int> valuesToTry = {29, 31, 37, 41, 43, 47,  53,  59,  61,  67,  71,
                             73, 79, 83, 89, 97, 100, 101, 149, 200, 229, 1111};

  for (int i = 1; i <= 25; ++i)
    valuesToTry.push_back(i);

  for (int e : valuesToTry)
    for (int ee : valuesToTry)
      outOfBoundsTest(file, e, ee);
}

void dataTest(DataFile *file, TestFile *testFile,
              double maxRelErrDouble = MAX_REL_ERR_DOUBLE,
              double maxRelErrFloat = MAX_REL_ERR_FLOAT,
              double maxAbsErrDouble = MAX_ABS_ERR_DOUBLE,
              double maxAbsErrFloat = MAX_ABS_ERR_FLOAT) {
  int channelCount = file->getChannelCount();
  auto values = testFile->getValues();

  int n = static_cast<int>(values.size());
  ASSERT_EQ(n % channelCount, 0);
  int sampleCount = n / channelCount;

  vector<double> dataD;
  dataD.insert(dataD.begin(), n, 0);
  file->readSignal(dataD.data(), 0, sampleCount - 1);

  vector<float> dataF;
  dataF.insert(dataF.begin(), n, 0);
  file->readSignal(dataF.data(), 0, sampleCount - 1);

  double relErr, absErr;

  compareMatrix(dataD.data(), values.data(), channelCount, sampleCount, &relErr,
                &absErr);
  EXPECT_LT(relErr, maxRelErrDouble);
  EXPECT_LT(absErr, maxAbsErrDouble);

  compareMatrix(dataF.data(), values.data(), channelCount, sampleCount, &relErr,
                &absErr);
  EXPECT_LT(relErr, maxRelErrFloat);
  EXPECT_LT(absErr, maxAbsErrFloat);
}

template <class T> void gdfStartTimeTest() {
  unique_ptr<DataFile> file;

  ASSERT_NO_THROW(file.reset(new T(TEST_DATA_PATH + "gdf/startTime.gdf")));

  double date = file->getStartDate();
  double days = 733779.88725694443564862013; // sprintf('%20.20f', datenum(2009,
                                             // 1, 6, 21, 17, 39)) in Matlab
  EXPECT_NEAR(date, days, 1. / 24 / 60 / 60 / 1000);
}

const int MAT_FS = 100;
const int MAT_CHANNELS = 19;
const int MAT_SAMPLES = 400;

} // namespace

class primary_file_test : public ::testing::Test {
protected:
  primary_file_test()
      : gdf00(TEST_DATA_PATH + "gdf/gdf00.gdf", 200, 19, 364000),
        gdf01(TEST_DATA_PATH + "gdf/gdf01.gdf", 50, 1, 2050),
        edf00(TEST_DATA_PATH + "edf/edf00.edf", 200, 37, 363620),
        mat4(TEST_DATA_PATH + "mat/4.mat", MAT_FS, MAT_CHANNELS, MAT_SAMPLES),
        mat6(TEST_DATA_PATH + "mat/6.mat", MAT_FS, MAT_CHANNELS, MAT_SAMPLES),
        mat7(TEST_DATA_PATH + "mat/7.mat", MAT_FS, MAT_CHANNELS, MAT_SAMPLES),
        mat73(TEST_DATA_PATH + "mat/73.mat", MAT_FS, MAT_CHANNELS, MAT_SAMPLES),
        matDefault(TEST_DATA_PATH + "mat/default.mat", MAT_FS, MAT_CHANNELS,
                   MAT_SAMPLES) {}

  TestFile gdf00, gdf01;
  TestFile edf00;
  TestFile mat4, mat6, mat7, mat73, matDefault;
};

// Tests common to all file types.
TEST_F(primary_file_test, outOfBounds) {
  outOfBoundsTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get());
  outOfBoundsTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get());

  outOfBoundsTest(unique_ptr<DataFile>(edf00.makeEDF()).get());

  // TODO: Fix this test. Perhaps lover the precision as in EDF_data_00.
  //  MATvars matVars;
  //  matVars.data = {"data0", "data1"};
  //  matVars.frequency = "Fs";
  //  outOfBoundsTest(unique_ptr<DataFile>(mat4.makeMAT(matVars)).get());
}

// TODO: Add all kinds of crazy tests that read samples and compare them to data
// read from the whole file. Like read only one sample long block.
// TODO: Test whether readSignal modifies immediately before and after the bufer
// size -- whether it writes out of bounds.

// Tests of my GDF implementation.
TEST_F(primary_file_test, GDF2_exceptions) {
  EXPECT_ANY_THROW(printException(
      [this]() { GDF2 file(TEST_DATA_PATH + "gdf/empty.gdf"); }));
  EXPECT_ANY_THROW(printException(
      [this]() { GDF2 file(TEST_DATA_PATH + "gdf/headerOnly.gdf"); }));
  EXPECT_THROW(printException(
                   [this]() { GDF2 file(TEST_DATA_PATH + "gdf/badType.gdf"); }),
               runtime_error);
  EXPECT_THROW(printException(
                   [this]() { GDF2 file(TEST_DATA_PATH + "gdf/badFile.gdf"); }),
               runtime_error);

  unique_ptr<DataFile> file;

  vector<double> data;
  data.insert(data.begin(), 100000, 0);

  ASSERT_NO_THROW(file.reset(new GDF2(TEST_DATA_PATH + "gdf/gdf00.gdf")));
  EXPECT_THROW(printException([this, &file, &data]() {
                 file->readSignal(data.data(), 100, 50);
               }),
               invalid_argument);
}

TEST_F(primary_file_test, GDF2_start_time) { gdfStartTimeTest<GDF2>(); }

TEST_F(primary_file_test, GDF2_meta_info) {
  metaInfoTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get(), &gdf00);
  metaInfoTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get(), &gdf01);
}

TEST_F(primary_file_test, GDF2_data_00) {
  // TODO: generate new values files with higher precision
  dataTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get(), &gdf00);
}

TEST_F(primary_file_test, GDF2_data_01) {
  dataTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get(), &gdf01);
}

// Tests of EDFlib.
TEST_F(primary_file_test, EDF_exceptions) {
  unique_ptr<DataFile> file;

  vector<double> data;
  data.insert(data.begin(), 100000, 0);

  ASSERT_NO_THROW(file.reset(new EDF(TEST_DATA_PATH + "edf/edf00.edf")));
  EXPECT_THROW(printException([this, &file, &data]() {
                 file->readSignal(data.data(), 100, 50);
               }),
               invalid_argument);
}

TEST_F(primary_file_test, EDF_start_time) {
  unique_ptr<DataFile> file;

  ASSERT_NO_THROW(file.reset(new EDF(TEST_DATA_PATH + "edf/edf00.edf")));

  double date = file->getStartDate();
  double days = 732570.42965277773328125477; // sprintf('%20.20f', datenum(2005,
                                             // 9, 15, 10, 18, 42)) in Matlab
  EXPECT_LE(abs(date - days), 1.) << "Start time is not within 24 hours.";
  // Because the returned start date is effected by time-zone settings in the
  // OS. TODO: Fix this.
}

TEST_F(primary_file_test, EDF_meta_info) {
  metaInfoTest(unique_ptr<DataFile>(edf00.makeEDF()).get(), &edf00);
}

TEST_F(primary_file_test, EDF_data_00) {
  dataTest(unique_ptr<DataFile>(edf00.makeEDF()).get(), &edf00,
           MAX_REL_ERR_DOUBLE / 500, MAX_REL_ERR_FLOAT / 100,
           MAX_ABS_ERR_DOUBLE / 10000, MAX_ABS_ERR_FLOAT / 100);
}

// Tests of MAT. TODO: Split these tests into smaller test cases (by file type
// and/or test type).
TEST_F(primary_file_test, MAT_meta_info_old) {
  MATvars vars;
  vars.data = vector<string>{"data0", "data1"};
  vars.frequency = "Fs";

  metaInfoTest(unique_ptr<DataFile>(mat4.makeMAT(vars)).get(), &mat4);

  vars.data = vector<string>{"data0"};

  metaInfoTest(unique_ptr<DataFile>(mat6.makeMAT(vars)).get(), &mat6);
}

TEST_F(primary_file_test, MAT_meta_info_compression) {
  MATvars vars;
  vars.data = vector<string>{"data0", "data1"};
  vars.frequency = "Fs";

  metaInfoTest(unique_ptr<DataFile>(mat7.makeMAT(vars)).get(), &mat7);
  metaInfoTest(unique_ptr<DataFile>(mat73.makeMAT(vars)).get(), &mat73);
  metaInfoTest(unique_ptr<DataFile>(matDefault.makeMAT(vars)).get(),
               &matDefault);
}

TEST_F(primary_file_test, MAT_data_old) {
  MATvars vars;
  vars.data = vector<string>{"data0", "data1"};
  vars.frequency = "Fs";

  dataTest(unique_ptr<DataFile>(mat4.makeMAT(vars)).get(), &mat4);

  vars.data = vector<string>{"data0"};

  dataTest(unique_ptr<DataFile>(mat6.makeMAT(vars)).get(), &mat6);
}

TEST_F(primary_file_test, MAT_data_compression) {
  MATvars vars;
  vars.data = vector<string>{"data0", "data1"};
  vars.frequency = "Fs";

  dataTest(unique_ptr<DataFile>(mat7.makeMAT(vars)).get(), &mat7);
  dataTest(unique_ptr<DataFile>(mat73.makeMAT(vars)).get(), &mat73);
  dataTest(unique_ptr<DataFile>(matDefault.makeMAT(vars)).get(), &matDefault);
}

// BioSig tests.
#ifdef USE_BIOSIG

TEST_F(primary_file_test, BioSig_meta_info_GDF) {
  metaInfoTest(unique_ptr<DataFile>(gdf00.makeBioSigFile()).get(), &gdf00);
  metaInfoTest(unique_ptr<DataFile>(gdf01.makeBioSigFile()).get(), &gdf01);
}

TEST_F(primary_file_test, BioSig_GDF2_data_00) {
  dataTest(unique_ptr<DataFile>(gdf00.makeBioSigFile()).get(), &gdf00);
}

TEST_F(primary_file_test, BioSig_GDF2_data_01) {
  dataTest(unique_ptr<DataFile>(gdf01.makeBioSigFile()).get(), &gdf01);
}

// TEST_F(primary_file_test, BioSig_meta_info_EDF) {
//  metaInfoTest(unique_ptr<DataFile>(edf00.makeBioSigFile()).get(), &edf00);
//}

// TEST_F(primary_file_test, BioSig_EDF_data_00) {
//  dataTest(unique_ptr<DataFile>(edf00.makeBioSigFile()).get(), &edf00,
//           MAX_REL_ERR_DOUBLE / 500, MAX_REL_ERR_FLOAT / 100,
//           MAX_ABS_ERR_DOUBLE / 10000, MAX_ABS_ERR_FLOAT / 100);
//}

#endif // USE_BIOSIG
