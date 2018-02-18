#ifndef VITNESSDATAMODEL_H
#define VITNESSDATAMODEL_H

#include "../../Alenka-File/include/AlenkaFile/datamodel.h"

#include <QObject>

class DataModelVitness : public QObject {
  Q_OBJECT

public:
  explicit DataModelVitness(QObject *parent = nullptr) : QObject(parent) {}

signals:
  void valueChanged(int row, int col);
  void rowsInserted(int row, int col);
  void rowsRemoved(int row, int col);
};

template <class Base, class BaseBase> class VitnessTable : public Base {
public:
  void insertRows(int row, int count) override {
    if (count > 0) {
      Base::insertRows(row, count);
      emit vitnessObject->rowsInserted(row, count);
    }
  }
  void removeRows(int row, int count) override {
    if (count > 0) {
      Base::removeRows(row, count);
      emit vitnessObject->rowsRemoved(row, count);
    }
  }

  static const DataModelVitness *vitness(const BaseBase *table) {
    return dynamic_cast<const VitnessTable *>(table)->vitnessObject.get();
  }

protected:
  std::unique_ptr<DataModelVitness> vitnessObject =
      std::make_unique<DataModelVitness>();
};

class VitnessEventTypeTable
    : public VitnessTable<AlenkaFile::EventTypeTable,
                          AlenkaFile::AbstractEventTypeTable> {
public:
  void row(int i, const AlenkaFile::EventType &value) override;
};

class VitnessEventTable : public VitnessTable<AlenkaFile::EventTable,
                                              AlenkaFile::AbstractEventTable> {
public:
  void row(int i, const AlenkaFile::Event &value) override;
};

class VitnessTrackTable : public VitnessTable<AlenkaFile::TrackTable,
                                              AlenkaFile::AbstractTrackTable> {
public:
  void row(int i, const AlenkaFile::Track &value) override;
  AlenkaFile::Track defaultValue(int row) const override;
};

class VitnessMontageTable
    : public VitnessTable<AlenkaFile::MontageTable,
                          AlenkaFile::AbstractMontageTable> {
public:
  void row(int i, const AlenkaFile::Montage &value) override;

private:
  std::unique_ptr<AlenkaFile::AbstractEventTable>
  makeEventTable() const override {
    return std::make_unique<VitnessEventTable>();
  }
  std::unique_ptr<AlenkaFile::AbstractTrackTable>
  makeTrackTable() const override {
    return std::make_unique<VitnessTrackTable>();
  }
};

#endif // VITNESSDATAMODEL_H
