#include "gdf2.h"

#include "../options.h"

#include <QDateTime>
#include <QDate>
#include <QFile>

#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <set>
#include <vector>

using namespace std;

GDF2::GDF2(const string& filePath) : DataFile(filePath)
{
	isLittleEndian = testLittleEndian();

	string fp = filePath + ".gdf";

	file = new QFile(fp.c_str());

	bool res = file->open(QIODevice::ReadWrite);
	checkErrorCode(res, true, "File '" << fp << "' could not be opened for reading and writing.");

	// Load fixed header.
	seekFile(0, true);

	readFile(fh.versionID, 8);
	fh.versionID[8] = 0;
	int major, minor;
	sscanf(fh.versionID + 4, "%d.%d", &major, &minor);
	version = minor + 100*major;

	if (string(fh.versionID, 3) != "GDF" || major != 2)
	{
		throw runtime_error("Unrecognized file format.");
	}

	readFile(fh.patientID, 66);
	fh.patientID[66] = 0;

	seekFile(10);

	readFile(&fh.drugs);

	readFile(&fh.weight);

	readFile(&fh.height);

	readFile(&fh.patientDetails);

	readFile(fh.recordingID, 64);
	fh.recordingID[64] = 0;

	readFile(fh.recordingLocation, 4);

	readFile(fh.startDate, 2);

	readFile(fh.birthday, 2);

	readFile(&fh.headerLength);

	readFile(fh.ICD, 6);

	readFile(&fh.equipmentProviderID);

	seekFile(6);

	readFile(fh.headsize, 3);

	readFile(fh.positionRE, 3);

	readFile(fh.positionGE, 3);

	readFile(&fh.numberOfDataRecords);

	if (fh.numberOfDataRecords < 0)
	{
		runtime_error("GDF file with unknown number of data records is not supported.");
	}

	double duration;
	if (version > 220)
	{
		double* ptr = reinterpret_cast<double*>(fh.durationOfDataRecord);
		readFile(ptr, 1);
		duration = *ptr;
	}
	else
	{
		uint32_t* ptr = reinterpret_cast<uint32_t*>(fh.durationOfDataRecord);
		readFile(ptr, 2);
		duration = static_cast<double>(ptr[0])/static_cast<double>(ptr[1]);
	}

	readFile(&fh.numberOfChannels);

	// Load variable header.
	seekFile(2);
	assert(file->pos() == 256);

	vh.label = new char[getChannelCount()][16 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(vh.label[i], 16);
		vh.label[i][16] = 0;
	}

	vh.typeOfSensor = new char[getChannelCount()][80 + 1];
	for (unsigned int i = 0; i < getChannelCount(); ++i)
	{
		readFile(vh.typeOfSensor[i], 80);
		vh.typeOfSensor[i][80] = 0;
	}

	seekFile(6*getChannelCount());

	vh.physicalDimensionCode = new uint16_t[getChannelCount()];
	readFile(vh.physicalDimensionCode, getChannelCount());

	vh.physicalMinimum = new double[getChannelCount()];
	readFile(vh.physicalMinimum, getChannelCount());

	vh.physicalMaximum = new double[getChannelCount()];
	readFile(vh.physicalMaximum, getChannelCount());

	vh.digitalMinimum = new double[getChannelCount()];
	readFile(vh.digitalMinimum, getChannelCount());

	vh.digitalMaximum = new double[getChannelCount()];
	readFile(vh.digitalMaximum, getChannelCount());

	seekFile(64*getChannelCount());

	vh.timeOffset = new float[getChannelCount()];
	readFile(vh.timeOffset, getChannelCount());

	vh.lowpass = new float[getChannelCount()];
	readFile(vh.lowpass, getChannelCount());

	vh.highpass = new float[getChannelCount()];
	readFile(vh.highpass, getChannelCount());

	vh.notch = new float[getChannelCount()];
	readFile(vh.notch, getChannelCount());

	vh.samplesPerRecord = new uint32_t[getChannelCount()];
	readFile(vh.samplesPerRecord, getChannelCount());

	vh.typeOfData = new uint32_t[getChannelCount()];
	readFile(vh.typeOfData, getChannelCount());

	vh.sensorPosition = new float[getChannelCount()][3];
	readFile(*vh.sensorPosition, 3*getChannelCount());

	vh.sensorInfo = new char[getChannelCount()][20];
	readFile(*vh.sensorInfo, 20*getChannelCount());

	assert(file->pos() == 256 + 256*getChannelCount());

	// Initialize other members.
	samplesRecorded = vh.samplesPerRecord[0]*fh.numberOfDataRecords;

	startOfData = 256*fh.headerLength;

#define CASE(a_, b_)\
case a_:\
	dataTypeSize = sizeof(b_);\
	convertSampleToDouble = [] (void* sample) -> double\
	{\
		b_ tmp = *reinterpret_cast<b_*>(sample);\
		return static_cast<double>(tmp);\
	};\
	break;

	switch (vh.typeOfData[0])
	{
		CASE(1, int8_t);
		CASE(2, uint8_t);
		CASE(3, int16_t);
		CASE(4, uint16_t);
		CASE(5, int32_t);
		CASE(6, uint32_t);
		CASE(7, int64_t);
		CASE(8, uint64_t);
		CASE(16, float);
		CASE(17, double);
	default:
		throw runtime_error("Unsupported data type.");
		break;
	}

#undef CASE

	uncalibrated = PROGRAM_OPTIONS["uncalibrated"].as<bool>();

	if (uncalibrated == false)
	{
		scale = new double[getChannelCount()];
		for (unsigned int i = 0; i < getChannelCount(); ++i)
		{
			scale[i] = (vh.digitalMaximum[i] - vh.digitalMinimum[i])/
					   (vh.physicalMaximum[i] - vh.physicalMinimum[i]);
		}
	}
	else
	{
		scale = nullptr;
	}

	samplingFrequency = vh.samplesPerRecord[0]/duration;

	int64_t dataRecordBytes = vh.samplesPerRecord[0]*getChannelCount()*dataTypeSize;
	startOfEventTable = startOfData + dataRecordBytes*fh.numberOfDataRecords;

	// Construct the cache.
	int64_t memoryAvailable = PROGRAM_OPTIONS["dataFileCacheSize"].as<int64_t>();
	int cacheBlocks = memoryAvailable/dataRecordBytes;
	if (cacheBlocks > 0)
	{
		cache = new QCache<unsigned int, std::vector<char>>(cacheBlocks);
	}

	// Load info from secondary files.
	load();
}

