#include "datafile.h"

#include <qfile.h>

using namespace std;

DataFile::DataFile(const string& filePath) : filePath(filePath)
{
}

DataFile::~DataFile()
{
	saveXMLFile(".info", [this] (QXmlStreamWriter* xml)
	{
		infoTable.write(xml);
	});

	for (const auto& e : montageTables)
	{
		delete e;
	}
}

void DataFile::save()
{
	saveXMLFile(".mont", [this] (QXmlStreamWriter* xml)
	{
		for (const auto& e : montageTables)
		{
			e->write(xml);
		}

		eventTypeTable.write(xml);
	});
}

bool DataFile::loadMontFile()
{
	return loadXMLFile(".mont", [this] (QXmlStreamReader* xml)
	{
		while (xml->readNextStartElement())
		{
			auto name = xml->name();
			if (name == "montageTable")
			{
				montageTables.push_back(new MontageTable);
				montageTables.back()->read(xml);
			}
			else if (name == "eventTypeTable")
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

bool DataFile::loadInfoFile()
{
	return loadXMLFile(".info", [this] (QXmlStreamReader* xml)
	{
		infoTable.read(xml);
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

	xmlFile.close();
}
