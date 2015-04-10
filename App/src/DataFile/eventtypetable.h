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
class MontageTable;

class EventTypeTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum class Column
	{
		id, name, opacity, color, hidden, collumnCount
	};

	EventTypeTable(QObject* parent = nullptr);
	~EventTypeTable();

	void setReferences(MontageTable* montageTable)
	{
		this->montageTable = montageTable;
	}
	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	MontageTable* getMontageTable()
	{
		return montageTable;
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

	int getId(int i) const
	{
		assert(0 <= i && i < static_cast<int>(id.size()));

		return id[i];
	}
	void setId(int value, int i)
	{
		assert(0 <= i && i < static_cast<int>(id.size()));

		id[i] = value;
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
	double getOpacity(int i) const
	{
		assert(0 <= i && i < static_cast<int>(opacity.size()));

		return opacity[i];
	}
	void setOpacity(double value, int i)
	{
		assert(0 <= i && i < static_cast<int>(opacity.size()));

		opacity[i] = value;
	}
	QColor getColor(int i) const
	{
		assert(0 <= i && i < static_cast<int>(color.size()));

		return color[i];
	}
	void setColor(QColor value, int i)
	{
		assert(0 <= i && i < static_cast<int>(color.size()));

		color[i] = value;
	}
	bool getHidden(int i) const
	{
		assert(0 <= i && i < static_cast<int>(hidden.size()));

		return hidden[i];
	}
	void setHidden(bool value, int i)
	{
		assert(0 <= i && i < static_cast<int>(hidden.size()));

		hidden[i] = value;
	}

	static const char* NO_TYPE_STRING;

private:
	std::vector<int> id;
	std::vector<std::string> name;
	std::vector<double> opacity;
	std::vector<QColor> color;
	std::vector<bool> hidden;

	MontageTable* montageTable;
	std::vector<int> order;
};

#endif // EVENTTYPETABLE_H
