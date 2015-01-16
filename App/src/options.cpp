#include "options.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace boost::program_options;

const Options* PROGRAM_OPTIONS;

Options::Options(int ac, char** av)
{
    options_description commandLineOnly("Command line options");
    commandLineOnly.add_options()
	("help", "help message")
	("config,c", value<string>()->default_value("options.cfg"), "config file")
	;

    options_description other("Configuration");
    other.add_options()
	("uncalibrated", value<bool>()->default_value(false), "assume uncalibrated data in gdf files")
	;

    options_description all("Alloved options");
    all.add(commandLineOnly).add(other);

    store(parse_command_line(ac, av, all), vm);
    notify(vm);

	ifstream ifs(vm["config"].as<string>());

	if (ifs.good())
	{
        store(parse_config_file(ifs, other), vm);
		notify(vm);
	}
	else
	{
		cerr << "Config file '" << vm["config"].as<string>() << "' not found." << endl;
	}

	if (isSet("help"))
	{
        cout << all << "\n";
	}
}


