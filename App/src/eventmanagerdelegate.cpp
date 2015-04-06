#include "eventmanagerdelegate.h"

#include "DataFile/eventtable.h"
#include "DataFile/eventtypetable.h"
#include "DataFile/tracktable.h"

#include <QComboBox>

EventManagerDelegate::EventManagerDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

EventManagerDelegate::~EventManagerDelegate()
{
}

QWidget* EventManagerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const EventTable* eventTable = reinterpret_cast<const EventTable*>(index.model());
	const EventTypeTable* eventTypeTable = eventTable->getEventTypeTable();
	const TrackTable* trackTable = eventTable->getTrackTable();

	switch (index.column())
	{
	case 1:
	{
		QComboBox* combo = new QComboBox(parent);

		for (int i = 0; i < eventTypeTable->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(eventTypeTable->getName(i)));
		}

		return combo;
	}
	case 4:
	{
		QComboBox* combo = new QComboBox(parent);

		combo->addItem("All");
		for (int i = 0; i < trackTable->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(trackTable->getLabel(i)));
		}

		return combo;
	}
	}

	return QStyledItemDelegate::createEditor(parent, option, index);
}

void EventManagerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	switch (index.column())
	{
	case 1:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		combo->setCurrentIndex(index.model()->data(index, Qt::EditRole).toInt());
		return;
	}
	case 4:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.model()->data(index, Qt::EditRole).toInt();
		combo->setCurrentIndex(i < 0 ? 0 : i + 1);
		return;
	}
	}

	QStyledItemDelegate::setEditorData(editor, index);
}

void EventManagerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	switch (index.column())
	{
	case 1:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex());
		return;
	}
	case 4:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex() - 1);
		return;
	}
	}

	QStyledItemDelegate::setModelData(editor, model, index);
}

void EventManagerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
