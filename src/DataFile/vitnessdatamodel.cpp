#include "vitnessdatamodel.h"

namespace
{

void emitValueChanged(DataModelVitness* vitness, int row, int col)
{
	if (vitness)
		emit vitness->valueChanged(col, row);
}

} // namespace

void VitnessEventTypeTable::row(int i, const AlenkaFile::EventType& value)
{
	//AlenkaFile::EventType et = EventTypeTable::row(i);

	//if (et.id != value.id) // TODO: add conditions
	emitValueChanged(vitness, i, 0);
	emitValueChanged(vitness, i, 1);
	emitValueChanged(vitness, i, 2);
	emitValueChanged(vitness, i, 3);
	emitValueChanged(vitness, i, 4);

	EventTypeTable::row(i, value);
}

void VitnessEventTable::row(int i, const AlenkaFile::Event& value)
{
	emitValueChanged(vitness, i, 0);
	emitValueChanged(vitness, i, 1);
	emitValueChanged(vitness, i, 2);
	emitValueChanged(vitness, i, 3);
	emitValueChanged(vitness, i, 4);
	emitValueChanged(vitness, i, 5);

	EventTable::row(i, value);
}

void VitnessTrackTable::row(int i, const AlenkaFile::Track& value)
{
	emitValueChanged(vitness, i, 0);
	emitValueChanged(vitness, i, 1);
	emitValueChanged(vitness, i, 2);
	emitValueChanged(vitness, i, 3);
	emitValueChanged(vitness, i, 4);

	TrackTable::row(i, value);
}

void VitnessMontageTable::row(int i, const AlenkaFile::Montage& value)
{
	emitValueChanged(vitness, i, 0);
	emitValueChanged(vitness, i, 1);

	MontageTable::row(i, value);
}
