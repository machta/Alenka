#ifndef WX_H
#define	WX_H

#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <cassert>
#include <atomic>

template<class T>
using wxVector = std::vector<T>;

#define wxT(a_) wxString(a_)

const int wxEVT_THREAD = 0;

class wxString : public std::string
{
public:
	wxString() : std::string() {}
	wxString(const char* str) : std::string(str) {}

	template<class... T>
	int Printf(const wxString& format, T... args)
	{
		const char* formatStr = format.c_str();

		int size1 = snprintf(nullptr, 0, formatStr, args...);

		std::unique_ptr<char[]> buffer(new char[size1 + 1]);

		int size2 = sprintf(buffer.get(), formatStr, args...);
		assert(size1 == size2); (void)size2;

		assign(buffer.get(), buffer.get() + size1);

		return size1;
	}

	template<class... T>
	static wxString Format(const wxString& format, T... args)
	{
		wxString str;
		str.Printf(format, args...);
		return str;
	}
};

class wxThreadEvent
{
	int eventType, id, intVal;
	wxString stringVal;

public:
	wxThreadEvent(int eventType, int id) : eventType(eventType), id(id) {}

	void SetInt(int intCommand)
	{
		intVal = intCommand;
	}
	int GetInt() const
	{
		return intVal;
	}

	void SetString(const wxString& string)
	{
		stringVal = string;
	}
	wxString GetString() const
	{
		return stringVal;
	}

	wxThreadEvent* Clone() const
	{
		auto copy = new wxThreadEvent(eventType, id);
		copy->intVal = intVal;
		copy->stringVal = stringVal;
		return copy;
	}
};

struct wxEvtHandler
{
	std::atomic<int>* progress;
};

inline void wxQueueEvent(wxEvtHandler* dest, wxThreadEvent* event)
{
	int p = event->GetInt();

	if (p == -1)
		p = 100;

	dest->progress->store(p);
}

#endif // WX_H
