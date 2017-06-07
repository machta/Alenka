#ifndef ALENKAFILE_DATAMODEL_H
#define ALENKAFILE_DATAMODEL_H

#include "abstractdatamodel.h"

#include <vector>

namespace AlenkaFile {

class EventTypeTable : public AbstractEventTypeTable {
  std::vector<EventType> table;

public:
  ~EventTypeTable() override = default;
  int rowCount() const override { return static_cast<int>(table.size()); }
  void insertRows(int row, int count) override;
  void removeRows(int row, int count) override;
  EventType row(int i) const override { return table[i]; }
  void row(int i, const EventType &value) override { table[i] = value; }
  EventType defaultValue(int row) const override;
};

class EventTable : public AbstractEventTable {
  std::vector<Event> table;

public:
  ~EventTable() override = default;
  int rowCount() const override { return static_cast<int>(table.size()); }
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  Event row(int i) const override { return table[i]; }
  void row(int i, const Event &value) override { table[i] = value; }
  Event defaultValue(int row) const override;
};

class TrackTable : public AbstractTrackTable {
  std::vector<Track> table;

public:
  ~TrackTable() override = default;
  int rowCount() const override { return static_cast<int>(table.size()); }
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  Track row(int i) const override { return table[i]; }
  void row(int i, const Track &value) override { table[i] = value; }
  Track defaultValue(int row) const override;
};

class MontageTable : public AbstractMontageTable {
  std::vector<Montage> table;
  std::vector<AbstractEventTable *> eTable;
  std::vector<AbstractTrackTable *> tTable;

public:
  ~MontageTable() override;
  int rowCount() const override { return static_cast<int>(table.size()); }
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  Montage row(int i) const override { return table[i]; }
  void row(int i, const Montage &value) override { table[i] = value; }
  Montage defaultValue(int row) const override;
  AbstractEventTable *eventTable(int i) override { return eTable[i]; }
  const AbstractEventTable *eventTable(int i) const override {
    return eTable[i];
  }
  AbstractTrackTable *trackTable(int i) override { return tTable[i]; }
  const AbstractTrackTable *trackTable(int i) const override {
    return tTable[i];
  }

protected:
  AbstractEventTable *makeEventTable() override { return new EventTable(); }
  AbstractTrackTable *makeTrackTable() override { return new TrackTable(); }
};

} // namespace AlenkaFile

#endif // ALENKAFILE_DATAMODEL_H
