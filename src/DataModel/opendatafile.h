#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include <QObject>

#include "infotable.h"
#include <AlenkaFile/datafile.h>

#include <vector>

class UndoCommandFactory;

class OpenDataFile : public QObject
{
	Q_OBJECT

public:
	explicit OpenDataFile(QObject* parent = nullptr) : QObject(parent) {}

	const std::vector<float>& getFilterCoefficients() const
	{
		return filterCoefficients;
	}
	void setFilterCoefficients(const std::vector<float>& c)
	{
		filterCoefficients = c;
		emit filterCoefficientsChanged();
	}

	AlenkaFile::DataFile* file;
	const AlenkaFile::DataModel* dataModel;
	UndoCommandFactory* undoFactory;

	static InfoTable infoTable;

signals:
	void filterCoefficientsChanged();

private:
	std::vector<float> filterCoefficients;
};

#endif // OPENDATAFILE_H
