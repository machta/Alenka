#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <AlenkaFile/datamodel.h>
#include "../DataFile/infotable.h"

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QColor>

#include <string>
#include <cassert>
#include <vector>
#include <functional>

const std::string NO_TYPE_STRING = "<No Type>";
const std::string NO_CHANNEL_STRING = "<No Channel>";
const std::string ALL_CHANNEL_STRING = "<All>";

class TableColumn
{
public:
	TableColumn(const QString& header, InfoTable* infoTable, AlenkaFile::DataModel dataModel) : header(header), infoTable(infoTable), dataModel(dataModel)
	{}

	virtual QVariant headerData() const
	{
		return QString(header);
	}

	virtual QVariant data(int row, int role = Qt::DisplayRole) const = 0;
	virtual bool setData(int row, const QVariant& value, int role = Qt::EditRole) = 0;
	virtual Qt::ItemFlags flags() const
	{
		return Qt::ItemIsEditable;
	}

	virtual std::function<bool (int, int)> sortPredicate(Qt::SortOrder order) const
	{
		if (order == Qt::AscendingOrder)
			return [this] (int a, int b) { return data(a) < data(b); };
		else
			return [this] (int a, int b) { return data(a) > data(b); };
	}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const
	{
		(void)parent; (void)option; (void)index; (void)widget;
		return false;
	}
	virtual bool setEditorData(QWidget* editor, const QModelIndex& index) const
	{
		(void)editor; (void)index;
		return false;
	}
	virtual bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		(void)editor; (void)model; (void)index;
		return false;
	}

protected:
	QString header;
	InfoTable* infoTable;
	AlenkaFile::DataModel dataModel;
};

class StringTableColumn : public TableColumn
{
public:
	StringTableColumn(const QString& header, InfoTable* infoTable, AlenkaFile::DataModel dataModel) : TableColumn(header, infoTable, dataModel)
	{}

	virtual std::function<bool (int, int)> sortPredicate(Qt::SortOrder order) const override;
};

class BoolTableColumn : public TableColumn
{
public:
	BoolTableColumn(const QString& header, InfoTable* infoTable, AlenkaFile::DataModel dataModel) : TableColumn(header, infoTable, dataModel)
	{}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override
	{
		(void)parent; (void)option;

		const_cast<QAbstractItemModel*>(index.model())->setData(index, !index.data(Qt::EditRole).toBool());

		//emit const_cast<TrackManagerDelegate*>(this)->closeEditor(nullptr);

		*widget = nullptr;
		return true;
	}
};

class ColorTableColumn : public TableColumn
{
public:
	ColorTableColumn(const QString& header, InfoTable* infoTable, AlenkaFile::DataModel dataModel) : TableColumn(header, infoTable, dataModel)
	{}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override;
};

class TableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	TableModel(InfoTable* infoTable, AlenkaFile::DataModel dataModel, QObject* parent = nullptr);
	~TableModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;
		return rowOrder.size();
	}
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;
		return columns.size();
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
	{
		(void)orientation; (void)role;
		return columns[section]->headerData();
	}
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
	{
		int row = rowOrder[index.row()];
		return columns[index.column()]->data(row, role);
	}
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
	{
		int row = rowOrder[index.row()];
		return columns[index.column()]->setData(row, value, role);
	}
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return Qt::ItemIsEnabled;
		else
			return QAbstractTableModel::flags(index) | columns[index.column()]->flags();
	}
	virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
	virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
	{
		(void)row; (void)count; (void)parent;
		// TODO: implement this
		return false;
	}
	virtual void sort(int column, Qt::SortOrder order) override
	{
		std::function<bool (int, int)> predicate = columns[column]->sortPredicate(order);

		emit layoutAboutToBeChanged();

		std::sort(this->rowOrder.begin(), this->rowOrder.end(), predicate);

		changePersistentIndex(index(0, 0), index(rowCount() - 1, columnCount() - 1));

		emit layoutChanged();
	}

	QStyledItemDelegate* getDelegate()
	{
		return delegate;
	}

public slots:
	void emitColumnChanged(int column)
	{
		int c = static_cast<int>(column);
		emit dataChanged(index(0, c), index(rowCount() - 1, c));
	}

protected:
	InfoTable* infoTable;
	AlenkaFile::DataModel dataModel;
	std::vector<TableColumn*> columns;
	std::vector<int> rowOrder;

	virtual void insertRowBack() = 0;

private:
	QStyledItemDelegate* delegate;
};

#endif // TABLEMODEL_H
