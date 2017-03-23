#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include "infotable.h"
#include <AlenkaFile/datafile.h>

class QUndoStack;

struct OpenDataFile
{
	AlenkaFile::DataFile* file;
	AlenkaFile::DataModel* dataModel;
	QUndoStack* undoStack;

	static InfoTable infoTable;
};

#endif // OPENDATAFILE_H
