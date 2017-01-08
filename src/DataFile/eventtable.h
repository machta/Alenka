#ifndef EVENTTABLE_H
#define EVENTTABLE_H

#include <QAbstractTableModel>

#include <QColor>

#include <string>
#include <tuple>
#include <cassert>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;
class EventTypeTable;
class TrackTable;
class DataFile;

/**
 * @brief A data structure for handling events.
 *
 * This class stores info about events.
 *
 * Two interfaces are available for accessing the underlying data:
 * * direct access via get/set functions
 * * interface inherited from QAbstractTableModel
 */
class EventTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	/**
	 * @brief Enum defining symbolic constants for the table columns and the number of columns.
	 */
	enum class Column
	{
		label, type, position, duration, channel, description, size
	};

	EventTable(DataFile* file, QObject* parent = nullptr);
	~EventTable();

	/**
	 * @brief Sets pointers to related objects.
	 */
	void setReferences(EventTypeTable* eventTypeTable, TrackTable* trackTable)
	{
		this->eventTypeTable = eventTypeTable;
		this->trackTable = trackTable;
	}

	/**
	 * @brief Writes an eventTable element.
	 */
	void write(QXmlStreamWriter* xml) const;

	/**
	 * @brief Parses the eventTable element.
	 */
	void read(QXmlStreamReader* xml);

	/**
	 * @brief A convenience method for retrieving event info for rendering.
	 * @param allChannelEvents [out] All-channel event details sorted by type.
	 * @param singleChannelEvents [out] Single-channel event details sorted by type and channel.
	 */
	void getEventsForRendering(int firstSample, int lastSample, std::vector<std::tuple<int, int, int>>* allChannelEvents, std::vector<std::tuple<int, int, int, int>>* singleChannelEvents);

	const EventTypeTable* getEventTypeTable() const
	{
		return eventTypeTable;
	}
	const TrackTable* getTrackTable() const
	{
		return trackTable;
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

		return label.size();
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
	std::string getLabel(int i) const
	{
		assert(0 <= i && i < static_cast<int>(label.size()));

		return label[i];
	}
	void setLabel(const std::string& value, int i)
	{
		assert(0 <= i && i < static_cast<int>(label.size()));

		label[i] = value;
	}
	int getType(int i) const
	{
		assert(0 <= i && i < static_cast<int>(type.size()));

		return type[i];
	}
	void setType(int value, int i)
	{
		assert(0 <= i && i < static_cast<int>(type.size()));

		type[i] = value;
	}
	int getPosition(int i) const
	{
		assert(0 <= i && i < static_cast<int>(position.size()));

		return position[i];
	}
	void setPosition(int value, int i)
	{
		assert(0 <= i && i < static_cast<int>(position.size()));

		position[i] = value;
	}
	int getDuration(int i) const
	{
		assert(0 <= i && i < static_cast<int>(duration.size()));

		return duration[i];
	}
	void setDuration(int value, int i)
	{
		assert(0 <= i && i < static_cast<int>(duration.size()));

		duration[i] = value;
	}
	int getChannel(int i) const
	{
		assert(0 <= i && i < static_cast<int>(channel.size()));

		return channel[i];
	}
	void setChannel(int value, int i)
	{
		assert(0 <= i && i < static_cast<int>(channel.size()));

		channel[i] = value;
	}
	std::string getDescription(int i) const
	{
		assert(0 <= i && i < static_cast<int>(description.size()));

		return description[i];
	}
	void setDescription(const std::string& value, int i)
	{
		assert(0 <= i && i < static_cast<int>(description.size()));

		description[i] = value;
	}

private:
	DataFile* file;

	std::vector<std::string> label;
	std::vector<int> type;
	std::vector<int> position;
	std::vector<int> duration;
	std::vector<int> channel;
	std::vector<std::string> description;

	EventTypeTable* eventTypeTable;
	TrackTable* trackTable;
	std::vector<int> order;
};

#endif // EVENTTABLE_H
