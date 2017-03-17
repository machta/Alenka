#ifndef VITNESSDATAMODEL_H
#define VITNESSDATAMODEL_H

#include "AlenkaFile/datamodel.h"

#include <QObject>

class DataModelVitness : public QObject
{
	Q_OBJECT

public:
	explicit DataModelVitness(QObject* parent = nullptr) : QObject(parent) {}

signals:
	void valueChanged(int row, int col);
	void rowsInserted(int row, int col);
	void rowsRemoved(int row, int col);
};

class VitnessEventTypeTable : public AlenkaFile::EventTypeTable
{
public:
	VitnessEventTypeTable() : AlenkaFile::EventTypeTable()
	{
		vitnessObject = new DataModelVitness();
	}
	virtual ~VitnessEventTypeTable() override
	{
		delete vitnessObject;
	}
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual void row(int i, const AlenkaFile::EventType& value) override;

	static DataModelVitness* vitness(AlenkaFile::AbstractEventTypeTable* table)
	{
		return dynamic_cast<VitnessEventTypeTable*>(table)->vitnessObject;
	}

private:
	DataModelVitness* vitnessObject = nullptr;
};

class VitnessEventTable : public AlenkaFile::EventTable
{
public:
	VitnessEventTable() : AlenkaFile::EventTable()
	{
		vitnessObject = new DataModelVitness();
	}
	virtual ~VitnessEventTable() override
	{
		delete vitnessObject;
	}
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual void row(int i, const AlenkaFile::Event& value) override;

	static DataModelVitness* vitness(AlenkaFile::AbstractEventTable* table)
	{
		return dynamic_cast<VitnessEventTable*>(table)->vitnessObject;
	}

private:
	DataModelVitness* vitnessObject = nullptr;
};

class VitnessTrackTable : public AlenkaFile::TrackTable
{
public:
	VitnessTrackTable() : AlenkaFile::TrackTable()
	{
		vitnessObject = new DataModelVitness();
	}
	virtual ~VitnessTrackTable() override
	{
		delete vitnessObject;
	}
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual void row(int i, const AlenkaFile::Track& value) override;

	static DataModelVitness* vitness(AlenkaFile::AbstractTrackTable* table)
	{
		return dynamic_cast<VitnessTrackTable*>(table)->vitnessObject;
	}

private:
	DataModelVitness* vitnessObject = nullptr;
};

class VitnessMontageTable : public AlenkaFile::MontageTable
{
public:
	VitnessMontageTable() : AlenkaFile::MontageTable()
	{
		vitnessObject = new DataModelVitness();
	}
	virtual ~VitnessMontageTable() override
	{
		delete vitnessObject;
	}
	virtual void insertRows(int row, int count = 1) override;
	virtual void removeRows(int row, int count = 1) override;
	virtual void row(int i, const AlenkaFile::Montage& value) override;

	static DataModelVitness* vitness(AlenkaFile::AbstractMontageTable* table)
	{
		return dynamic_cast<VitnessMontageTable*>(table)->vitnessObject;
	}

protected:
	virtual AlenkaFile::AbstractEventTable* makeEventTable() override { return new VitnessEventTable(); }
	virtual AlenkaFile::AbstractTrackTable* makeTrackTable() override { return new VitnessTrackTable(); }

private:
	DataModelVitness* vitnessObject = nullptr;
};

#endif // VITNESSDATAMODEL_H
