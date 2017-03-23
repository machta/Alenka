#include "tracktablemodel.h"

#include "codeeditdialog.h"
#include "../DataModel/opendatafile.h"
#include "../DataModel/vitnessdatamodel.h"
#include "../DataModel/trackcodevalidator.h"

#include <QColor>
#include <QLocale>
#include <QLineEdit>
#include <QAction>
#include <QDoubleSpinBox>

using namespace std;
using namespace AlenkaFile;

namespace
{

AbstractTrackTable* currentTrackTable(DataModel* dataModel)
{
	return dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());
}

class Id : public TableColumn
{
public:
	Id(DataModel* dataModel) : TableColumn("ID", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return row;

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

class Label : public TableColumn
{
public:
	Label(DataModel* dataModel) : TableColumn("Label", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(currentTrackTable(dataModel)->row(row).label);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = currentTrackTable(dataModel)->row(row);
			t.label = value.toString().toStdString();
			currentTrackTable(dataModel)->row(row, t);
			return true;
		}

		return false;
	}
};

class Code : public TableColumn
{
public:
	Code(DataModel* dataModel) : TableColumn("Code", dataModel)
	{
		validator = new TrackCodeValidator();
	}
	virtual ~Code()
	{
		delete validator;
	}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(currentTrackTable(dataModel)->row(row).code);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = currentTrackTable(dataModel)->row(row);
			QString qc = value.toString();
			string c = qc.toStdString();

			if (t.code != c && validator->validate(qc))
			{
				t.code = c;
				currentTrackTable(dataModel)->row(row, t);
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

		lineEdit->connect(action, &QAction::triggered, [lineEdit] () {
			CodeEditDialog dialog(lineEdit);
			dialog.setText(lineEdit->text());
			int result = dialog.exec();

			if (result == QDialog::Accepted)
			{
				lineEdit->setText(dialog.getText());

				// TODO: Decide whether to turn this back on, or leave it out.
				//emit const_cast<TrackManagerDelegate*>(this)->commitData(lineEdit);
				//emit const_cast<TrackManagerDelegate*>(this)->closeEditor(lineEdit);
			}
		});

		*widget = lineEdit;
		return true;
	}

	virtual bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		(void)model; (void)index;

		QLineEdit* lineEdit = reinterpret_cast<QLineEdit*>(editor);
		QString message;

		if (!validator->validate(lineEdit->text(), &message))
		{
			CodeEditDialog::errorMessageDialog(message, editor);
			return true;
		}

		return false;
	}

private:
	TrackCodeValidator* validator;
};

class Color : public ColorTableColumn
{
public:
	Color(DataModel* dataModel) : ColorTableColumn("Color", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::DecorationRole)
		{
			auto colorArray = currentTrackTable(dataModel)->row(row).color;
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
			Track t = currentTrackTable(dataModel)->row(row);
			DataModel::color2array(value.value<QColor>(), t.color);
			currentTrackTable(dataModel)->row(row, t);
			return true;
		}

		return false;
	}
};

class Amplitude : public TableColumn
{
public:
	Amplitude(DataModel* dataModel) : TableColumn("Amplitude", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return currentTrackTable(dataModel)->row(row).amplitude;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = currentTrackTable(dataModel)->row(row);

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

			currentTrackTable(dataModel)->row(row, t);
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
	Hidden(DataModel* dataModel) : BoolTableColumn("Hidden", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return currentTrackTable(dataModel)->row(row).hidden;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Track t = currentTrackTable(dataModel)->row(row);
			t.hidden = value.toBool();
			currentTrackTable(dataModel)->row(row, t);
			return true;
		}

		return false;
	}
};

} // namespace

TrackTableModel::TrackTableModel(OpenDataFile* file, QObject* parent) : TableModel(file, parent)
{
	columns.push_back(new Id(file->dataModel));
	columns.push_back(new Label(file->dataModel));
	columns.push_back(new Code(file->dataModel));
	columns.push_back(new Color(file->dataModel));
	columns.push_back(new Amplitude(file->dataModel));
	columns.push_back(new Hidden(file->dataModel));

	connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(setSelectedMontage(int)));
	setSelectedMontage(OpenDataFile::infoTable.getSelectedMontage());
}

int TrackTableModel::rowCount(const QModelIndex& parent) const
{
	(void)parent;
	return currentTrackTable(file->dataModel)->rowCount();
}


void TrackTableModel::insertRowBack()
{
	int rc = currentTrackTable(file->dataModel)->rowCount();
	currentTrackTable(file->dataModel)->insertRows(rc);
}

void TrackTableModel::removeRowsFromDataModel(int row, int count)
{
	// Update the channels of events to point to correct tracks after the rows are removed.
	for (int i = 0; i < file->dataModel->montageTable()->rowCount(); ++i)
	{
		AbstractEventTable* eventTable = file->dataModel->montageTable()->eventTable(i);

		for (int j = 0; j < eventTable->rowCount(); ++j)
		{
			Event e = eventTable->row(j);

			if (row + count - 1 < e.channel)
				e.channel -= count;
			else if (row <= e.channel && e.channel <= row + count - 1)
				e.channel = -2;

			eventTable->row(j, e);
		}
	}

	currentTrackTable(file->dataModel)->removeRows(row, count);
}

void TrackTableModel::setSelectedMontage(int i)
{
	beginResetModel();

	for (auto e : trackTableConnections)
		disconnect(e);
	trackTableConnections.clear();

	auto vitness = VitnessTrackTable::vitness(file->dataModel->montageTable()->trackTable(i));

	auto c = connect(vitness, &DataModelVitness::valueChanged, [this] (int row, int col) {
		emitDataChanged(row, col + 1);
	});
	trackTableConnections.push_back(c);

	c = connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(insertDataModelRows(int, int)));
	trackTableConnections.push_back(c);

	endResetModel();
}
