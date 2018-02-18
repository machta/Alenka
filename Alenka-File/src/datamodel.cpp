#include "../include/AlenkaFile/datamodel.h"

using namespace std;

namespace AlenkaFile {

EventType EventTypeTable::defaultValue(int row) const {
  EventType et;

  et.id = row;
  et.name = "Type " + to_string(row);
  et.opacity = 0.25;
  et.color[0] = 255;
  et.color[1] = et.color[2] = 0;
  et.hidden = false;

  return et;
}

Event EventTable::defaultValue(int row) const {
  Event e;

  e.label = "Event " + to_string(row);
  e.type = -1;
  e.position = 0;
  e.duration = 1;
  e.channel = -2;
  e.description = "";

  return e;
}

Track TrackTable::defaultValue(int row) const {
  Track t;

  t.label = "T " + to_string(row);
  t.code = "out = in(INDEX);";
  t.color[0] = t.color[1] = t.color[2] = 0;
  t.amplitude = 1;
  t.hidden = false;
  t.x = t.y = t.z = 0;

  return t;
}

void MontageTable::insertRows(int row, int count) {
  Table<Montage, AbstractMontageTable>::insertRows(row, count);

  for (int i = 0; i < count; ++i) {
    eTable.insert(eTable.begin() + row + i, makeEventTable());
    tTable.insert(tTable.begin() + row + i, makeTrackTable());
  }
}

void MontageTable::removeRows(int row, int count) {
  Table<Montage, AbstractMontageTable>::removeRows(row, count);

  eTable.erase(eTable.begin() + row, eTable.begin() + row + count);
  tTable.erase(tTable.begin() + row, tTable.begin() + row + count);
}

Montage MontageTable::defaultValue(int row) const {
  Montage m;

  m.name = "Montage " + to_string(row);
  m.save = false;

  return m;
}

} // namespace AlenkaFile
