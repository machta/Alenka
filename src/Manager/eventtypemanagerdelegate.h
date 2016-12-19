#ifndef EVENTTYPEMANAGERDELEGATE_H
#define EVENTTYPEMANAGERDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief Implements custom editors for EventTypeManager.
 */
class EventTypeManagerDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	EventTypeManagerDelegate(QObject* parent = nullptr);
	~EventTypeManagerDelegate();

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // EVENTTYPEMANAGERDELEGATE_H
