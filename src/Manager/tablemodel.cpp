#include "tablemodel.h"

#include <QCollator>
#include <QColorDialog>
#include <QLineEdit>
#include <QAction>

#include <cassert>

using namespace std;

namespace
{

class Delegate : public QStyledItemDelegate
{
public:
	explicit Delegate(std::vector<TableColumn*>* columns, QObject* parent = nullptr) : QStyledItemDelegate(parent), columns(columns) {}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QWidget* widget;
		if ((*columns)[index.column()]->createEditor(parent, option, index, &widget))
			return widget;
		else
			return QStyledItemDelegate::createEditor(parent, option, index);
	}
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		if (!(*columns)[index.column()]->setEditorData(editor, index))
			QStyledItemDelegate::setEditorData(editor, index);
	}
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		if (!(*columns)[index.column()]->setModelData(editor, model, index))
			QStyledItemDelegate::setModelData(editor, model, index);
	}

private:
	std::vector<TableColumn*>* columns;
};

} // namespace

bool BoolTableColumn::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const
{
	(void)parent; (void)option;

	const_cast<QAbstractItemModel*>(index.model())->setData(index, !index.data(Qt::EditRole).toBool());

	// TODO: Decide whether to turn this back on, or leave it out.
	//emit const_cast<TrackManagerDelegate*>(this)->closeEditor(nullptr);

	*widget = nullptr;
	return true;
}

bool ColorTableColumn::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const
{
	(void)option; (void)index;

	QLineEdit* lineEdit = new QLineEdit(parent);
	QAction* action = lineEdit->addAction(QIcon(":/icons/edit.png"), QLineEdit::TrailingPosition);

	lineEdit->connect(action, &QAction::triggered, [lineEdit] () {
		QColor color;
		color.setNamedColor(lineEdit->text());

		color = QColorDialog::getColor(color, lineEdit);

		if (color.isValid())
		{
			lineEdit->setText(color.name());

			// TODO: Decide whether to turn this back on, or leave it out.
			//emit const_cast<TrackManagerDelegate*>(this)->commitData(lineEdit);
			//emit const_cast<TrackManagerDelegate*>(this)->closeEditor(lineEdit);
		}
	});

	*widget = lineEdit;
	return true;
}

TableModel::TableModel(OpenDataFile* file, QObject* parent) : QAbstractTableModel(parent), file(file)
{
	delegate = new Delegate(&columns);
}

TableModel::~TableModel()
{
	delete delegate;

	for (auto e : columns)
		delete e;
}

bool TableModel::removeRows(int row, int count, const QModelIndex& parent)
{
	(void)parent;

	if (count > 0)
	{
		int rowLast = row + count - 1;
		assert(rowLast < rowCount());

		beginRemoveRows(QModelIndex(), row, rowLast);
		removeRowsFromDataModel(row, count);
		endRemoveRows();

		return true;
	}

	return false;
}

void TableModel::insertDataModelRows(int row, int count)
{
	if (0 < count)
	{
		beginInsertRows(QModelIndex(), row, row + count - 1);
		endInsertRows();
	}
}

void TableModel::removeDataModelRows(int row, int count)
{
	if (0 < count)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);
		endRemoveRows();
	}
}
