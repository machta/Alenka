#ifndef UNDOCOMMANDFACTORY_H
#define UNDOCOMMANDFACTORY_H

#include <AlenkaFile/abstractdatamodel.h>

#include <QString>

class QUndoStack;

class UndoCommandFactory
{
public:
	UndoCommandFactory(AlenkaFile::DataModel* dataModel, QUndoStack* undoStack) :
		dataModel(dataModel), undoStack(undoStack) {}

	void beginMacro(const QString &text);
	void endMacro();

	void changeEventType(int i, const AlenkaFile::EventType& value, const QString& text = "") const;
	void changeMontage(int i, const AlenkaFile::Montage& value, const QString& text = "") const;
	void changeEvent(int i, int j, const AlenkaFile::Event& value, const QString& text = "") const;
	void changeTrack(int i, int j, const AlenkaFile::Track& value, const QString& text = "") const;

	void insertEventType(int i, int c = 1, const QString& text = "") const;
	void insertMontage(int i, int c = 1, const QString& text = "") const;
	void insertEvent(int i, int j, int c = 1, const QString& text = "") const;
	void insertTrack(int i, int j, int c = 1, const QString& text = "") const;

	void removeEventType(int i, int c = 1, const QString& text = "") const;
	void removeMontage(int i, int c = 1, const QString& text = "") const;
	void removeEvent(int i, int j, int c = 1, const QString& text = "") const;
	void removeTrack(int i, int j, int c = 1, const QString& text = "") const;

private:
	AlenkaFile::DataModel* dataModel;
	QUndoStack* undoStack;
};

#endif // UNDOCOMMANDFACTORY_H
