#ifndef OPENDATAFILE_H
#define OPENDATAFILE_H

#include <QObject>

#include "infotable.h"
#include <AlenkaFile/datafile.h>
#include "kernelcache.h"

#include <vector>

class UndoCommandFactory;

struct OpenDataFile
{
	AlenkaFile::DataFile* file = nullptr;
	const AlenkaFile::DataModel* dataModel = nullptr;
	UndoCommandFactory* undoFactory = nullptr;
	KernelCache* kernelCache = nullptr;

	static InfoTable infoTable;	
};

#endif // OPENDATAFILE_H
