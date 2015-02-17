#include "options.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace boost::program_options;

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
	("platform", value<int>()->default_value(0), "OpenCL platform id")
	("device", value<int>()->default_value(0), "OpenCL device id")
	("window", value<string>()->default_value(""), "window function to be used on FIR coefficients (hamming | blackman)")
	("blockSize", value<unsigned int>()->default_value(1*1024), "size of one block of signal data")
	("memoryBuffersSize", value<unsigned int>()->default_value(100*1000*1000), "total memory available for signal buffers")
	("file,f", value<string>(), "data file")
	("vert", value<string>()->default_value("shader.vert"), "vertex shader source file")
	("frag", value<string>()->default_value("shader.frag"), "fragment shader source file")
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

	desc.add(all);
}

const Options* PROGRAM_OPTIONS;
