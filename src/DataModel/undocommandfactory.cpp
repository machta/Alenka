#include "undocommandfactory.h"

#include <cassert>

using namespace std;
using namespace AlenkaFile;

namespace
{

class ChangeEventType : public QUndoCommand
{
	DataFile* file;
	int i;
	EventType before, after;

public:
	ChangeEventType(DataFile* file, int i, const EventType& value, const QString& text) :
		QUndoCommand(text), file(file), i(i), after(value)
	{
		before = file->getDataModel()->eventTypeTable()->row(i);
	}
	virtual void redo() override
	{
		file->getDataModel()->eventTypeTable()->row(i, after);
	}
	virtual void undo() override
	{
		file->getDataModel()->eventTypeTable()->row(i, before);
	}
};

class ChangeMontage : public QUndoCommand
{
	DataFile* file;
	int i;
	Montage before, after;

public:
	ChangeMontage(DataFile* file, int i, const Montage& value, const QString& text) :
		QUndoCommand(text), file(file), i(i), after(value)
	{
		before = file->getDataModel()->montageTable()->row(i);
	}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->row(i, after);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->row(i, before);
	}
};

class ChangeEvent : public QUndoCommand
{
	DataFile* file;
	int i, j;
	Event before, after;

public:
	ChangeEvent(DataFile* file, int i, int j, const Event& value, const QString& text) :
		QUndoCommand(text), file(file), i(i), j(j), after(value)
	{
		before = file->getDataModel()->montageTable()->eventTable(i)->row(j);
	}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->eventTable(i)->row(j, after);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->eventTable(i)->row(j, before);
	}
};

class ChangeTrack : public QUndoCommand
{
	DataFile* file;
	int i, j;
	Track before, after;

public:
	ChangeTrack(DataFile* file, int i, int j, const Track& value, const QString& text) :
		QUndoCommand(text), file(file), i(i), j(j), after(value)
	{
		before = file->getDataModel()->montageTable()->trackTable(i)->row(j);
	}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->trackTable(i)->row(j, after);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->trackTable(i)->row(j, before);
	}
};

