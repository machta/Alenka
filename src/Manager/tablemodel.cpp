#include "tablemodel.h"

#include <QCollator>
#include <QColorDialog>
#include <QLineEdit>
#include <QAction>

using namespace std;

namespace
{

class Delegate : public QStyledItemDelegate
{
public:
	Delegate(std::vector<TableColumn*>* columns, QObject* parent = nullptr) : QStyledItemDelegate(parent), columns(columns)
	{}

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

std::function<bool (int, int)> StringTableColumn::sortPredicate(Qt::SortOrder order) const
{
	QCollator collator;
	collator.setNumericMode(true);

	if (order == Qt::AscendingOrder)
		return [this, &collator] (int a, int b) { return collator.compare(data(a).toString(), data(b).toString()) < 0; };
	else
		return [this, &collator] (int a, int b) { return collator.compare(data(a).toString(), data(b).toString()) > 0; };
}

bool ColorTableColumn::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const
{
	(void)option; (void)index;

	QLineEdit* lineEdit = new QLineEdit(parent);
	QAction* action = lineEdit->addAction(QIcon(":/edit-icon.png"), QLineEdit::TrailingPosition);

	lineEdit->connect(action, &QAction::triggered, [this, lineEdit] ()
	{
		QColor color;
		color.setNamedColor(lineEdit->text());

		color = QColorDialog::getColor(color, lineEdit); // There is a bug in Qt implementation that selects a bad initial color. (https://bugreports.qt.io/browse/QTBUG-44154) TODO: check if this is still true in the new version

		if (color.isValid())
		{
			lineEdit->setText(color.name());

			//emit const_cast<TrackManagerDelegate*>(this)->commitData(lineEdit);
			//emit const_cast<TrackManagerDelegate*>(this)->closeEditor(lineEdit);
		}
	});

	*widget = lineEdit;
	return true;
}

TableModel::TableModel(InfoTable* infoTable, AlenkaFile::DataModel dataModel, QObject* parent) : QAbstractTableModel(parent), infoTable(infoTable), dataModel(dataModel)
{
	delegate = new Delegate(&columns);
}

TableModel::~TableModel()
{
	delete delegate;
}

bool TableModel::insertRows(int row, int count, const QModelIndex& parent)
{
	(void)parent;
	if (count >= 1 && row == rowCount())
	{
		int row = rowCount();

		beginInsertRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			insertRowBack();

			rowOrder.push_back(rowOrder.size());
		}

		endInsertRows();

		return true;
	}
	else
	{
		return false;
	}
}
