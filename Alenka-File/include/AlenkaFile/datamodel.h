#ifndef ALENKAFILE_DATAMODEL_H
#define ALENKAFILE_DATAMODEL_H

#include "abstractdatamodel.h"

#include <memory>
#include <vector>

namespace AlenkaFile {

template <class T, class Base> class Table : public Base {
public:
  int rowCount() const override { return static_cast<int>(table.size()); }
  void insertRows(int row, int count) override {
    for (int i = 0; i < count; ++i)
      table.insert(table.begin() + row + i, defaultValue(row + i));
  }
  void removeRows(int row, int count) override {
    table.erase(table.begin() + row, table.begin() + row + count);
  }
  T row(int i) const override { return table[i]; }
  void row(int i, const T &value) override { table[i] = value; }

  // This must be here, otherwise there is a compile error.
  // Perhaps ask about it on Stack Overflow?
  T defaultValue(int row) const = 0;

protected:
  std::vector<T> table;
};

class EventTypeTable : public Table<EventType, AbstractEventTypeTable> {
public:
  EventType defaultValue(int row) const override;
};

class EventTable : public Table<Event, AbstractEventTable> {
public:
  Event defaultValue(int row) const override;
};

class TrackTable : public Table<Track, AbstractTrackTable> {
public:
  Track defaultValue(int row) const override;
};

class MontageTable : public Table<Montage, AbstractMontageTable> {
  std::vector<std::unique_ptr<AbstractEventTable>> eTable;
  std::vector<std::unique_ptr<AbstractTrackTable>> tTable;

public:
  void insertRows(int row, int count) override;
  void removeRows(int row, int count) override;
  Montage defaultValue(int row) const override;
  AbstractEventTable *eventTable(int i) override { return eTable[i].get(); }
  const AbstractEventTable *eventTable(int i) const override {
    return eTable[i].get();
  }
  AbstractTrackTable *trackTable(int i) override { return tTable[i].get(); }
  const AbstractTrackTable *trackTable(int i) const override {
    return tTable[i].get();
  }

protected:
  std::unique_ptr<AbstractEventTable> makeEventTable() const override {
    return std::make_unique<EventTable>();
  }
  std::unique_ptr<AbstractTrackTable> makeTrackTable() const override {
    return std::make_unique<TrackTable>();
  }
};

} // namespace AlenkaFile

#endif // ALENKAFILE_DATAMODEL_H
