#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class InfoTable : QObject
{
	Q_OBJECT

public:
	InfoTable();
	~InfoTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
};

#endif // INFOTABLE_H
