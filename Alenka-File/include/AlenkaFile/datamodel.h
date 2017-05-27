#ifndef ALENKAFILE_DATAMODEL_H
#define ALENKAFILE_DATAMODEL_H

#include "abstractdatamodel.h"

#include <vector>

namespace AlenkaFile
{

class EventTypeTable : public AbstractEventTypeTable
{
	std::vector<EventType> table;

public:
	virtual ~EventTypeTable() override {}
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count) override;
	virtual void removeRows(int row, int count) override;
	virtual EventType row(int i) const override { return table[i]; }
	virtual void row(int i, const EventType& value) override { table[i] = value; }
	virtual EventType defaultValue(int row) const override;
};

class EventTable : public AbstractEventTable
{
	std::vector<Event> table;

public:
	virtual ~EventTable() override {}
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual Event row(int i) const override { return table[i]; }
	virtual void row(int i, const Event& value) override { table[i] = value; }
	virtual Event defaultValue(int row) const override;
};

class TrackTable : public AbstractTrackTable
{
	std::vector<Track> table;

public:
	virtual ~TrackTable() override {}
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual Track row(int i) const override { return table[i]; }
	virtual void row(int i, const Track& value) override { table[i] = value; }
	virtual Track defaultValue(int row) const override;
};

class MontageTable : public AbstractMontageTable
{
	std::vector<Montage> table;
	std::vector<AbstractEventTable*> eTable;
	std::vector<AbstractTrackTable*> tTable;

public:
	virtual ~MontageTable() override;
	virtual int rowCount() const override { return static_cast<int>(table.size()); }
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual Montage row(int i) const override { return table[i]; }
	virtual void row(int i, const Montage& value) override { table[i] = value; }
	virtual Montage defaultValue(int row) const override;
	virtual AbstractEventTable* eventTable(int i) override { return eTable[i]; }
	virtual const AbstractEventTable* eventTable(int i) const override { return eTable[i]; }
	virtual AbstractTrackTable* trackTable(int i) override { return tTable[i]; }
	virtual const AbstractTrackTable* trackTable(int i) const override { return tTable[i]; }

protected:
	virtual AbstractEventTable* makeEventTable() override { return new EventTable(); }
	virtual AbstractTrackTable* makeTrackTable() override { return new TrackTable(); }
};

} // namespace AlenkaFile

#endif // ALENKAFILE_DATAMODEL_H
