#ifndef THREAD_H
#define	THREAD_H

#include <thread>
#include <cassert>
#include <atomic>

enum
{
	wxTHREAD_JOINABLE, wxTHREAD_DETACHED
};

class wxThread
{
public:
	typedef void* ExitCode;

	wxThread(int kind = wxTHREAD_DETACHED) : kind(kind) {}
	virtual ~wxThread() {}

	int Run()
	{
		t = std::thread([this] {
			entryReturn = Entry();
		});

		if (kind == wxTHREAD_DETACHED)
			Wait();

		return 0;
	}

	ExitCode Wait()
	{
		t.join();
		return entryReturn;
	}

	virtual bool TestDestroy() { return stop; }

	void Stop()
	{
		stop = true;
	}

protected:
	virtual ExitCode Entry() = 0;

private:
	std::thread t;
	int kind;
	ExitCode entryReturn;
	std::atomic<bool> stop{false};
};


#endif // THREAD_H
