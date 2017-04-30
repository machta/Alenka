#include "options.h"

#include "error.h"

#include <fstream>

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
	("config,c", value<string>()->default_value(configDefault), "config file path")
	("clInfo", "print OpenCL platform and device info")
	("glInfo", "print OpenGL info")
	("printBuffers", "dump OpenCL buffers for debugging")
#ifdef TESTS
	("test", "run unit tests")
#endif
	;

	options_description configuration("Configuration", lineWidth);
	configuration.add_options()
	("locale", value<string>()->default_value("en_us"), "the locale to be use; mostly controls decimal number format")
	("log", value<string>()->default_value("%Y-%m-%d--%H-%M-%S.log"), "string passed to strftime() to create the file name")
	("uncalibratedGDF", value<bool>()->default_value(false), "treat GDF files as uncalibrated")
	("autosaveInterval", value<int>()->default_value(2*60), "in seconds")
	("kernelCacheSize", value<int>()->default_value(0), "how many montage kernels can be cached; if 0, the existing file is removed")
	("kernelCacheDir", value<string>()->default_value(""), "directory for storing cache files (empty means the same dir as the executable)")
	("gl20", value<bool>()->default_value(false), "use OpenGL 2.0 plus some extensions instead of the default 3.0")
	("gl43", value<bool>()->default_value(false), "use OpenGL 4.3 instead of the default 3.0")
	("cl11", value<bool>()->default_value(false), "use OpenCL 1.1 instead of the default 1.2")
	("glSharing", value<bool>()->default_value(true), "use cl_khr_gl_sharing extension; this causes problems with Mesa, so set it to 0")
	("clPlatform", value<int>()->default_value(0), "OpenCL platform ID")
	("clDevice", value<int>()->default_value(0), "OpenCL device ID")
	("blockSize", value<unsigned int>()->default_value(32*1024), "how many samples per channel are in one block")
	("gpuMemorySize", value<int>()->default_value(0), "allowed GPU memory in MB; 0 means no limit")
	("parallelProcessors", value<unsigned int>()->default_value(2), "parallel signal processor queue count")
	("fileCacheSize", value<int>()->default_value(0), "allowed RAM for caching signal files in MB")
	("notchFrequency", value<double>()->default_value(50), "frequency used to filter power interference")
	("matD", value<string>()->default_value("d"), "data var name for MAT files")
	("matFs", value<string>()->default_value("fs"), "sample rate var name for MAT files")
	("matMults", value<string>()->default_value("mults"), "channel multipliers var name for MAT files")
	;

	options_description spikedet("Spikedet settings", lineWidth);
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
	("osd", value<bool>()->default_value(true), "use original Spikedet implementation")
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
	const int autosaveInterval = get("autosaveInterval").as<int>();
	if (autosaveInterval <= 0)
		throw validation_error(validation_error::invalid_option_value, "autosaveInterval", to_string(autosaveInterval));

	if (get("gl43").as<bool>())
		throw runtime_error("Option 'gl43' is disabled at the moment.");
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
