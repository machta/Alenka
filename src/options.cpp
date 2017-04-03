#include "options.h"

#include "error.h"

#include <fstream>
#include <cstdint>

#ifdef UNIX_BUILD
#include <sys/ioctl.h>
#endif

using namespace std;
using namespace boost::program_options;

namespace
{

int getTerminalWidth()
{
#ifdef UNIX_BUILD
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
#endif
	return 80;
}

} // namespace

Options::Options(int argc, char** argv) : programSettings("Martin Barta", "ZSBS")
{
	const int lineWidth = max(60, getTerminalWidth());
	const char* configDefault = "options.cfg";

	options_description commandLineOnly("Command line options", lineWidth);
	commandLineOnly.add_options()
	("help", "help message")
	("config,c", value<string>()->default_value(configDefault), "config file")
	("printBuffers", "print the values in buffers during signal processing")
	("clInfo", "print OpenCL platform and device info")
	("glInfo", "print OpenGL info")
	;

	options_description configuration("Configuration", lineWidth);
	configuration.add_options()
	("locale", value<string>()->default_value("en_us"), "the locale to be use; mostly controls decimal number format")
	("uncalibrated", value<bool>()->default_value(false), "assume uncalibrated data in gdf files")
	("clPlatform", value<int>()->default_value(0), "OpenCL platform id")
	("clDevice", value<int>()->default_value(0), "OpenCL device id to be used in SignalProcessor")
	("blockSize", value<unsigned int>()->default_value(32*1024), "size of one block of signal data")
	("gpuMemorySize", value<int64_t>()->default_value(0), "the maximum amount of GPU memory used; if <= 0 then the value is relative to the implementation defined maximum, otherwise it is absolute")
	("dataFileCacheSize", value<int64_t>()->default_value(0), "maximum total memory used by the data file cache; zero means don't use cache")
	("printBuffersFolder", value<string>()->default_value("."), "path to the folder to which the values will be saved (no end slash), only in debug build")
	("notchFrequency", value<double>()->default_value(50), "frequency used to filter power interference with the signal")
	("onlineFilter", value<bool>()->default_value(false), "should the signal be filtered every time before it is rendered")
	("glSharing", value<bool>()->default_value(false), "if true, OpenCL will share data directly with OpenGL")
	("eventRenderMode", value<int>()->default_value(2), "controls rendering of single-channel events; accepted values (from simplest mode) are 1, 2")
	("log", value<string>()->default_value("%Y-%m-%d--%H-%M-%S.log"), "string passed to strftime() to create the file name")
	("kernelCacheSize", value<int>()->default_value(0), "the maximum number of montage kernels that will be cached")
	("kernelCacheDir", value<string>()->default_value(""), "directory for storing cache files (empty means the same dir as the executable)")
	("autoSaveInterval", value<int>()->default_value(2*60), "in seconds")
	;

	options_description spikedet("Spikedet", lineWidth);
	spikedet.add_options()
	("fl", value<int>()->default_value(10), "lowpass filter frequency")
	("fh", value<int>()->default_value(60), "highpass filter frequency")
	("k1", value<double>()->default_value(3.65), "K1")
	("k2", value<double>()->default_value(3.65), "K2")
	("k3", value<double>()->default_value(0), "K3")
	("w", value<int>()->default_value(5), "winsize")
	("n", value<double>()->default_value(4), "noverlap")
	("buf", value<int>()->default_value(300), "buffering")
	("h", value<int>()->default_value(50), "main hum. freq.")
	("dt", value<double>()->default_value(0.005), "discharge tol.")
	("pt", value<double>()->default_value(0.12), "polyspike union time")
	("dec", value<int>()->default_value(200), "decimation")
	("sed", value<double>()->default_value(0.1), "spike event duration in seconds")
	;

	configuration.add(spikedet);

	// TODO: Write some info on the first line (like "Usage: ./Alenka", version, some description what this program does).
	options_description all("", lineWidth);
	all.add(commandLineOnly).add(configuration);

	// Parse the command-line input.
	store(parse_command_line(argc, argv, all), vm);
	notify(vm);

	// Parse the config file.
	ifstream ifs(vm["config"].as<string>());

	if (ifs.good())
	{
		store(parse_config_file(ifs, configuration), vm);
		notify(vm);
	}
	else
	{
		if (vm["config"].as<string>() == configDefault)
		{
			logToFile("Config file '" << vm["config"].as<string>() << "' not found.");
		}
		else
		{
			logToFileAndConsole("Config file '" << vm["config"].as<string>() << "' not found.");
		}
	}

	validateValues();

	// Store for later.
	desc.add(all);
}

void Options::validateValues()
{
	const int mode = get("eventRenderMode").as<int>();
	if (mode != 1 && mode != 2)
	{
		throw validation_error(validation_error::invalid_option_value, "eventRenderMode", to_string(mode));
	}
}

void Options::logConfigFile() const
{
	string fileName = get("config").as<string>();
	ifstream ifs(fileName);

	if (ifs.good())
	{
		string str;
		while (ifs.peek() != EOF)
			str.push_back(ifs.get());

		logToFile("Config file '" << fileName << "':\n" << str);
	}
}

const Options* PROGRAM_OPTIONS_POINTER = nullptr;
Options* SET_PROGRAM_OPTIONS_POINTER = nullptr;