GDF2::~GDF2()
{
	delete file;
	delete[] scale;
	delete cache;

	// Delete variable header.
	delete[] vh.label;
	delete[] vh.typeOfSensor;
	delete[] vh.physicalDimensionCode;
	delete[] vh.physicalMinimum;
	delete[] vh.physicalMaximum;
	delete[] vh.digitalMinimum;
	delete[] vh.digitalMaximum;
	delete[] vh.timeOffset;
	delete[] vh.lowpass;
	delete[] vh.highpass;
	delete[] vh.notch;
	delete[] vh.samplesPerRecord;
	delete[] vh.typeOfData;
	delete[] vh.sensorPosition;
	delete[] vh.sensorInfo;
}

QDateTime GDF2::getStartDate() const
{
	QDateTime date(QDate(1970, 1, 1));
	date = date.addDays(fh.startDate[1] - 719529);

	double fractionOfDay = ldexp(static_cast<double>(fh.startDate[0]), -32);
	date = date.addMSecs(fractionOfDay*24*60*60*1000);

	return date;
}

void GDF2::save()
{
	lock_guard<mutex> lock(fileMutex);

	DataFile::save();

	// Collect events from montages marked 'save'.
	vector<uint32_t> positions;
	vector<uint16_t> types;
	vector<uint16_t> channels;
	vector<uint32_t> durations;

	for (int i = 0; i < getMontageTable()->rowCount(); ++i)
	{
		if (getMontageTable()->getSave(i))
		{
			EventTable* et = getMontageTable()->getEventTables()->at(i);

			for (int j = 0; j < et->rowCount(); ++j)
			{
				if (et->getChannel(j) >= -1 && et->getType(j) >= 0)
				{
					positions.push_back(et->getPosition(j) + 1);
					types.push_back(getEventTypeTable()->getId(et->getType(j)));
					channels.push_back(et->getChannel(j) + 1);
					durations.push_back(et->getDuration(j));
				}
			}
		}
	}

	// Write mode, NEV and SR.
	seekFile(startOfEventTable, true);

	uint8_t eventTableMode = 3;
	writeFile(&eventTableMode);

	int numberOfEvents = min<int>(positions.size(), (1 << 24) - 1); // 2^24 - 1 is the maximum length of the gdf event table
	uint8_t nev[3];
	int tmp = numberOfEvents;
	nev[0] = static_cast<uint8_t>(tmp%256);
	tmp >>= 8;
	nev[1] = static_cast<uint8_t>(tmp%256);
	tmp >>= 8;
	nev[2] = static_cast<uint8_t>(tmp%256);
	if (isLittleEndian == false)
	{
		changeEndianness(reinterpret_cast<char*>(nev), 3);
	}
	writeFile(nev, 3);

	float sr = getSamplingFrequency();
	writeFile(&sr);

	// Write the events to the gdf event table.
	writeFile(positions.data(), numberOfEvents);
	writeFile(types.data(), numberOfEvents);
	writeFile(channels.data(), numberOfEvents);
	writeFile(durations.data(), numberOfEvents);
}

