#ifndef UNDOCOMMANDFACTORY_H
#define UNDOCOMMANDFACTORY_H

#include "../DataModel/opendatafile.h"

#include <QUndoCommand>

class UndoCommandFactory
{
public:
	UndoCommandFactory(OpenDataFile* file) : file(file) {}

	QUndoCommand* changeEventType(int i, const AlenkaFile::EventType& value, const QString& text = "") const;
	QUndoCommand* changeMontage(int i, const AlenkaFile::Montage& value, const QString& text = "") const;
	QUndoCommand* changeEvent(int i, int j, const AlenkaFile::Event& value, const QString& text = "") const;
	QUndoCommand* changeTrack(int i, int j, const AlenkaFile::Track& value, const QString& text = "") const;

	QUndoCommand* insertEventType(int i, int c = 1, const QString& text = "") const;
	QUndoCommand* insertMontage(int i, int c = 1, const QString& text = "") const;
	QUndoCommand* insertEvent(int i, int j, int c = 1, const QString& text = "") const;
	QUndoCommand* insertTrack(int i, int j, int c = 1, const QString& text = "") const;

	QUndoCommand* removeEventType(int i, int c = 1, const QString& text = "") const;
	QUndoCommand* removeMontage(int i, int c = 1, const QString& text = "") const;
	QUndoCommand* removeEvent(int i, int j, int c = 1, const QString& text = "") const;
	QUndoCommand* removeTrack(int i, int j, int c = 1, const QString& text = "") const;

private:
	OpenDataFile* file;
};

#endif // UNDOCOMMANDFACTORY_H
