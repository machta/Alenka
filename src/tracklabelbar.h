#ifndef TRACKLABELBAR_H
#define TRACKLABELBAR_H

#include <QWidget>

#include "DataFile/infotable.h"

class DataFile;
class MontageTable;
class TrackTable;

/**
 * @brief This class implements the GUI control responsible for displaying track names.
 */
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
	void updateTrackTable();
	void updateLabels(const QModelIndex& topLeft, const QModelIndex& bottomRight);
};

#endif // TRACKLABELBAR_H
