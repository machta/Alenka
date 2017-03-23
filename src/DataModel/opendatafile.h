#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include "infotable.h"
#include <AlenkaFile/datafile.h>

class UndoCommandFactory;

struct OpenDataFile
{
	AlenkaFile::DataFile* file;
	const AlenkaFile::DataModel* dataModel;
	UndoCommandFactory* undoFactory;

	static InfoTable infoTable;
};

#endif // OPENDATAFILE_H
