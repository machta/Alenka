#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include <QWidget>

#include "filtervisualizer.h"

class OpenDataFile;

class FilterManager : public QWidget
{
	Q_OBJECT

public:
	explicit FilterManager(QWidget* parent = nullptr);

	void changeFile(OpenDataFile* file)
	{
		filterVisulizer->changeFile(file);
	}

private:
	FilterVisualizer* filterVisulizer;
};

#endif // FILTERMANAGER_H
