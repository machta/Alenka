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
class EventTypeTable;

class MontageTable : public QAbstractTableModel
{
public:
	enum class Column
	{
		name, save, collumnCount
	};

	MontageTable(QObject* parent = nullptr);
	~MontageTable();

	void setReferences(EventTypeTable* eventTypeTable)
	{
		this->eventTypeTable = eventTypeTable;
	}
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
	EventTypeTable* getEventTypeTable()
	{
		return eventTypeTable;
	}
	bool insertRowsBack(int count = 1);

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return name.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return static_cast<int>(Column::collumnCount);
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

		name[i] = value;
	}
	bool getSave(int i) const
	{
		assert(0 <= i && i < static_cast<int>(save.size()));

		return save[i];
	}
	void setSave(bool value, int i)
	{
		assert(0 <= i && i < static_cast<int>(save.size()));

		save[i] = value;
	}

private:
	std::vector<std::string> name;
	std::vector<bool> save;

	EventTypeTable* eventTypeTable;
	std::vector<int> order;
	std::vector<TrackTable*> trackTables;
	std::vector<EventTable*> eventTables;

	void pushBackNew();
};

#endif // MONTAGETABLE_H
