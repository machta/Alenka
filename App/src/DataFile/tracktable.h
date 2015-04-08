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

class TrackTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	TrackTable(QObject* parent = nullptr);
	~TrackTable();

	void write(QXmlStreamWriter* xml) const;
	void read(QXmlStreamReader* xml);
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return label.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;

		return 5;
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

		setData(index(order[i], 0), QString::fromStdString(value));
	}
	std::string getCode(int i) const
	{
		assert(0 <= i && i < static_cast<int>(code.size()));

		return code[i];
	}
	void setCode(const std::string& value, int i)
	{
		assert(0 <= i && i < static_cast<int>(code.size()));

		setData(index(order[i], 1), QString::fromStdString(value));
	}
	QColor getColor(int i) const
	{
		assert(0 <= i && i < static_cast<int>(color.size()));

		return color[i];
	}
	void setColor(QColor value, int i)
	{
		assert(0 <= i && i < static_cast<int>(color.size()));

		setData(index(order[i], 2), value);
	}
	double getAmplitude(int i) const
	{
		assert(0 <= i && i < static_cast<int>(amplitude.size()));

		return amplitude[i];
	}
	void setAmplitude(double value, int i)
	{
		assert(0 <= i && i < static_cast<int>(amplitude.size()));

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
	std::vector<std::string> label;
	std::vector<std::string> code;
	std::vector<QColor> color;
	std::vector<double> amplitude;
	std::vector<bool> hidden;

	std::vector<int> order;
	TrackCodeValidator validator;
};

#endif // TRACKTABLE_H
