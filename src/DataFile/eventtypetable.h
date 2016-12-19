#ifndef EVENTTYPETABLE_H
#define EVENTTYPETABLE_H

#include <QAbstractTableModel>

#include <QColor>

#include <string>
#include <sstream>
#include <cassert>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;
class MontageTable;
class DataFile;

/**
 * @brief A data structure for handling event types.
 *
 * This class stores info about event types.
 *
 * Two interfaces are available for accessing the underlying data:
 * * direct access via get/set functions
 * * interface inherited from QAbstractTableModel
 */
class EventTypeTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	/**
	 * @brief Enum defining symbolic constants for the table columns and the number of columns.
	 */
	enum class Column
	{
		id, name, opacity, color, hidden, size
	};

	EventTypeTable(DataFile* file, QObject* parent = nullptr);
	~EventTypeTable();

	/**
	 * @brief Sets pointers to related objects.
	 */
	void setReferences(MontageTable* montageTable)
	{
		this->montageTable = montageTable;
	}

	/**
	 * @brief Writes an eventTypeTable element.
	 */
	void write(QXmlStreamWriter* xml) const;

	/**
	 * @brief Parses the eventTypeTable element.
	 */
	void read(QXmlStreamReader* xml);

	MontageTable* getMontageTable()
	{
		return montageTable;
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

	/**
	 * @brief A string to be used by GUI controls to represent unknown type option.
	 */
	static const char* const NO_TYPE_STRING;

private:
	DataFile* file;

	std::vector<int> id;
	std::vector<std::string> name;
	std::vector<double> opacity;
	std::vector<QColor> color;
	std::vector<bool> hidden;

	MontageTable* montageTable;
	std::vector<int> order;
};

#endif // EVENTTYPETABLE_H