bool GDF2::load()
{
	if (DataFile::load() == false)
	{
		// TODO: handle unexpected values in event table and strange track labels
		lock_guard<mutex> lock(fileMutex);

		getInfoTable()->setVirtualWidth(getSamplesRecorded()/getSamplingFrequency()*100); // Set default zoom at 100 pixels per second.

		// Fill the track table of the default montage with channels from gdf.
		assert(getChannelCount() > 0);

		TrackTable* defaultTracks = getMontageTable()->getTrackTables()->back();
		defaultTracks->insertRowsBack(getChannelCount());

		for (int i = 0; i < defaultTracks->rowCount(); ++i)
		{
			defaultTracks->setLabel(vh.label[i], i);
		}

		// Load event table.
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

		readGdfEventTable(numberOfEvents, eventTableMode);
	}

	return true;
}

template<typename T>
void GDF2::readFile(T* val, int elements)
{
	size_t read = file->read(reinterpret_cast<char*>(val), sizeof(T)*elements);
	checkErrorCode(read, sizeof(T)*elements, "read() failed")

	if (isLittleEndian == false)
	{
		for (int i = 0; i < elements; ++i)
		{
			changeEndianness(val + i);
		}
	}
}

void GDF2::seekFile(int64_t offset, bool fromStart)
{
	if (fromStart == false)
	{
		offset += file->pos();
	}

	bool res = file->seek(offset);
	checkErrorCode(res, true, "seek() failed")
}

template<typename T>
void GDF2::writeFile(const T* val, int elements)
{
	if (isLittleEndian == false)
	{
		for (int i = 0; i < elements; ++i)
		{
			T tmp = val[i];
			changeEndianness(&tmp);
			file->write(reinterpret_cast<char*>(&tmp), sizeof(T));
		}
	}
	else
	{
		file->write(reinterpret_cast<const char*>(val), sizeof(T)*elements);
	}
}

void GDF2::readGdfEventTable(int numberOfEvents, int eventTableMode)
{
	if (numberOfEvents == 0)
	{
		return;
	}

	EventTable* defaultEvents = getMontageTable()->getEventTables()->back();

	defaultEvents->insertRowsBack(numberOfEvents);

	for (int i = 0; i < numberOfEvents; ++i)
	{
		uint32_t position;
		readFile(&position);
		int tmp = position - 1;
		defaultEvents->setPosition(tmp, i);
	}

	set<int> eventTypesUsed;

	for (int i = 0; i < numberOfEvents; ++i)
	{
		uint16_t type;
		readFile(&type);
		eventTypesUsed.insert(type);
		defaultEvents->setType(type, i);
	}

	for (int i = 0; i < numberOfEvents; ++i)
	{
		int type = defaultEvents->getType(i);

		type = distance(eventTypesUsed.begin(), eventTypesUsed.find(type));

		defaultEvents->setType(type, i);
	}

	if (eventTableMode & 0x02)
	{
		for (int i = 0; i < numberOfEvents; ++i)
		{
			uint16_t channel;
			readFile(&channel);
			int tmp = channel - 1;

			if (tmp >= getChannelCount())
				tmp = -1;

			defaultEvents->setChannel(tmp, i);
		}

		for (int i = 0; i < numberOfEvents; ++i)
		{
			uint32_t duration;
			readFile(&duration);
			defaultEvents->setDuration(duration, i);
		}
	}

	// Add all event types used in the gdf event table.
	for (const auto& e : eventTypesUsed)
	{
		int row = getEventTypeTable()->rowCount();

		getEventTypeTable()->insertRowsBack();

		std::stringstream ss;
		ss << "Type " << e;

		getEventTypeTable()->setId(e, row);
		getEventTypeTable()->setName(ss.str(), row);
	}
}

