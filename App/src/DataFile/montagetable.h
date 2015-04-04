#ifndef MONTAGETABLE_H
#define MONTAGETABLE_H

#include <QAbstractTableModel>

#include "tracktable.h"
#include "eventtable.h"

#include <QObject>
#include <QColor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <vector>
#include <string>
#include <sstream>

class MontageTable : public QAbstractTableModel
{
public:
	MontageTable(QObject* parent = nullptr);
	~MontageTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	std::vector<TrackTable*>* getTrackTables()
	{
		return &trackTables;
	}
	std::vector<EventTable*>* getEventTables()
	{
		return &eventTables;
	}
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return name.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return 2;
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
		{
			return Qt::ItemIsEnabled;
		}

		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
	std::vector<TrackTable*> trackTables;
	std::vector<EventTable*> eventTables;

	std::vector<std::string> name;
	std::vector<bool> save;
};

#endif // MONTAGETABLE_H
