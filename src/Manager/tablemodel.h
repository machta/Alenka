#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QColor>

#include <string>
#include <vector>
#include <functional>

const std::string NO_TYPE_STRING = "<No Type>";
const std::string NO_CHANNEL_STRING = "<No Channel>";
const std::string ALL_CHANNEL_STRING = "<All>";

class OpenDataFile;

class TableColumn
{
public:
	TableColumn(const QString& header, OpenDataFile* file) : header(header), file(file) {}
	virtual ~TableColumn() {}

	virtual QVariant headerData() const
	{
		return header;
	}

	virtual QVariant data(int row, int role = Qt::DisplayRole) const = 0;
	virtual bool setData(int row, const QVariant& value, int role = Qt::EditRole) = 0;
	virtual Qt::ItemFlags flags() const
	{
		return Qt::ItemIsEditable;
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
	OpenDataFile* file;
};

class BoolTableColumn : public TableColumn
{
public:
	BoolTableColumn(const QString& header, OpenDataFile* file) : TableColumn(header, file) {}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override;
};

class ColorTableColumn : public TableColumn
{
public:
	ColorTableColumn(const QString& header, OpenDataFile* file) : TableColumn(header, file) {}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override;
};

class TableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit TableModel(OpenDataFile* file, QObject* parent = nullptr);
	~TableModel();

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override = 0;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		(void)parent;
		return static_cast<int>(columns.size());
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal && 0 <= section && section < columnCount())
			return columns[section]->headerData();

		return QVariant();
	}
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
	{
		if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
			return columns[index.column()]->data(index.row(), role);

		return QVariant();
	}
	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
	{
		if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
			return columns[index.column()]->setData(index.row(), value, role);

		return false;
	}
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return Qt::ItemIsEnabled;

		return QAbstractTableModel::flags(index) | columns[index.column()]->flags();
	}
	virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

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
	OpenDataFile* file;
	std::vector<TableColumn*> columns;

	virtual void removeRowsFromDataModel(int row, int count) = 0;

protected slots:
	void emitDataChanged(int row, int col)
	{
		QModelIndex i = index(row, col);
		emit dataChanged(i, i);
	}

	void insertDataModelRows(int row, int count);
	void removeDataModelRows(int row, int count);

private:
	QStyledItemDelegate* delegate;
};

#endif // TABLEMODEL_H
