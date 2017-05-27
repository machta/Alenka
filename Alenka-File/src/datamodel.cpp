#include "../include/AlenkaFile/datamodel.h"

#include <cassert>

using namespace AlenkaFile;
using namespace std;

namespace
{

template<class T>
void eraseVector(vector<T>& v, int i, int count)
{
	v.erase(v.begin() + i, v.begin() + i + count);
}

} // namespace

namespace AlenkaFile
{

void EventTypeTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; ++i)
		table.insert(table.begin() + row + i, defaultValue(row + i));
}

void EventTypeTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);
}

EventType EventTypeTable::defaultValue(int row) const
{
	EventType et;

	et.id = row;
	et.name = "Type " + to_string(row);
	et.opacity = 0.25;
	et.color[0] = 255; et.color[1] = et.color[2] = 0;
	et.hidden = false;

	return et;
}

void EventTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; ++i)
		table.insert(table.begin() + row + i, defaultValue(row + i));
}

void EventTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);
}

Event EventTable::defaultValue(int row) const
{
	Event e;

	e.label = "Event " + to_string(row);
	e.type = -1;
	e.position = 0;
	e.duration = 1;
	e.channel = -2;

	return e;
}

void TrackTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; ++i)
		table.insert(table.begin() + row + i, defaultValue(row + i));
}

void TrackTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);
}

Track TrackTable::defaultValue(int row) const
{
	Track t;

	t.label = "T " + to_string(row);
	t.code = "out = in(" + to_string(row) + ");";
	t.color[0] = t.color[1] = t.color[2] = 0;
	t.amplitude = 1;
	t.hidden = false;

	return t;
}

MontageTable::~MontageTable()
{
	assert(eTable.size() == tTable.size());

	for (unsigned int i = 0; i < eTable.size(); i++)
	{
		delete eTable[i];
		delete tTable[i];
	}
}

void MontageTable::insertRows(int row, int count)
{
	for (int i = 0; i < count; ++i)
		table.insert(table.begin() + row + i, defaultValue(row + i));

	eTable.insert(eTable.begin() + row, count, nullptr);
	tTable.insert(tTable.begin() + row, count, nullptr);
	for (int i = 0; i < count; i++)
	{
		eTable[row + i] = makeEventTable();
		tTable[row + i] = makeTrackTable();
	}
}

void MontageTable::removeRows(int row, int count)
{
	eraseVector(table, row, count);

	for (int i = 0; i < count; i++)
	{
		delete eTable[row + i];
		delete tTable[row + i];
	}
	eraseVector(eTable, row, count);
	eraseVector(tTable, row, count);
}

Montage MontageTable::defaultValue(int row) const
{
	Montage m;

	m.name = "Montage " + to_string(row);
	m.save = false;

	return m;
}

} // namespace AlenkaFile
