#ifndef ALENKAFILE_ABSTRACTDATAMODEL_H
#define ALENKAFILE_ABSTRACTDATAMODEL_H

#include <array>
#include <cstdio>
#include <memory>
#include <string>

namespace AlenkaFile {

struct EventType {
  int id;
  std::string name;
  float opacity;
  std::array<int, 3> color;
  bool hidden;

  enum class Index { id, name, opacity, color, hidden, size };
};

class AbstractEventTypeTable {
public:
  virtual ~AbstractEventTypeTable() = default;
  virtual int rowCount() const = 0;
  virtual void insertRows(int row, int count = 1) = 0;
  virtual void removeRows(int row, int count = 1) = 0;
  virtual EventType row(int i) const = 0;
  virtual void row(int i, const EventType &value) = 0;
  virtual EventType defaultValue(int row) const = 0;
};

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

class AbstractEventTable {
public:
  virtual ~AbstractEventTable() = default;
  virtual int rowCount() const = 0;
  virtual void insertRows(int row, int count = 1) = 0;
  virtual void removeRows(int row, int count = 1) = 0;
  virtual Event row(int i) const = 0;
  virtual void row(int i, const Event &value) = 0;
  virtual Event defaultValue(int row) const = 0;
};

struct Track {
  std::string label, code;
  std::array<int, 3> color;
  double amplitude;
  bool hidden;
  float x, y, z;

  enum class Index { label, code, color, amplitude, hidden, x, y, z, size };
};

class AbstractTrackTable {
public:
  virtual ~AbstractTrackTable() = default;
  virtual int rowCount() const = 0;
  virtual void insertRows(int row, int count = 1) = 0;
  virtual void removeRows(int row, int count = 1) = 0;
  virtual Track row(int i) const = 0;
  virtual void row(int i, const Track &value) = 0;
  virtual Track defaultValue(int row) const = 0;
};

struct Montage {
  std::string name;
  bool save;

  enum class Index { name, save, size };
};

class AbstractMontageTable {
public:
  virtual ~AbstractMontageTable() = default;
  virtual int rowCount() const = 0;
  virtual void insertRows(int row, int count = 1) = 0;
  virtual void removeRows(int row, int count = 1) = 0;
  virtual Montage row(int i) const = 0;
  virtual void row(int i, const Montage &value) = 0;
  virtual Montage defaultValue(int row) const = 0;
  virtual AbstractEventTable *eventTable(int i) = 0;
  virtual const AbstractEventTable *eventTable(int i) const = 0;
  virtual AbstractTrackTable *trackTable(int i) = 0;
  virtual const AbstractTrackTable *trackTable(int i) const = 0;

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