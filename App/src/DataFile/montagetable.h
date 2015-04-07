#ifndef MONTAGETABLE_H
#define MONTAGETABLE_H

#include <QAbstractTableModel>

#include "tracktable.h"
#include "eventtable.h"

#include <QColor>

#include <vector>
#include <string>
#include <sstream>
#include <cassert>

class QXmlStreamReader;
class QXmlStreamWriter;

class MontageTable : public QAbstractTableModel
{
public:
	MontageTable(EventTypeTable* eventTypeTable, QObject* parent = nullptr);
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
	virtual void sort(int column, Qt::SortOrder order) override;

	std::string getName(int i) const
	{
		assert(0 <= i && i < static_cast<int>(name.size()));

		return name[i];
	}
	void setName(const std::string& value, int i)
	{
		assert(0 <= i && i < static_cast<int>(name.size()));

		setData(index(order[i], 0), QString::fromStdString(value));
	}
	bool getSave(int i) const
	{
		assert(0 <= i && i < static_cast<int>(save.size()));

		return save[i];
	}
	void setSave(bool value, int i)
	{
		assert(0 <= i && i < static_cast<int>(save.size()));

		setData(index(order[i], 1), value);
	}

private:
	std::vector<std::string> name;
	std::vector<bool> save;

	EventTypeTable* eventTypeTable;
	std::vector<int> order;
	std::vector<TrackTable*> trackTables;
	std::vector<EventTable*> eventTables;
};

#endif // MONTAGETABLE_H
