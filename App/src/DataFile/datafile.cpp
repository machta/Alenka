#include "datafile.h"

#include <qfile.h>

using namespace std;

DataFile::DataFile(const string& filePath) : filePath(filePath)
{
}

DataFile::~DataFile()
{
	for (const auto& e : montageTables)
	{
		delete e;
	}
}

void DataFile::save()
{
	QFile montFile(QString::fromStdString(filePath + ".mont"));
	montFile.open(QIODevice::WriteOnly);

	QXmlStreamWriter xml(&montFile);
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-1);
	xml.writeStartDocument();
	xml.writeStartElement("document");

	for (const auto& e : montageTables)
	{
		e->write(&xml);
	}

	eventTypeTable.write(&xml);

	xml.writeEndDocument();

	montFile.close();
}

bool DataFile::loadMontFile()
{
	QFile montFile(QString::fromStdString(filePath + ".mont"));

	if (montFile.exists())
	{
		montFile.open(QIODevice::ReadOnly);

		QXmlStreamReader xml(&montFile);
		xml.readNextStartElement();
		assert(xml.name() == "document");

		while (xml.readNextStartElement())
		{
			auto name = xml.name();
			if (name == "montageTable")
			{
				montageTables.push_back(new MontageTable);
				montageTables.back()->read(&xml);
			}
			else if (name == "eventTypeTable")
			{
				eventTypeTable.read(&xml);
			}
			else
			{
				xml.skipCurrentElement();
			}
		}

		return true;
	}
	else
	{
		return false;
	}

	montFile.close();
}
