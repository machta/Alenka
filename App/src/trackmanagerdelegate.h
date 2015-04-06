#ifndef TRACKMANAGERDELEGATE_H
#define TRACKMANAGERDELEGATE_H

#include <QStyledItemDelegate>

class TrackManagerDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	TrackManagerDelegate(QObject* parent = nullptr);
	~TrackManagerDelegate();

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // TRACKMANAGERDELEGATE_H
