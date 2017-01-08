#include "edftmp.h"

#include "tracktable.h"
#include "../options.h"

#include <QDateTime>
#include <QDate>

#include <AlenkaFile/edf.h>

#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <set>
#include <vector>

using namespace std;

EdfTmp::EdfTmp(const string& filePath) : DataFile(filePath)
{
	file = new AlenkaFile::EDF(filePath + ".edf");

	// Load info from secondary files.
	load();
}

EdfTmp::~EdfTmp()
{
	delete file;
}

double EdfTmp::getSamplingFrequency() const
{
	return file->getSamplingFrequency();
}

unsigned int EdfTmp::getChannelCount() const
{
	return file->getChannelCount();
}

uint64_t EdfTmp::getSamplesRecorded() const
{
	return file->getSamplesRecorded();
}

QDateTime EdfTmp::getStartDate() const
{
	return QDateTime::fromTime_t(file->getStartDate());

	/*QDateTime date(QDate();
	date = date.addDays(fh.startDate[1] - 719529);

	double fractionOfDay = ldexp(static_cast<double>(fh.startDate[0]), -32);
	date = date.addMSecs(fractionOfDay*24*60*60*1000);

	return date;*/
}

void EdfTmp::save()
{
	lock_guard<mutex> lock(fileMutex);

	DataFile::save();	
}

void EdfTmp::readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample)
{
	lock_guard<mutex> lock(fileMutex);

	file->readSignal(data->data(), firstSample, lastSample);
}

void EdfTmp::readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample)
{
	lock_guard<mutex> lock(fileMutex);

	file->readSignal(data->data(), firstSample, lastSample);
}

bool EdfTmp::load()
{
	if (DataFile::load() == false)
	{
		// TODO: handle unexpected values in event table and strange track labels
		// TODO: handle events in the primary EDF file

		lock_guard<mutex> lock(fileMutex);

		getInfoTable()->setVirtualWidth(getSamplesRecorded()/getSamplingFrequency()*100); // Set default zoom at 100 pixels per second.

		// Fill the track table of the default montage with channels from gdf.
		assert(getChannelCount() > 0);

		TrackTable* defaultTracks = getMontageTable()->getTrackTables()->back();
		defaultTracks->insertRowsBack(getChannelCount());

		for (int i = 0; i < defaultTracks->rowCount(); ++i)
		{
			defaultTracks->setLabel("Ch " + to_string(i), i);
		}

		/*// Load event table.
		seekFile(startOfEventTable, true);

		uint8_t eventTableMode;
		readFile(&eventTableMode);

		uint8_t nev[3];
		readFile(nev, 3);
		if (isLittleEndian == false)
		{
			changeEndianness(reinterpret_cast<char*>(nev), 3);
		}
		int numberOfEvents = nev[0] + nev[1]*256 + nev[2]*256*256;

		seekFile(4);

		readGdfEventTable(numberOfEvents, eventTableMode);*/
	}

	return true;
}
