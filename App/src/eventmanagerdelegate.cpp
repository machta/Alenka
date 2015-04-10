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

	switch (static_cast<EventTable::Column>(index.column()))
	{
	case EventTable::Column::type:
	{
		QComboBox* combo = new QComboBox(parent);

		combo->addItem(EventTypeTable::NO_TYPE_STRING);
		for (int i = 0; i < eventTypeTable->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(eventTypeTable->getName(i)));
		}

		return combo;
	}
	case EventTable::Column::channel:
	{
		QComboBox* combo = new QComboBox(parent);

		combo->addItem(TrackTable::NO_CHANNEL_STRING);
		combo->addItem(TrackTable::ALL_CHANNEL_STRING);
		for (int i = 0; i < trackTable->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(trackTable->getLabel(i)));
		}

		return combo;
	}		
	default:
		break;
	}

	return QStyledItemDelegate::createEditor(parent, option, index);
}

void EventManagerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	switch (static_cast<EventTable::Column>(index.column()))
	{
	case EventTable::Column::type:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(i < 0 ? 0 : i + 1);
		return;
	}
	case EventTable::Column::channel:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(i < -1 ? 0 : i + 1);
		return;
	}
	default:
		break;
	}

	QStyledItemDelegate::setEditorData(editor, index);
}

void EventManagerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	switch (static_cast<EventTable::Column>(index.column()))
	{
	case EventTable::Column::type:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex() - 1);
		return;
	}
	case EventTable::Column::channel:
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex() - 2);
		return;
	}
	default:
		break;
	}

	QStyledItemDelegate::setModelData(editor, model, index);
}

void EventManagerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
