#ifndef MONTAGETABLE_H
#define MONTAGETABLE_H

#include <QAbstractTableModel>

#include "eventtable.h"

#include <QObject>
#include <QColor>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <vector>
#include <string>
#include <sstream>

class MontageTable : public QAbstractTableModel
{
	Q_OBJECT

public:
	MontageTable(QObject* parent = 0);
	~MontageTable();

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
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
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
	std::vector<std::string> code;
	std::vector<QColor> color;
	std::vector<double> amplitude;
	std::vector<bool> hidden;

	EventTable eventTable;
};

#endif // MONTAGETABLE_H
