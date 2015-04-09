#ifndef TRACKTABLE_H
#define TRACKTABLE_H

#include <QAbstractTableModel>

#include "../SignalProcessor/montage.h"
#include "trackcodevalidator.h"

#include <QColor>

#include <vector>
#include <string>
#include <sstream>
#include <cassert>

class QXmlStreamReader;
class QXmlStreamWriter;
class EventTable;

class TrackTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum class Collumn
	{
		label, code, color, amplitude, hidden, collumnCount
	};

	TrackTable(QObject* parent = nullptr);
	~TrackTable();

	void setReferences(EventTable* eventTable)
	{
		this->eventTable = eventTable;
	}
	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	EventTable* getEventTable()
	{
		return eventTable;
	}
	bool insertRowsBack(int count = 1);

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return label.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return static_cast<int>(Collumn::collumnCount);
	}
	std::vector<std::string> getCode() const;
	bool validateTrackCode(const QString& code, QString* message = nullptr)
	{
		return validator.validate(code, message);
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

	static const char* NO_CHANNEL_STRING;
	static const char* ALL_CHANNEL_STRING;

private:
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
