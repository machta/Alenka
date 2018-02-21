#ifndef ALENKAFILE_ABSTRACTDATAMODEL_H
#define ALENKAFILE_ABSTRACTDATAMODEL_H

#include <array>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

namespace AlenkaFile {

template <class T> class AbstractTable {
public:
  virtual ~AbstractTable() = default;
  virtual int rowCount() const = 0;
  virtual void insertRows(int row, int count = 1) = 0;
  virtual void removeRows(int row, int count = 1) = 0;
  virtual T row(int i) const = 0;
  virtual void row(int i, const T &value) = 0;
  virtual T defaultValue(int row) const = 0;

  void copy(const AbstractTable *src) {
    const int oldCount = rowCount();
    const int newCount = src->rowCount();
    const int diff = abs(oldCount - newCount);

    if (oldCount < newCount)
      insertRows(oldCount, diff);
    else if (oldCount > newCount)
      removeRows(newCount, diff);
    assert(newCount == rowCount());

    for (int i = 0; i < newCount; ++i)
      row(i, src->row(i));
  }
};

struct EventType {
  int id;
  std::string name;
  float opacity;
  std::array<int, 3> color;
  bool hidden;

  enum class Index { id, name, opacity, color, hidden, size };
};

class AbstractEventTypeTable : public AbstractTable<EventType> {};

struct Event {
  std::string label;
  int type, position, duration, channel;
  std::string description;

  enum class Index {
    label,
    type,
    position,
    duration,
    channel,
    description,
    size
  };
};

class AbstractEventTable : public AbstractTable<Event> {};

struct Track {
  std::string label, code;
  std::array<int, 3> color;
  double amplitude;
  bool hidden;
  float x, y, z;

  enum class Index { label, code, color, amplitude, hidden, x, y, z, size };
};

class AbstractTrackTable : public AbstractTable<Track> {};

struct Montage {
  std::string name;
  bool save;

  enum class Index { name, save, size };
};

class AbstractMontageTable : public AbstractTable<Montage> {
public:
  virtual AbstractEventTable *eventTable(int i) = 0;
  virtual const AbstractEventTable *eventTable(int i) const = 0;
  virtual AbstractTrackTable *trackTable(int i) = 0;
  virtual const AbstractTrackTable *trackTable(int i) const = 0;

  void copy(const AbstractMontageTable *src) {
    AbstractTable<Montage>::copy(src);

    const int count = rowCount();
    for (int i = 0; i < count; ++i) {
      eventTable(i)->copy(src->eventTable(i));
      trackTable(i)->copy(src->trackTable(i));
    }
  }

protected:
  virtual std::unique_ptr<AbstractEventTable> makeEventTable() const = 0;
  virtual std::unique_ptr<AbstractTrackTable> makeTrackTable() const = 0;
};

class DataModel {
  std::unique_ptr<AbstractEventTypeTable> ett;
  std::unique_ptr<AbstractMontageTable> mt;

public:
  DataModel(std::unique_ptr<AbstractEventTypeTable> eventTypeTable,
            std::unique_ptr<AbstractMontageTable> montageTable)
      : ett(std::move(eventTypeTable)), mt(std::move(montageTable)) {}

  AbstractEventTypeTable *eventTypeTable() { return ett.get(); }
  const AbstractEventTypeTable *eventTypeTable() const { return ett.get(); }
  AbstractMontageTable *montageTable() { return mt.get(); }
  const AbstractMontageTable *montageTable() const { return mt.get(); }

  void copy(const DataModel &src) {
    ett->copy(src.ett.get());
    mt->copy(src.mt.get());
  }

  // Some helper functions for converting colors.
  static std::string colorArray2str(std::array<int, 3> color) {
    std::string str = "#";
    for (int i = 0; i < 3; ++i) {
      char tmp[3];
      sprintf(tmp, "%02x", color[i]);
      str += tmp;
    }
    return str;
  }
  static auto str2colorArray(const char *str) {
    unsigned int r, g, b;
    sscanf(str, "#%02x%02x%02x", &r, &g, &b);
    return std::array<int, 3>{
        {static_cast<int>(r), static_cast<int>(g), static_cast<int>(b)}};
  }
  template <class T> static T array2color(std::array<int, 3> c) {
    T color;
    color.setRed(c[0]);
    color.setGreen(c[1]);
    color.setBlue(c[2]);
    return color;
  }
  template <class T> static auto color2colorArray(const T &color) {
    return std::array<int, 3>{{color.red(), color.green(), color.blue()}};
  }
};

} // namespace AlenkaFile

#endif // ALENKAFILE_ABSTRACTDATAMODEL_H
