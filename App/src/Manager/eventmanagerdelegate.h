#ifndef EVENTMANAGERDELEGATE_H
#define EVENTMANAGERDELEGATE_H

#include <QStyledItemDelegate>

class EventManagerDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	EventManagerDelegate(QObject* parent = nullptr);
	~EventManagerDelegate();

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // EVENTMANAGERDELEGATE_H
