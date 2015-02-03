#include "options.h"
#include "testwindow.h"

#include <QApplication>

#include <iostream>

using namespace std;

int main(int ac, char** av)
{
    Options* options = new Options(ac, av);
    PROGRAM_OPTIONS = options;

	if (options->isSet("help"))
	{
		cout << options->getDescription() << endl;

		delete options;
		return 0;
	}

	QApplication app(ac, av);

	TestWindow window;
	window.show();

	int res = app.exec();

	delete options;
	return res;
}
