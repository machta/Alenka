#include "eventtablemodel.h"

#include "../DataFile/datafile.h"

#include <QLocale>
#include <QComboBox>

using namespace std;
using namespace AlenkaFile;

namespace
{

class Label : public StringTableColumn
{
public:
	Label(InfoTable* infoTable, DataModel dataModel) : StringTableColumn("Label", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return QString::fromStdString(dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).label);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row);
			e.label = value.toString().toStdString();
			dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row, e);
			return true;
		}

		return false;
	}
};

class Type : public TableColumn
{
public:
	Type(InfoTable* infoTable, DataModel dataModel) : TableColumn("Type", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole)
		{
			return dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).type;
		}

		if (role == Qt::EditRole)
		{
			int type = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).type;
			return QString::fromStdString(type < 0 ?  NO_TYPE_STRING : dataModel.eventTypeTable->row(type).name);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row);
			e.type = value.toInt();
			dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row, e);
			return true;
		}

		return false;
	}

	virtual bool createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index, QWidget** widget) const override
	{
		(void)option; (void)index;

		QComboBox* combo = new QComboBox(parent);

		combo->addItem(NO_TYPE_STRING.c_str());
		for (int i = 0; i < dataModel.eventTypeTable->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(dataModel.eventTypeTable->row(i).name));
		}

		*widget = combo;
		return true;
	}
	virtual bool setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(i < 0 ? 0 : i + 1);
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
	Position(InfoTable* infoTable, DataModel dataModel) : TableColumn("Position", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
//		if (role == Qt::DisplayRole)
//		{
//			file->sampleToDateTimeString(position[row]);
//		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).position;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row);
			e.position = value.toDouble();
			dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row, e);
			return true;
		}

		return false;
	}
};

class Duration : public TableColumn
{
public:
	Duration(InfoTable* infoTable, DataModel dataModel) : TableColumn("Duration", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
//		if (role == Qt::DisplayRole)
//		{
//			file->sampleToDateTimeString(duration[row]);
//		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).duration;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row);
			e.duration = value.toDouble();
			dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row, e);
			return true;
		}

		return false;
	}
};

class Channel : public TableColumn
{
public:
	Channel(InfoTable* infoTable, DataModel dataModel) : TableColumn("Channel", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		int channel = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).channel;

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
				str = dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(channel).label;
			}
			return QString::fromStdString(str);
		}

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return channel;
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row);
			e.channel = value.toInt();
			dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row, e);
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
		for (int i = 0; i < dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->rowCount(); ++i)
		{
			combo->addItem(QString::fromStdString(dataModel.montageTable->trackTable(infoTable->getSelectedMontage())->row(i).label));
		}

		*widget = combo;
		return true;
	}
	virtual bool setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		int i = index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(i < -1 ? 0 : i + 1);
		return true;
	}
	virtual bool setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		QComboBox* combo = reinterpret_cast<QComboBox*>(editor);
		model->setData(index, combo->currentIndex() - 2);
		return true;
	}
};

class Description: public StringTableColumn
{
public:
	Description(InfoTable* infoTable, DataModel dataModel) : StringTableColumn("Description", infoTable, dataModel)
	{}

	virtual QVariant data(int row, int role) const override
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return QString::fromStdString(dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row).description);
		}

		return QVariant();
	}

	virtual bool setData(int row, const QVariant& value, int role) override
	{
		if (role == Qt::EditRole)
		{
			Event e = dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row);
			e.description= value.toString().toStdString();
			dataModel.montageTable->eventTable(infoTable->getSelectedMontage())->row(row, e);
			return true;
		}

		return false;
	}
};

} // namespace

EventTableModel::EventTableModel(InfoTable* infoTable, DataModel dataModel, QObject* parent) : TableModel(infoTable, dataModel, parent)
{
	columns.push_back(new Label(infoTable, dataModel));
	columns.push_back(new Type(infoTable, dataModel));
	columns.push_back(new Position(infoTable, dataModel));
	columns.push_back(new Duration(infoTable, dataModel));
	columns.push_back(new Channel(infoTable, dataModel));
	columns.push_back(new Description(infoTable, dataModel));
}
