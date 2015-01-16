#include "options.h"

#include <iostream>

using namespace std;

int main(int ac, char** av)
{
    Options* options = new Options(ac, av);
    PROGRAM_OPTIONS = options;

    delete options;
	return 0;
}
