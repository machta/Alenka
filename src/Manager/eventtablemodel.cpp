#include "eventtablemodel.h"

#include <AlenkaFile/datafile.h>
#include "../DataModel/opendatafile.h"
#include "../signalfilebrowserwindow.h"
#include "../DataModel/vitnessdatamodel.h"

#include <QLocale>
#include <QComboBox>

using namespace std;
using namespace AlenkaFile;

namespace
{

AbstractTrackTable* currentTrackTable(DataModel* dataModel)
{
	return dataModel->montageTable()->trackTable(OpenDataFile::infoTable.getSelectedMontage());
}

AbstractEventTable* currentEventTable(DataModel* dataModel)
{
	return dataModel->montageTable()->eventTable(OpenDataFile::infoTable.getSelectedMontage());
}

class Label : public TableColumn
{
public:
	Label(DataModel* dataModel) : TableColumn("Label", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(currentEventTable(dataModel)->row(row).label);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = currentEventTable(dataModel)->row(row);
			e.label = value.toString().toStdString();
			currentEventTable(dataModel)->row(row, e);
			return true;
		}

		return false;
	}
};

class Type : public TableColumn
{
public:
	Type(DataModel* dataModel) : TableColumn("Type", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		int type = currentEventTable(dataModel)->row(row).type;

		if (role == Qt::DisplayRole)
			return QString::fromStdString(type < 0 ? NO_TYPE_STRING : dataModel->eventTypeTable()->row(type).name);
		else if (role == Qt::EditRole)
			return type;
		else if (role == Qt::DecorationRole && 0 <= type)
			return DataModel::array2color<QColor>(dataModel->eventTypeTable()->row(type).color);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = currentEventTable(dataModel)->row(row);
			e.type = value.toInt();
			currentEventTable(dataModel)->row(row, e);
			return true;
		}

		return false;
	}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override
	{
		(void)option; (void)index;

		QComboBox* combo = new QComboBox(parent);

		combo->addItem(NO_TYPE_STRING.c_str());
		for (int i = 0; i < dataModel->eventTypeTable()->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(dataModel->eventTypeTable()->row(i).name));
		}

		*widget = combo;
		return true;
	}
	virtual bool setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(i + 1);
		return true;
	}
	virtual bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex() - 1);
		return true;
	}
};

class Position : public TableColumn
{
public:
	Position(DataModel* dataModel, DataFile* file) : TableColumn("Position", dataModel), file(file) {}

	virtual QVariant data(int row, int role) const override
	{
		int position = currentEventTable(dataModel)->row(row).position;

		if (role == Qt::DisplayRole)
			return SignalFileBrowserWindow::sampleToDateTimeString(file, position);
		else if (role == Qt::EditRole)
			return position;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = currentEventTable(dataModel)->row(row);
			e.position = value.toDouble();
			currentEventTable(dataModel)->row(row, e);
			return true;
		}

		return false;
	}

private:
	DataFile* file;
};

class Duration : public TableColumn
{
public:
	Duration(DataModel* dataModel, DataFile* file) : TableColumn("Duration", dataModel), file(file) {}

	virtual QVariant data(int row, int role) const override
	{
		int duration = currentEventTable(dataModel)->row(row).duration;

		if (role == Qt::DisplayRole)
			return SignalFileBrowserWindow::sampleToDateTimeString(file, duration);
		else if (role == Qt::EditRole)
			return duration;

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = currentEventTable(dataModel)->row(row);
			e.duration = value.toDouble();
			currentEventTable(dataModel)->row(row, e);
			return true;
		}

		return false;
	}

private:
	DataFile* file;
};

class Channel : public TableColumn
{
public:
	Channel(DataModel* dataModel) : TableColumn("Channel", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		int channel = currentEventTable(dataModel)->row(row).channel;

		if (role == Qt::DisplayRole)
		{
			string str;
			if (channel < 0)
			{
				if (channel == -1)
					str = ALL_CHANNEL_STRING;
				else
					str = NO_CHANNEL_STRING;
			}
			else
			{
				str = currentTrackTable(dataModel)->row(channel).label;
			}

			return QString::fromStdString(str);
		}
		else if (role == Qt::EditRole)
		{
			return channel;
		}
		else if (role == Qt::DecorationRole && 0 <= channel)
		{
			return DataModel::array2color<QColor>(currentTrackTable(dataModel)->row(channel).color);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = currentEventTable(dataModel)->row(row);
			e.channel = value.toInt();
			currentEventTable(dataModel)->row(row, e);
			return true;
		}

		return false;
	}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override
	{
		(void)option; (void)index;

		QComboBox* combo = new QComboBox(parent);

		combo->addItem(NO_CHANNEL_STRING.c_str());
		combo->addItem(ALL_CHANNEL_STRING.c_str());

		for (int i = 0; i < currentTrackTable(dataModel)->rowCount(); ++i)
			combo->addItem(QString::fromStdString(currentTrackTable(dataModel)->row(i).label));

		*widget = combo;
		return true;
	}
	virtual bool setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(i + 2);
		return true;
	}
	virtual bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex() - 2);
		return true;
	}
};

class Description: public TableColumn
{
public:
	Description(DataModel* dataModel) : TableColumn("Description", dataModel) {}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
			return QString::fromStdString(currentEventTable(dataModel)->row(row).description);

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = currentEventTable(dataModel)->row(row);
			e.description= value.toString().toStdString();
			currentEventTable(dataModel)->row(row, e);
			return true;
		}

		return false;
	}
};

} // namespace

EventTableModel::EventTableModel(DataFile* file, QObject* parent) : TableModel(file, parent)
{
	columns.push_back(new Label(file->getDataModel()));
	columns.push_back(new Type(file->getDataModel()));
	columns.push_back(new Position(file->getDataModel(), file));
	columns.push_back(new Duration(file->getDataModel(), file));
	columns.push_back(new Channel(file->getDataModel()));
	columns.push_back(new Description(file->getDataModel()));

	connect(&OpenDataFile::infoTable, SIGNAL(selectedMontageChanged(int)), this, SLOT(setSelectedMontage(int)));
	setSelectedMontage(OpenDataFile::infoTable.getSelectedMontage());

	connect(&OpenDataFile::infoTable, SIGNAL(timeModeChanged(int)), this, SLOT(beginEndReset()));
}

int EventTableModel::rowCount(const QModelIndex& parent) const
{
	(void)parent;
	return currentEventTable(file->getDataModel())->rowCount();
}

void EventTableModel::removeRowsFromDataModel(int row, int count)
{
	currentEventTable(file->getDataModel())->removeRows(row, count);
}

void EventTableModel::insertRowBack()
{
	int rc = currentEventTable(file->getDataModel())->rowCount();
	currentEventTable(file->getDataModel())->insertRows(rc);
}

void EventTableModel::setSelectedMontage(int i)
{
	beginResetModel();

	for (auto e : montageTableConnections)
		disconnect(e);
	montageTableConnections.clear();

	auto vitness = VitnessEventTable::vitness(file->getDataModel()->montageTable()->eventTable(i));

	auto c = connect(vitness, SIGNAL(valueChanged(int, int)), this, SLOT(emitDataChanged(int, int)));
	montageTableConnections.push_back(c);

	c = connect(vitness, SIGNAL(rowsInserted(int, int)), this, SLOT(insertDataModelRows(int, int)));
	montageTableConnections.push_back(c);

	endResetModel();
}
