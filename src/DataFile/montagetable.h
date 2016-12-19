#ifndef MONTAGETABLE_H
#define MONTAGETABLE_H

#include <QAbstractTableModel>

#include "tracktable.h"
#include "eventtable.h"

#include <QColor>

#include <string>
#include <sstream>
#include <cassert>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;
class EventTypeTable;
class DataFile;

/**
 * @brief A data structure for handling montages.
 *
 * This class stores info about montages.
 *
 * Two interfaces are available for accessing the underlying data:
 * * direct access via get/set functions
 * * interface inherited from QAbstractTableModel
 */
class MontageTable : public QAbstractTableModel
{
public:
	/**
	 * @brief Enum defining symbolic constants for the table columns and the number of columns.
	 */
	enum class Column
	{
		name, save, size
	};

	MontageTable(DataFile* file, QObject* parent = nullptr);
	~MontageTable();

	/**
	 * @brief Sets pointers to related objects.
	 */
	void setReferences(EventTypeTable* eventTypeTable)
	{
		this->eventTypeTable = eventTypeTable;
	}

	/**
	 * @brief Writes an montageTable element.
	 */
	void write(QXmlStreamWriter* xml) const;

	/**
	 * @brief Parses the montageTable element.
	 */
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

	/**
	 * @brief Inserts count rows with default values at the end of the table.
	 */
	bool insertRowsBack(int count = 1);

	/**
	 * @brief Notify the program that the values in column have changed.
	 */
	void emitColumnChanged(Column column)
	{
		emit dataChanged(index(0, static_cast<int>(column)), index(rowCount() - 1, static_cast<int>(column)));
	}

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return name.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return static_cast<int>(Column::size);
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

	DataFile* getDataFile()
	{
		return file;
	}
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
	DataFile* file;

	std::vector<std::string> name;
	std::vector<bool> save;

	EventTypeTable* eventTypeTable;
	std::vector<int> order;
	std::vector<TrackTable*> trackTables;
	std::vector<EventTable*> eventTables;

	void pushBackNew();
};

#endif // MONTAGETABLE_H
