#include "options.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace boost::program_options;

Options::Options(int ac, char** av)
{
	options_description cmdConfig("Command line options");
	cmdConfig.add_options()
	("help", "help message")
	("config,c", value<string>()->default_value("options.cfg"), "config file")
	;

	options_description otherConfig("Configuration");
	otherConfig.add_options()
	("uncalibrated", value<bool>()->default_value(false), "assume uncalibrated data in gdf files")
	;

	description = new options_description("Alloved options");
	description->add(cmdConfig).add(otherConfig);

    store(parse_command_line(ac, av, cmdConfig), vm);
    notify(vm);

	ifstream ifs(vm["config"].as<string>());

	if (ifs.good())
	{
		store(parse_config_file(ifs, otherConfig), vm);
		notify(vm);
	}
	else
	{
		cerr << "Config file '" << vm["config"].as<string>() << "' not found." << endl;
	}

	if (isSet("help"))
	{
		cout << *description << "\n";
	}
}


