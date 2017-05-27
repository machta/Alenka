#ifndef ALENKAFILE_ABSTRACTDATAMODEL_H
#define ALENKAFILE_ABSTRACTDATAMODEL_H

#include <string>
#include <cstdio>

namespace AlenkaFile
{

struct EventType
{
	int id;
	std::string name;
	double opacity;
	unsigned char color[3];
	bool hidden;

	enum class Index
	{
		id, name, opacity, color, hidden, size
	};
};

class AbstractEventTypeTable
{
public:
	virtual ~AbstractEventTypeTable() {}
	virtual int rowCount() const = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual EventType row(int i) const = 0;
	virtual void row(int i, const EventType& value) = 0;
	virtual EventType defaultValue(int row) const = 0;
};

struct Event
{
	std::string label;
	int type;
	int position;
	int duration;
	int channel;
	std::string description;

	enum class Index
	{
		label, type, position, duration, channel, description, size
	};
};

class AbstractEventTable
{
public:
	virtual ~AbstractEventTable() {}
	virtual int rowCount() const = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual Event row(int i) const = 0;
	virtual void row(int i, const Event& value) = 0;
	virtual Event defaultValue(int row) const = 0;
};

struct Track
{
	std::string label;
	std::string code;
	unsigned char color[3];
	double amplitude;
	bool hidden;

	enum class Index
	{
		label, code, color, amplitude, hidden, size
	};
};

class AbstractTrackTable
{
public:
	virtual ~AbstractTrackTable() {}
	virtual int rowCount() const = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual Track row(int i) const = 0;
	virtual void row(int i, const Track& value) = 0;
	virtual Track defaultValue(int row) const = 0;
};

struct Montage
{
	std::string name;
	bool save;

	enum class Index
	{
		name, save, size
	};
};

class AbstractMontageTable
{
public:
	virtual ~AbstractMontageTable() {}
	virtual int rowCount() const = 0;
	virtual void insertRows(int row, int count = 1) = 0;
	virtual void removeRows(int row, int count = 1) = 0;
	virtual Montage row(int i) const = 0;
	virtual void row(int i, const Montage& value) = 0;
	virtual Montage defaultValue(int row) const = 0;
	virtual AbstractEventTable* eventTable(int i) = 0;
	virtual const AbstractEventTable* eventTable(int i) const = 0;
	virtual AbstractTrackTable* trackTable(int i) = 0;
	virtual const AbstractTrackTable* trackTable(int i) const = 0;

protected:
	virtual AbstractEventTable* makeEventTable() = 0;
	virtual AbstractTrackTable* makeTrackTable() = 0;
};

class DataModel
{
	AbstractEventTypeTable* ett;
	AbstractMontageTable* mt;

public:
	DataModel(AbstractEventTypeTable* eventTypeTable, AbstractMontageTable* montageTable) :
		ett(eventTypeTable), mt(montageTable) {}
	~DataModel()
	{
		delete ett;
		delete mt;
	}

	AbstractEventTypeTable* eventTypeTable() { return ett; }
	const AbstractEventTypeTable* eventTypeTable() const { return ett; }
	AbstractMontageTable* montageTable() { return mt; }
	const AbstractMontageTable* montageTable() const { return mt; }

	static std::string color2str(const unsigned char color[3])
	{
		std::string str = "#";
		for (int i = 0; i < 3; i++)
		{
			char tmp[3];
			sprintf(tmp, "%02x", color[i]);
			str += tmp;
		}
		return str;
	}
	static void str2color(const char* str, unsigned char* color)
	{
		unsigned int r, g, b;
		sscanf(str, "#%02x%02x%02x", &r, &g, &b);
		color[0] = static_cast<unsigned char>(r);
		color[1] = static_cast<unsigned char>(g);
		color[2] = static_cast<unsigned char>(b);
	}
	template<class T>
	static T array2color(unsigned char* c)
	{
		T color;
		color.setRed(c[0]);
		color.setGreen(c[1]);
		color.setBlue(c[2]);
		return color;
	}
	template<class T>
	static void color2array(const T& color, unsigned char* c)
	{
		c[0] = color.red();
		c[1] = color.green();
		c[2] = color.blue();
	}
};

} // namespace AlenkaFile

#endif // ALENKAFILE_ABSTRACTDATAMODEL_H
