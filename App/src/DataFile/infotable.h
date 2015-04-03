#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <QObject>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class InfoTable : public QObject
{
	Q_OBJECT

public:
	InfoTable();
	~InfoTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);

	int getVirtualWidth() const
	{
		return virtualWidth;
	}
	int getPosition() const
	{
		return position;
	}

signals:
	void virtualWidthChanged(int);
	void positionChanged(int);

public slots:
	void setVirtualWidth(int value)
	{
		if (value != virtualWidth)
		{
			virtualWidth = value;
			emit virtualWidthChanged(virtualWidth);
		}
	}
	void setPosition(int value)
	{
		if (value != position)
		{
			position = value;
			emit positionChanged(position);
		}
	}

private:
	int virtualWidth = 100000;
	int position = 0;
};

#endif // INFOTABLE_H
