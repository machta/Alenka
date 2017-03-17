#include "vitnessdatamodel.h"

using namespace std;
using namespace AlenkaFile;

void VitnessEventTypeTable::insertRows(int row, int count)
{
	if (count > 0)
	{
		EventTypeTable::insertRows(row, count);
		emit vitnessObject->rowsInserted(row, count);
	}
}

void VitnessEventTypeTable::removeRows(int row, int count)
{
	if (count > 0)
	{
		EventTypeTable::removeRows(row, count);
		emit vitnessObject->rowsRemoved(row, count);
	}
}

void VitnessEventTypeTable::row(int i, const EventType& value)
{
	EventType oldValue = EventTypeTable::row(i);
	EventTypeTable::row(i, value);

	if (oldValue.id != value.id)
		emit vitnessObject->valueChanged(i, 0);
	if (oldValue.name != value.name)
		emit vitnessObject->valueChanged(i, 1);
	if (oldValue.opacity != value.opacity)
		emit vitnessObject->valueChanged(i, 2);
	if (oldValue.color[0] != value.color[0] && oldValue.color[1] != value.color[1] && oldValue.color[2] != value.color[2])
		emit vitnessObject->valueChanged(i, 3);
	if (oldValue.hidden != value.hidden)
		emit vitnessObject->valueChanged(i, 4);
}

void VitnessEventTable::insertRows(int row, int count)
{
	if (count > 0)
	{
		EventTable::insertRows(row, count);
		emit vitnessObject->rowsInserted(row, count);
	}
}

void VitnessEventTable::removeRows(int row, int count)
{
	if (count > 0)
	{
		EventTable::removeRows(row, count);
		emit vitnessObject->rowsRemoved(row, count);
	}
}

void VitnessEventTable::row(int i, const Event& value)
{
	Event oldValue = EventTable::row(i);
	EventTable::row(i, value);

	if (oldValue.label != value.label)
		emit vitnessObject->valueChanged(i, 0);
	if (oldValue.type != value.type)
		emit vitnessObject->valueChanged(i, 1);
	if (oldValue.position != value.position)
		emit vitnessObject->valueChanged(i, 2);
	if (oldValue.duration != value.duration)
		emit vitnessObject->valueChanged(i, 3);
	if (oldValue.channel != value.channel)
		emit vitnessObject->valueChanged(i, 4);
	if (oldValue.description != value.description)
		emit vitnessObject->valueChanged(i, 5);
}

void VitnessTrackTable::insertRows(int row, int count)
{
	if (count > 0)
	{
		TrackTable::insertRows(row, count);
		emit vitnessObject->rowsInserted(row, count);
	}
}

void VitnessTrackTable::removeRows(int row, int count)
{
	if (count > 0)
	{
		TrackTable::removeRows(row, count);
		emit vitnessObject->rowsRemoved(row, count);
	}
}

void VitnessTrackTable::row(int i, const Track& value)
{
	Track oldValue = TrackTable::row(i);
	TrackTable::row(i, value);

	if (oldValue.label != value.label)
		emit vitnessObject->valueChanged(i, 0);
	if (oldValue.code != value.code)
		emit vitnessObject->valueChanged(i, 1);
	if (oldValue.color[0] != value.color[0] && oldValue.color[1] != value.color[1] && oldValue.color[2] != value.color[2])
		emit vitnessObject->valueChanged(i, 2);
	if (oldValue.amplitude != value.amplitude)
		emit vitnessObject->valueChanged(i, 3);
	if (oldValue.hidden != value.hidden)
		emit vitnessObject->valueChanged(i, 4);
}

void VitnessMontageTable::insertRows(int row, int count)
{
	if (count > 0)
	{
		MontageTable::insertRows(row, count);
		emit vitnessObject->rowsInserted(row, count);
	}
}

void VitnessMontageTable::removeRows(int row, int count)
{
	if (count > 0)
	{
		MontageTable::removeRows(row, count);
		emit vitnessObject->rowsRemoved(row, count);
	}
}

void VitnessMontageTable::row(int i, const Montage& value)
{
	Montage oldValue = MontageTable::row(i);
	MontageTable::row(i, value);

	if (oldValue.name != value.name)
		emit vitnessObject->valueChanged(i, 0);
	if (oldValue.save != value.save)
		emit vitnessObject->valueChanged(i, 1);
}
