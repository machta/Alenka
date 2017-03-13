#ifndef VITNESSDATAMODEL_H
#define VITNESSDATAMODEL_H

#include "AlenkaFile/datamodel.h"

#include <QObject>

class DataModelVitness : public QObject
{
	Q_OBJECT

public:
	DataModelVitness(QObject* parent = nullptr) : QObject(parent) {}

signals:
	void valueChanged(int col, int row);
};

class VitnessEventTypeTable : public AlenkaFile::EventTypeTable
{
public:
	virtual ~VitnessEventTypeTable() override {}
	virtual void row(int i, const AlenkaFile::EventType& value) override;

	DataModelVitness* getVitness() const
	{
		return vitness;
	}
	void setVitness(DataModelVitness* value)
	{
		vitness = value;
	}

private:
	DataModelVitness* vitness = nullptr;
};

class VitnessEventTable : public AlenkaFile::EventTable
{
public:
	virtual ~VitnessEventTable() override {}
	virtual void row(int i, const AlenkaFile::Event& value) override;

	DataModelVitness* getVitness() const
	{
		return vitness;
	}
	void setVitness(DataModelVitness* value)
	{
		vitness = value;
	}

private:
	DataModelVitness* vitness = nullptr;
};

class VitnessTrackTable : public AlenkaFile::TrackTable
{
public:
	virtual ~VitnessTrackTable() override {}
	virtual void row(int i, const AlenkaFile::Track& value) override;

	DataModelVitness* getVitness() const
	{
		return vitness;
	}
	void setVitness(DataModelVitness* value)
	{
		vitness = value;
	}

private:
	DataModelVitness* vitness = nullptr;
};

class VitnessMontageTable : public AlenkaFile::MontageTable
{
public:
	virtual ~VitnessMontageTable() override;
	virtual void row(int i, const AlenkaFile::Montage& value) override;
	virtual AlenkaFile::AbstractEventTable* makeEventTable() override { return new VitnessEventTable(); }
	virtual AlenkaFile::AbstractTrackTable* makeTrackTable() override { return new VitnessTrackTable(); }

	DataModelVitness* getVitness() const
	{
		return vitness;
	}
	void setVitness(DataModelVitness* value)
	{
		vitness = value;
	}

private:
	DataModelVitness* vitness = nullptr;
};

#endif // VITNESSDATAMODEL_H
