#include "datafile.h"

#include "../error.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDateTime>
#include <QDate>

#include <cassert>

using namespace std;

DataFile::DataFile(const string& filePath) : filePath(filePath), eventTypeTable(this), montageTable(this)
{
	eventTypeTable.setReferences(&montageTable);
	montageTable.setReferences(&eventTypeTable);
}

DataFile::~DataFile()
{
	saveXMLFile(".info", [this] (QXmlStreamWriter* xml)
	{
		infoTable.write(xml);
	});
}

void DataFile::save()
{
	saveXMLFile(".mont", [this] (QXmlStreamWriter* xml)
	{
		montageTable.write(xml);
		eventTypeTable.write(xml);
	});
}

QDateTime DataFile::sampleToDate(int sample)
{
	int timeOffset = static_cast<int>(sample/getSamplingFrequency()*1000);

	QDateTime date = getStartDate();
	date = date.addMSecs(timeOffset);

	return date;
}

QDateTime DataFile::sampleToOffset(int sample)
{
	int timeOffset = static_cast<int>(sample/getSamplingFrequency()*1000);

	QDateTime date(QDate(1970, 1, 1));
	date = date.addMSecs(timeOffset);

	return date;
}

QString DataFile::sampleToDateTimeString(int sample)
{
	if (infoTable.getTimeMode() == InfoTable::TimeMode::samples)
	{
		return QString::number(sample);
	}
	else if (infoTable.getTimeMode() == InfoTable::TimeMode::offset)
	{
		QDateTime date = sampleToOffset(sample);
		return QString::number(date.date().day() - 1, 2) + date.toString(":hh:mm:ss");
	}
	else if (infoTable.getTimeMode() == InfoTable::TimeMode::real)
	{
		return sampleToDate(sample).toString("dd:MM:yyyy hh:mm:ss");
	}

	return QString();
}

bool DataFile::load()
{
	loadXMLFile(".info", [this] (QXmlStreamReader* xml)
	{
		infoTable.read(xml);
	});

	return loadXMLFile(".mont", [this] (QXmlStreamReader* xml)
	{
		while (xml->readNextStartElement())
		{
			if (xml->name() == "montageTable")
			{
				montageTable.read(xml);
			}
			else if (xml->name() == "eventTypeTable")
			{
				eventTypeTable.read(xml);
			}
			else
			{
				xml->skipCurrentElement();
			}
		}
	});	
}

bool DataFile::loadXMLFile(const string& extension, function<void (QXmlStreamReader*)> loadFunction)
{
	QFile xmlFile(QString::fromStdString(filePath + extension));
	bool ret;

	if (xmlFile.exists())
	{
		xmlFile.open(QIODevice::ReadOnly);

		QXmlStreamReader xml(&xmlFile);
		xml.readNextStartElement();
		assert(xml.name() == "document");

		loadFunction(&xml);

		if (xml.hasError())
		{
			logToBoth("XML error(" << xml.error() << ") while reading file '" << filePath + extension << "': " << xml.errorString().toStdString());
		}

		ret = true;
	}
	else
	{
		ret = false;
	}

	xmlFile.close();
	return ret;
}

void DataFile::saveXMLFile(const string& extension, std::function<void (QXmlStreamWriter*)> saveFunction)
{
	QFile xmlFile(QString::fromStdString(filePath + extension));
	xmlFile.open(QIODevice::WriteOnly);

	QXmlStreamWriter xml(&xmlFile);
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-1);
	xml.writeStartDocument();
	xml.writeStartElement("document");

	saveFunction(&xml);

	xml.writeEndDocument();

	if (xml.hasError())
	{
		logToBoth("XML error occurred while writing to file '" << filePath + extension << "'");
	}

	xmlFile.close();
}
