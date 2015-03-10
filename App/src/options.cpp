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
	("printFilter", "should the filter coefficients be printed (everytime they are computed)")
	("printBuffers", "print the values in buffers during signal processing")
	("clInfo", "print OpenCL platform and device info")
	("glInfo", "print OpenGL info")
	;

	options_description other("Configuration");
	other.add_options()
	("uncalibrated", value<bool>()->default_value(false), "assume uncalibrated data in gdf files")
	("platform", value<int>()->default_value(0), "OpenCL platform id")
	("device", value<int>()->default_value(0), "OpenCL device id to be used in SignalProcessor")
	("window", value<string>(), "window function to be used on FIR coefficients (hamming | blackman)")
	("blockSize", value<unsigned int>()->default_value(8*1024), "size of one block of signal data")
	("gpuMemorySize", value<unsigned int>()->default_value(100*1000*1000), "total size of the GPU memory used in bytes")
	("dataFileCacheSize", value<unsigned int>()->default_value(0), "maximum total memory used by the data file cache; zero means don't use cache")
	("file,f", value<string>(), "data file")
	("vert", value<string>()->default_value("shader.vert"), "vertex shader source file")
	("frag", value<string>()->default_value("shader.frag"), "fragment shader source file")
	("kernels", value<string>()->default_value("kernels.cl"), "OpenCL kernel source file")
	("printFilterFile", value<string>(), "print filter to a file with this name; if empty, stderr is used")
	("printBuffersFolder", value<string>()->default_value("."), "path to the folder to which the values will be saved (no end slash), only in debug build")
	("powerFrequency", value<double>()->default_value(50), "frequency used to filter power interference with the signal")
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

const Options* PROGRAM_OPTIONS_POINTER;
