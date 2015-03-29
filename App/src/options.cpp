#include "options.h"

#include "error.h"

#include <iostream>
#include <fstream>
#include <cstdint>

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
	("clPlatform", value<int>()->default_value(0), "OpenCL platform id")
	("clDevice", value<int>()->default_value(0), "OpenCL device id to be used in SignalProcessor")
	("window", value<string>(), "window function to be used on FIR coefficients (hamming | blackman)")
	("blockSize", value<unsigned int>()->default_value(32*1024), "size of one block of signal data")
	("gpuMemorySize", value<int64_t>()->default_value(0), "the maximum amount of GPU memory used; if <= 0 then the value is relative to the implementation defined maximum, otherwise it is absolute")
	("dataFileCacheSize", value<int64_t>()->default_value(0), "maximum total memory used by the data file cache; zero means don't use cache")
	("file,f", value<string>(), "data file")
	("kernels", value<string>()->default_value("kernels.cl"), "OpenCL kernel source file")
	("printFilterFile", value<string>(), "print filter to a file with this name; if empty, stderr is used")
	("printBuffersFolder", value<string>()->default_value("."), "path to the folder to which the values will be saved (no end slash), only in debug build")
	("powerFrequency", value<double>()->default_value(50), "frequency used to filter power interference with the signal")
	("onlineFilter", value<bool>()->default_value(false), "should the signal be filtered everytime before it is rendered")
	("prepareFrames", value<unsigned int>()->default_value(2), "how many frames should be prepared in memory")
	("logFileName", value<string>()->default_value("%Y-%m-%d--%H-%M-%S.log"), "string passed to strftime() to create the file name")

	("lowpass", value<double>()->default_value(1000*1000*1000), "lowpass filter frequency")
	("highpass", value<double>()->default_value(-1), "highpass filter frequency")
	("notch", value<bool>()->default_value(false), "notch filter on or off")
	("montageFile", value<string>(), "definition of the montage, one row per line")
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
		logToBoth("Config file '" << vm["config"].as<string>() << "' not found.");
	}

	desc.add(all);
}

const Options* PROGRAM_OPTIONS_POINTER;
