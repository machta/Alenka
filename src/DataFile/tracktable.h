#ifndef TRACKTABLE_H
#define TRACKTABLE_H

#include <QAbstractTableModel>

//#include "montage.h"
#include "trackcodevalidator.h"

#include <QColor>

#include <string>
#include <sstream>
#include <cassert>
#include <vector>

class QXmlStreamReader;
class QXmlStreamWriter;
class EventTable;
class DataFile;

/**
 * @brief A data structure for handling tracks.
 *
 * This class stores info about tracks.
 *
 * Two interfaces are available for accessing the underlying data:
 * * direct access via get/set functions
 * * interface inherited from QAbstractTableModel
 */
class TrackTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	/**
	 * @brief Enum defining symbolic constants for the table columns and the number of columns.
	 */
	enum class Column
	{
		label, code, color, amplitude, hidden, size
	};

	TrackTable(DataFile* file, QObject* parent = nullptr);
	~TrackTable();

	/**
	 * @brief Sets pointers to related objects.
	 */
	void setReferences(EventTable* eventTable)
	{
		this->eventTable = eventTable;
	}

	/**
	 * @brief Writes an trackTable element.
	 */
	void write(QXmlStreamWriter* xml) const;

	/**
	 * @brief Parses the trackTable element.
	 */
	void read(QXmlStreamReader* xml);

	EventTable* getEventTable()
	{
		return eventTable;
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

	/**
	 * @brief Return the code for montage tracks.
	 *
	 * Hidden tracks are excluded.
	 */
	std::vector<std::string> getCode() const;

	/**
	 * @brief Test code with TrackCodeValidator.
	 * @param message [out]
	 * @return True if the test succeeds.
	 */
	bool validateTrackCode(const QString& code, QString* message = nullptr)
	{
		return validator.validate(code, message);
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
	std::string getCode(int i) const
	{
		assert(0 <= i && i < static_cast<int>(code.size()));

		return code[i];
	}
	void setCode(const std::string& value, int i)
	{
		assert(0 <= i && i < static_cast<int>(code.size()));

		code[i] = value;
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
	double getAmplitude(int i) const
	{
		assert(0 <= i && i < static_cast<int>(amplitude.size()));

		return amplitude[i];
	}
	void setAmplitude(double value, int i)
	{
		assert(0 <= i && i < static_cast<int>(amplitude.size()));

		amplitude[i] = value;
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
	 * @brief A string to be used by GUI controls to represent unknown channel option.
	 */
	static const char* const NO_CHANNEL_STRING;
	/**
	 * @brief A string to be used by GUI controls to represent all-channel option.
	 */
	static const char* const ALL_CHANNEL_STRING;

private:
	DataFile* file;

	std::vector<std::string> label;
	std::vector<std::string> code;
	std::vector<QColor> color;
	std::vector<double> amplitude;
	std::vector<bool> hidden;

	EventTable* eventTable;
	std::vector<int> order;
	TrackCodeValidator validator;
};

#endif // TRACKTABLE_H
