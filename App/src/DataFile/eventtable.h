#ifndef EVENTTABLE_H
#define EVENTTABLE_H

#include <QAbstractTableModel>

#include <QColor>

#include <vector>
#include <string>
#include <sstream>
#include <tuple>
#include <cassert>

class QXmlStreamReader;
class QXmlStreamWriter;

class EventTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	EventTable(QObject* parent = nullptr);
	~EventTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);	
	void getEventsForRendering(int firstSample, int lastSample, std::vector<std::tuple<int, int, int>>* allChannelEvents, std::vector<std::tuple<int, int, int, int>>* singleChannelEvents);

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return label.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return 6;
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

	std::string getLabel(int i) const
	{
		assert(0 <= i && i < label.size());

		return label[i];
	}
	void setLabel(const std::string& value, int i)
	{
		assert(0 <= i && i < label.size());

		setData(index(order[i], 0), QString::fromStdString(value));
	}
	int getType(int i) const
	{
		assert(0 <= i && i < type.size());

		return type[i];
	}
	void setType(int value, int i)
	{
		assert(0 <= i && i < type.size());

		setData(index(order[i], 1), value);
	}
	int getPosition(int i) const
	{
		assert(0 <= i && i < position.size());

		return position[i];
	}
	void setPosition(int value, int i)
	{
		assert(0 <= i && i < position.size());

		setData(index(order[i], 2), value);
	}
	int getDuration(int i) const
	{
		assert(0 <= i && i < duration.size());

		return duration[i];
	}
	void setDuration(int value, int i)
	{
		assert(0 <= i && i < duration.size());

		setData(index(order[i], 3), value);
	}
	int getChannel(int i) const
	{
		assert(0 <= i && i < channel.size());

		return channel[i];
	}
	void setChannel(int value, int i)
	{
		assert(0 <= i && i < channel.size());

		setData(index(order[i], 4), value);
	}
	std::string getDescription(int i) const
	{
		assert(0 <= i && i < description.size());

		return description[i];
	}
	void setDescription(const std::string& value, int i)
	{
		assert(0 <= i && i < description.size());

		setData(index(order[i], 5), QString::fromStdString(value));
	}

private:
	std::vector<std::string> label;
	std::vector<int> type;
	std::vector<int> position;
	std::vector<int> duration;
	std::vector<int> channel;
	std::vector<std::string> description;

	std::vector<int> order;
};

#endif // EVENTTABLE_H
