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

class EventTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	EventTable(QObject* parent = 0);
	~EventTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
	{
		(void)parent;

		return label.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
	{
		(void)parent;

		return 6;
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);
	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		if (!index.isValid())
		{
			return Qt::ItemIsEnabled;
		}

		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

private:
	std::vector<std::string> label;
	std::vector<int> type;
	std::vector<qlonglong> position;
	std::vector<qlonglong> duration;
	std::vector<int> channel;
	std::vector<std::string> description;
};

#endif // EVENTTABLE_H