class InsertEventType : public QUndoCommand
{
	DataFile* file;
	int i, c;

public:
	InsertEventType(DataFile* file, int i, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), c(c) {}
	virtual void redo() override
	{
		file->getDataModel()->eventTypeTable()->insertRows(i, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->eventTypeTable()->removeRows(i, c);
	}
};

class InsertMontage: public QUndoCommand
{
	DataFile* file;
	int i, c;

public:
	InsertMontage(DataFile* file, int i, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), c(c) {}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->insertRows(i, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->removeRows(i, c);
	}
};

class InsertEvent: public QUndoCommand
{
	DataFile* file;
	int i, j, c;

public:
	InsertEvent(DataFile* file, int i, int j, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), j(j), c(c) {}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->eventTable(i)->insertRows(j, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->eventTable(i)->removeRows(j, c);
	}
};

class InsertTrack: public QUndoCommand
{
	DataFile* file;
	int i, j, c;

public:
	InsertTrack(DataFile* file, int i, int j, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), j(j), c(c) {}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->trackTable(i)->insertRows(j, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->trackTable(i)->removeRows(j, c);
	}
};

class RemoveEventType : public QUndoCommand
{
	DataFile* file;
	int i, c;
	vector<EventType> before;

public:
	RemoveEventType(DataFile* file, int i, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), c(c)
	{
		for (int k = 0; k < c; ++k)
			before.push_back(file->getDataModel()->eventTypeTable()->row(i + k));
	}
	virtual void redo() override
	{
		file->getDataModel()->eventTypeTable()->removeRows(i, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->eventTypeTable()->insertRows(i, c);

		for (int k = 0; k < c; ++k)
			file->getDataModel()->eventTypeTable()->row(i + k, before[k]);
	}
};

class RemoveMontage: public QUndoCommand
{
	DataFile* file;
	int i, c;
	vector<Montage> before;
	vector<vector<Event>> eventsBefore;
	vector<vector<Track>> tracksBefore;

public:
	RemoveMontage(DataFile* file, int i, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), c(c)
	{
		eventsBefore.resize(c);
		tracksBefore.resize(c);

		AbstractMontageTable* mt = file->getDataModel()->montageTable();

		for (int k = 0; k < c; ++k)
			before.push_back(mt->row(i + k));

		for (int k = 0; k < c; ++k)
		{
			AbstractEventTable* et = mt->eventTable(i + k);
			int count = et->rowCount();
			for (int l = 0; l < count; ++l)
				eventsBefore[k].push_back(et->row(l));

			AbstractTrackTable* tt = mt->trackTable(i + k);
			count = tt->rowCount();
			for (int l = 0; l < count; ++l)
				tracksBefore[k].push_back(tt->row(l));
		}

	}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->removeRows(i, c);
	}
	virtual void undo() override
	{
		AbstractMontageTable* mt = file->getDataModel()->montageTable();
		mt->insertRows(i, c);

		for (int k = 0; k < c; ++k)
			mt->row(i + k, before[k]);

		for (int k = 0; k < c; ++k)
		{
			AbstractEventTable* et = mt->eventTable(i + k);
			int count = eventsBefore[k].size();
			et->insertRows(0, count);
			for (int l = 0; l < count; ++l)
				et->row(l, eventsBefore[k][l]);

			AbstractTrackTable* tt = mt->trackTable(i + k);
			count = tracksBefore[k].size();
			tt->insertRows(0, count);
			for (int l = 0; l < count; ++l)
				tt->row(l, tracksBefore[k][l]);
		}
	}
};

class RemoveEvent: public QUndoCommand
{
	DataFile* file;
	int i, j, c;
	vector<Event> before;

public:
	RemoveEvent(DataFile* file, int i, int j, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), j(j), c(c)
	{
		for (int k = 0; k < c; ++k)
			before.push_back(file->getDataModel()->montageTable()->eventTable(i)->row(j + k));
	}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->eventTable(i)->removeRows(j, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->eventTable(i)->insertRows(j, c);

		for (int k = 0; k < c; ++k)
			file->getDataModel()->montageTable()->eventTable(i)->row(j + k, before[k]);
	}
};

class RemoveTrack: public QUndoCommand
{
	DataFile* file;
	int i, j, c;
	vector<Track> before;

public:
	RemoveTrack(DataFile* file, int i, int j, int c, const QString& text) :
		QUndoCommand(text), file(file), i(i), j(j), c(c)
	{
		for (int k = 0; k < c; ++k)
			before.push_back(file->getDataModel()->montageTable()->trackTable(i)->row(j + k));
	}
	virtual void redo() override
	{
		file->getDataModel()->montageTable()->trackTable(i)->removeRows(j, c);
	}
	virtual void undo() override
	{
		file->getDataModel()->montageTable()->trackTable(i)->insertRows(j, c);

		for (int k = 0; k < c; ++k)
			file->getDataModel()->montageTable()->trackTable(i)->row(j + k, before[k]);
	}
};

} // namespace

QUndoCommand* UndoCommandFactory::changeEventType(int i, const EventType& value, const QString& text) const
{
	return new ChangeEventType(file->file, i, value, text);
}

QUndoCommand* UndoCommandFactory::changeMontage(int i, const Montage& value, const QString& text) const
{
	return new ChangeMontage(file->file, i, value, text);
}

QUndoCommand* UndoCommandFactory::changeEvent(int i, int j, const Event& value, const QString& text) const
{
	return new ChangeEvent(file->file, i, j, value, text);
}

QUndoCommand* UndoCommandFactory::changeTrack(int i, int j, const Track& value, const QString& text) const
{
	return new ChangeTrack(file->file, i, j, value, text);
}

QUndoCommand* UndoCommandFactory::insertEventType(int i, int c, const QString& text) const
{
	return new InsertEventType(file->file, i, c, text);
}

QUndoCommand* UndoCommandFactory::insertMontage(int i, int c, const QString& text) const
{
	return new InsertMontage(file->file, i, c, text);
}

QUndoCommand* UndoCommandFactory::insertEvent(int i, int j, int c, const QString& text) const
{
	return new InsertEvent(file->file, i, j, c, text);
}

QUndoCommand* UndoCommandFactory::insertTrack(int i, int j, int c, const QString& text) const
{
	return new InsertTrack(file->file, i, j, c, text);
}

QUndoCommand* UndoCommandFactory::removeEventType(int i, int c, const QString& text) const
{
	return new RemoveEventType(file->file, i, c, text);
}

QUndoCommand* UndoCommandFactory::removeMontage(int i, int c, const QString& text) const
{
	return new RemoveMontage(file->file, i, c, text);
}

QUndoCommand* UndoCommandFactory::removeEvent(int i, int j, int c, const QString& text) const
{
	return new RemoveEvent(file->file, i, j, c, text);
}

QUndoCommand* UndoCommandFactory::removeTrack(int i, int j, int c, const QString& text) const
{
	return new RemoveTrack(file->file, i, j, c, text);
}
