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

class VitnessEventTypeTable : public AlenkaFile::EventTypeTable {
  std::unique_ptr<DataModelVitness> vitnessObject =
      std::make_unique<DataModelVitness>();

public:
  VitnessEventTypeTable() : AlenkaFile::EventTypeTable() {}
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  void row(int i, const AlenkaFile::EventType &value) override;

  static const DataModelVitness *
  vitness(const AlenkaFile::AbstractEventTypeTable *table) {
    return dynamic_cast<const VitnessEventTypeTable *>(table)
        ->vitnessObject.get();
  }
};

class VitnessEventTable : public AlenkaFile::EventTable {
  std::unique_ptr<DataModelVitness> vitnessObject =
      std::make_unique<DataModelVitness>();

public:
  VitnessEventTable() : AlenkaFile::EventTable() {}
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  void row(int i, const AlenkaFile::Event &value) override;

  static const DataModelVitness *
  vitness(const AlenkaFile::AbstractEventTable *table) {
    return dynamic_cast<const VitnessEventTable *>(table)->vitnessObject.get();
  }
};

class VitnessTrackTable : public AlenkaFile::TrackTable {
  std::unique_ptr<DataModelVitness> vitnessObject =
      std::make_unique<DataModelVitness>();

public:
  VitnessTrackTable() : AlenkaFile::TrackTable() {}
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  void row(int i, const AlenkaFile::Track &value) override;
  AlenkaFile::Track defaultValue(int row) const override;

  static const DataModelVitness *
  vitness(const AlenkaFile::AbstractTrackTable *table) {
    return dynamic_cast<const VitnessTrackTable *>(table)->vitnessObject.get();
  }
};

class VitnessMontageTable : public AlenkaFile::MontageTable {
  std::unique_ptr<DataModelVitness> vitnessObject =
      std::make_unique<DataModelVitness>();

public:
  VitnessMontageTable() : AlenkaFile::MontageTable() {}
  void insertRows(int row, int count = 1) override;
  void removeRows(int row, int count = 1) override;
  void row(int i, const AlenkaFile::Montage &value) override;

  static const DataModelVitness *
  vitness(const AlenkaFile::AbstractMontageTable *table) {
    return dynamic_cast<const VitnessMontageTable *>(table)
        ->vitnessObject.get();
  }

protected:
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
