#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include "infotable.h"
#include <AlenkaFile/datafile.h>

struct OpenDataFile
{
	AlenkaFile::DataFile* file;
	AlenkaFile::DataModel* dataModel; // TODO: Switch from file.getDataFile() to this.

	static InfoTable infoTable;
};

#endif // OPENDATAFILE_H
