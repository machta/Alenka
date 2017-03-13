#include "tracktablemodel.h"

#include "codeeditdialog.h"

#include <QColor>
#include <QLocale>
#include <QLineEdit>
#include <QAction>
#include <QDoubleSpinBox>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Id : public TableColumn
{
public:
	Id(InfoTable* infoTable, DataModel dataModel) : TableColumn("ID", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return row;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		(void)row; (void)value; (void)role;
		return false;
	}

	virtual Qt::ItemFlags flags() const override
	{
		return Qt::NoItemFlags;
	}
};

class Label : public StringTableColumn
{
public:
	Label(InfoTable* infoTable, DataModel dataModel) : StringTableColumn("Label", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return QString::fromStdString(dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row).label);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row);
			t.label = value.toString().toStdString();
			dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row, t);
			return true;
		}

		return false;
	}
};

class Code : public StringTableColumn
{
public:
	Code(InfoTable* infoTable, DataModel dataModel) : StringTableColumn("Code", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return QString::fromStdString(dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row).code);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row);
			string c = value.toString().toStdString();
			if (t.code != c /*&& validateTrackCode(c)*/)
			{
				t.code = c;
				dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row, t);
				return true;
			}
		}

		return false;
	}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override
	{
		(void)option; (void)index;

		QLineEdit* lineEdit = new QLineEdit(parent);

		QAction* action = lineEdit->addAction(QIcon(":/edit-icon.png"), QLineEdit::TrailingPosition);

		lineEdit->connect(action, &QAction::triggered, [lineEdit] ()
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

		*widget = lineEdit;
		return true;
	}
	// TODO: Revise code validation.
//	virtual bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
//	{
//		QLineEdit* lineEdit = reinterpret_cast<QLineEdit*>(editor);
//		QString message;

//		TrackTable* tt = reinterpret_cast<TrackTable*>(model);
//		if (tt->validateTrackCode(lineEdit->text(), &message))
//		{
//			return false;
//		}
//		else
//		{
//			CodeEditDialog::errorMessageDialog(message, editor);
//			return true;
//		}
//	}
};

class Color : public ColorTableColumn
{
public:
	Color(InfoTable* infoTable, DataModel dataModel) : ColorTableColumn("Color", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::DecorationRole)
		{
			auto colorArray = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row).color;
			QColor color;
			color.setRgb(colorArray[0], colorArray[1], colorArray[2]);
			return color;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row);

			QColor color = value.value<QColor>();
			t.color[0] = color.red();
			t.color[1] = color.green();
			t.color[2] = color.blue();

			dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row, t);
			return true;
		}

		return false;
	}
};

class Amplitude : public TableColumn
{
public:
	Amplitude(InfoTable* infoTable, DataModel dataModel) : TableColumn("Amplitude", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row).amplitude;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row);

			QLocale locale;
			bool ok;
			double tmp = value.toDouble(&ok);
			if (ok)
			{
				if (t.amplitude == tmp)
					return false;
				t.amplitude = tmp;
			}
			else
			{
				tmp = locale.toDouble(value.toString());
				if (t.amplitude == tmp)
					return false;
				t.amplitude = tmp;
			}

			dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row, t);
			return true;
		}

		return false;
	}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override
	{
		(void)option; (void)index;

		QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
		spinBox->setDecimals(10);
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

		*widget = spinBox;
		return true;
	}
};

class Hidden : public BoolTableColumn
{
public:
	Hidden(InfoTable* infoTable, DataModel dataModel) : BoolTableColumn("Hidden", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row).hidden;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row);
			t.hidden = value.toBool();
			dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(row, t);
			return true;
		}

		return false;
	}
};

} // namespace

TrackTableModel::TrackTableModel(InfoTable* infoTable, DataModel dataModel, QObject* parent) : TableModel(infoTable, dataModel, parent)
{
	columns.push_back(new Id(infoTable, dataModel));
	columns.push_back(new Label(infoTable, dataModel));
	columns.push_back(new Code(infoTable, dataModel));
	columns.push_back(new Color(infoTable, dataModel));
	columns.push_back(new Amplitude(infoTable, dataModel));
	columns.push_back(new Hidden(infoTable, dataModel));
}
