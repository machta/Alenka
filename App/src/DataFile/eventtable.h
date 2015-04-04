#ifndef EVENTTABLE_H
#define EVENTTABLE_H

#include <QAbstractTableModel>

#include <QObject>
#include <QColor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <vector>
#include <string>
#include <sstream>
#include <tuple>

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

private:
	std::vector<std::string> label;
	std::vector<int> type;
	std::vector<int> position;
	std::vector<int> duration;
	std::vector<int> channel;
	std::vector<std::string> description;
};

#endif // EVENTTABLE_H
