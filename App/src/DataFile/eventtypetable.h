#ifndef EVENTTYPETABLE_H
#define EVENTTYPETABLE_H

#include <QAbstractTableModel>

#include <QColor>

#include <vector>
#include <string>
#include <sstream>
#include <cassert>

class QXmlStreamReader;
class QXmlStreamWriter;

class EventTypeTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	EventTypeTable(QObject* parent = nullptr);
	~EventTypeTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return name.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return 5;
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

	int getId(int i) const
	{
		assert(0 <= i && i < static_cast<int>(id.size()));

		return id[i];
	}
	void setId(int value, int i)
	{
		assert(0 <= i && i < static_cast<int>(id.size()));

		setData(index(order[i], 0), value);
	}
	std::string getName(int i) const
	{
		assert(0 <= i && i < static_cast<int>(name.size()));

		return name[i];
	}
	void setName(const std::string& value, int i)
	{
		assert(0 <= i && i < static_cast<int>(name.size()));

		setData(index(order[i], 1), QString::fromStdString(value));
	}
	double getOpacity(int i) const
	{
		assert(0 <= i && i < static_cast<int>(opacity.size()));

		return opacity[i];
	}
	void setOpacity(double value, int i)
	{
		assert(0 <= i && i < static_cast<int>(opacity.size()));

		setData(index(order[i], 2), value);
	}
	QColor getColor(int i) const
	{
		assert(0 <= i && i < static_cast<int>(color.size()));

		return color[i];
	}
	void setColor(QColor value, int i)
	{
		assert(0 <= i && i < static_cast<int>(color.size()));

		setData(index(order[i], 3), value);
	}
	bool getHidden(int i) const
	{
		assert(0 <= i && i < static_cast<int>(hidden.size()));

		return hidden[i];
	}
	void setHidden(bool value, int i)
	{
		assert(0 <= i && i < static_cast<int>(hidden.size()));

		setData(index(order[i], 4), value);
	}

private:
	std::vector<int> id;
	std::vector<std::string> name;
	std::vector<double> opacity;
	std::vector<QColor> color;
	std::vector<bool> hidden;

	std::vector<int> order;
};

#endif // EVENTTYPETABLE_H