template<typename T>
void GDF2::readDataLocal(vector<T>* data, int64_t firstSample, int64_t lastSample)
{
	lock_guard<mutex> lock(fileMutex);

#ifndef NDEBUG
	int64_t originalFS = firstSample, originalLS = lastSample;
	(void)originalFS;
	(void)originalLS;
#endif

	if (lastSample < firstSample)
	{
		throw invalid_argument("lastSample must be greater than or equal to firstSample.");
	}

	if (lastSample - firstSample + 1 > data->size())
	{
		throw invalid_argument("The requested range cannot fit into data.");
	}

	// Special cases for when data before or after the file is requested.
	if (firstSample < 0 || lastSample >= samplesRecorded)
	{
		for (auto& e : *data)
		{
			e = 0;
		}

		// Exit early if there is nothing to read.
		if (lastSample < 0 || firstSample >= static_cast<int64_t>(samplesRecorded))
		{
			return;
		}
	}

	// Init variables needed later.
	int samplesPerRecord = vh.samplesPerRecord[0];
	int64_t rowLen = lastSample - firstSample + 1,
			dataIndex = 0,
			dataOffset, recordI, lastRecordI, n;

	if (firstSample < 0)
	{
		dataOffset = -firstSample;
		firstSample = recordI = 0;
	}
	else
	{
		dataOffset = 0;
		recordI = firstSample/samplesPerRecord;
	}

	if (lastSample >= samplesRecorded)
	{
		lastSample = samplesRecorded - 1;
	}

	n = lastSample - firstSample + 1;
	lastRecordI = lastSample/samplesPerRecord;

	// Read data from the file.
	for (; recordI <= lastRecordI; ++recordI)
	{
		vector<char>* record;

		if (cache == nullptr || (record = cache->operator [](recordI)) == nullptr)
		{
			record = new vector<char>(samplesPerRecord*getChannelCount()*dataTypeSize);

			seekFile(startOfData + recordI*samplesPerRecord*getChannelCount()*dataTypeSize, true);

			file->read(record->data(), dataTypeSize*samplesPerRecord*getChannelCount());

			if (isLittleEndian == false)
			{
				for (int i = 0; i < samplesPerRecord*getChannelCount(); ++i)
				{
					changeEndianness(record->data() + i*dataTypeSize, dataTypeSize);
				}
			}

			if (cache != nullptr)
			{
				cache->insert(recordI, record);
			}
		}

		int recordOffset = static_cast<int>((firstSample + dataIndex)%samplesPerRecord);
		int samplesToRead = static_cast<int>(min<int64_t>(samplesPerRecord - recordOffset, n - dataIndex));

		for (unsigned int channelI = 0; channelI < getChannelCount(); ++channelI)
		{
			for (int i = 0; i < samplesToRead; ++i)
			{
				// Copy a sample from the record.
				double tmp;
				char rawTmp[8];

				memcpy(rawTmp, record->data() + (channelI*samplesPerRecord + recordOffset + i)*dataTypeSize, dataTypeSize);

				tmp = convertSampleToDouble(rawTmp);

				// Calibration.
				if (uncalibrated == false)
				{
					tmp -= vh.digitalMinimum[channelI];
					tmp /= scale[channelI];
					tmp += vh.physicalMinimum[channelI];
				}

				size_t index = channelI*rowLen + dataOffset + dataIndex + i;
#ifndef NDEBUG
				(*data).at(index)
#else
				(*data)[index]
#endif
				= static_cast<T>(tmp);
			}
		}

		dataIndex += samplesToRead;

		if (cache == nullptr)
		{
			delete record;
		}
	}
}
