#ifndef TRACKLABELBAR_H
#define TRACKLABELBAR_H

#include <QWidget>

#include "DataFile/infotable.h"
#include "DataFile/montagetable.h"
#include "DataFile/tracktable.h"

class DataFile;

class TrackLabelBar : public QWidget
{
	Q_OBJECT

public:
	explicit TrackLabelBar(QWidget* parent = nullptr);
	~TrackLabelBar();

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(DataFile* file);

signals:

public slots:
	void setSelectedTrack(int value)
	{
		selectedTrack = value;
		update();
	}

protected:
	virtual void paintEvent(QPaintEvent* event) override;

private:
	InfoTable* infoTable = nullptr;
	InfoTable defaultInfoTable;
	MontageTable* montageTable = nullptr;
	TrackTable* trackTable;
	int selectedTrack = -1;

	InfoTable* getInfoTable()
	{
		if (infoTable != nullptr)
		{
			return infoTable;
		}
		else
		{
			return &defaultInfoTable;
		}
	}

private slots:
	void updateTrackTable()
	{
		trackTable = montageTable->getTrackTables()->at(getInfoTable()->getSelectedMontage());

		connect(trackTable, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(updateLabels(QModelIndex, QModelIndex)));
		connect(trackTable, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(update()));
		connect(trackTable, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(update()));
	}
	void updateLabels(const QModelIndex& topLeft, const QModelIndex& bottomRight)
	{
		int column = static_cast<int>(TrackTable::Column::label);
		if (topLeft.column() <= column && column <= bottomRight.column())
		{
			update();
		}
	}
};

#endif // TRACKLABELBAR_H
