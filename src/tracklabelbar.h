#ifndef TRACKLABELBAR_H
#define TRACKLABELBAR_H

#include <QWidget>

class OpenDataFile;

/**
 * @brief This class implements the GUI control responsible for displaying track names.
 */
class TrackLabelBar : public QWidget
{
	Q_OBJECT

public:
	explicit TrackLabelBar(QWidget* parent = nullptr);

	/**
	 * @brief Notifies this object that the DataFile changed.
	 * @param file Pointer to the data file. nullptr means file was closed.
	 */
	void changeFile(OpenDataFile* file);

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
	OpenDataFile* file = nullptr;
	int selectedTrack = -1;
	std::vector<QMetaObject::Connection> trackConnections;

private slots:
	void updateTrackTable(int row);
	void updateLabels(int col);
};

#endif // TRACKLABELBAR_H
