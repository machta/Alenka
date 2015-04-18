#include "trackmanagerdelegate.h"

#include "codeeditdialog.h"
#include "../DataFile/tracktable.h"
#include "../DataFile/trackcodevalidator.h"

#include <QColorDialog>
#include <QLineEdit>
#include <QAction>
#include <QDoubleSpinBox>
#include <QInputDialog>

#include <limits>

TrackManagerDelegate::TrackManagerDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

TrackManagerDelegate::~TrackManagerDelegate()
{
}

QWidget* TrackManagerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	switch (static_cast<TrackTable::Column>(index.column()))
	{
	case TrackTable::Column::code:
	{
		QLineEdit* lineEdit = new QLineEdit(parent);

		QAction* action = lineEdit->addAction(QIcon("edit.png"), QLineEdit::TrailingPosition);

		connect(action, &QAction::triggered, [lineEdit] ()
		{
			CodeEditDialog dialog(lineEdit);
			dialog.setText(lineEdit->text());
			int result = dialog.exec();

			if (result == QDialog::Accepted)
			{
				lineEdit->setText(dialog.getText());

				//emit const_cast<TrackManagerDelegate*>(this)->commitData(lineEdit);
				//emit const_cast<TrackManagerDelegate*>(this)->closeEditor(lineEdit);
			}
		});

		return lineEdit;
	}
	case TrackTable::Column::color:
	{
		QLineEdit* lineEdit = new QLineEdit(parent);

		QAction* action = lineEdit->addAction(QIcon("edit.png"), QLineEdit::TrailingPosition);

		connect(action, &QAction::triggered, [this, lineEdit] ()
		{
			QColor color;
			color.setNamedColor(lineEdit->text());

			color = QColorDialog::getColor(color, lineEdit); // There is a bug in Qt implementation that selects a bad initial color. (https://bugreports.qt.io/browse/QTBUG-44154)

			if (color.isValid())
			{
				lineEdit->setText(color.name());

				emit const_cast<TrackManagerDelegate*>(this)->commitData(lineEdit);
				emit const_cast<TrackManagerDelegate*>(this)->closeEditor(lineEdit);
			}
		});

		return lineEdit;
	}
	case TrackTable::Column::amplitude:
	{
		QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
		spinBox->setDecimals(10);
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
		return spinBox;
	}
	case TrackTable::Column::hidden:
	{
		const_cast<QAbstractItemModel*>(index.model())->setData(index, !index.data(Qt::EditRole).toBool());

		emit const_cast<TrackManagerDelegate*>(this)->closeEditor(nullptr);

		return nullptr;
	}
	default:
		break;
	}

	return QStyledItemDelegate::createEditor(parent, option, index);
}

void TrackManagerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	QStyledItemDelegate::setEditorData(editor, index);
}

void TrackManagerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	switch (static_cast<TrackTable::Column>(index.column()))
	{
	case TrackTable::Column::code:
	{
		QLineEdit* lineEdit = reinterpret_cast<QLineEdit*>(editor);
		QString message;

		TrackTable* tt = reinterpret_cast<TrackTable*>(model);
		if (tt->validateTrackCode(lineEdit->text(), &message))
		{
			break;
		}
		else
		{
			CodeEditDialog::errorMessageDialog(message, editor);
			return;
		}
	}		
	default:
		break;
	}

	QStyledItemDelegate::setModelData(editor, model, index);
}

void TrackManagerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}
