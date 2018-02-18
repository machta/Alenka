#include "vitnessdatamodel.h"

using namespace std;
using namespace AlenkaFile;

namespace {

/**
 * @brief Convert an enum-class item to the underlying value type.
 *
 * ec() is for enum cast.
 */
template <typename E> constexpr auto ec(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

} // namespace

void VitnessEventTypeTable::row(int i, const EventType &value) {
  EventType oldValue = EventTypeTable::row(i);
  EventTypeTable::row(i, value);

  if (oldValue.id != value.id)
    emit vitnessObject->valueChanged(i, ec(EventType::Index::id));

  if (oldValue.name != value.name)
    emit vitnessObject->valueChanged(i, ec(EventType::Index::name));

  if (oldValue.opacity != value.opacity)
    emit vitnessObject->valueChanged(i, ec(EventType::Index::opacity));

  if (oldValue.color != value.color)
    emit vitnessObject->valueChanged(i, ec(EventType::Index::color));

  if (oldValue.hidden != value.hidden)
    emit vitnessObject->valueChanged(i, ec(EventType::Index::hidden));
}

void VitnessEventTable::row(int i, const Event &value) {
  Event oldValue = EventTable::row(i);
  EventTable::row(i, value);

  if (oldValue.label != value.label)
    emit vitnessObject->valueChanged(i, ec(Event::Index::label));

  if (oldValue.type != value.type)
    emit vitnessObject->valueChanged(i, ec(Event::Index::type));

  if (oldValue.position != value.position)
    emit vitnessObject->valueChanged(i, ec(Event::Index::position));

  if (oldValue.duration != value.duration)
    emit vitnessObject->valueChanged(i, ec(Event::Index::duration));

  if (oldValue.channel != value.channel)
    emit vitnessObject->valueChanged(i, ec(Event::Index::channel));

  if (oldValue.description != value.description)
    emit vitnessObject->valueChanged(i, ec(Event::Index::description));
}

void VitnessTrackTable::row(int i, const Track &value) {
  Track oldValue = TrackTable::row(i);
  TrackTable::row(i, value);

  if (oldValue.label != value.label)
    emit vitnessObject->valueChanged(i, ec(Track::Index::label));

  if (oldValue.code != value.code)
    emit vitnessObject->valueChanged(i, ec(Track::Index::code));

  if (oldValue.color != value.color)
    emit vitnessObject->valueChanged(i, ec(Track::Index::color));

  if (oldValue.amplitude != value.amplitude)
    emit vitnessObject->valueChanged(i, ec(Track::Index::amplitude));

  if (oldValue.hidden != value.hidden)
    emit vitnessObject->valueChanged(i, ec(Track::Index::hidden));

  if (oldValue.x != value.x)
    emit vitnessObject->valueChanged(i, ec(Track::Index::x));

  if (oldValue.y != value.y)
    emit vitnessObject->valueChanged(i, ec(Track::Index::y));

  if (oldValue.z != value.z)
    emit vitnessObject->valueChanged(i, ec(Track::Index::z));
}

AlenkaFile::Track VitnessTrackTable::defaultValue(int row) const {
  auto t = TrackTable::defaultValue(row);
  t.amplitude = 0.000001;
  return t;
}

void VitnessMontageTable::row(int i, const Montage &value) {
  Montage oldValue = MontageTable::row(i);
  MontageTable::row(i, value);

  if (oldValue.name != value.name)
    emit vitnessObject->valueChanged(i, ec(Montage::Index::name));

  if (oldValue.save != value.save)
    emit vitnessObject->valueChanged(i, ec(Montage::Index::save));
}
