#include "options.h"

#include "error.h"
#include "myapplication.h"

#include <QSettings>

#include <fstream>
#include <sstream>

#include <detailedexception.h>

#ifdef UNIX_BUILD
#include <sys/ioctl.h>
#endif

using namespace std;
using namespace boost::program_options;

namespace {

int getTerminalWidth() {
#ifdef UNIX_BUILD
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
#endif
  return 80;
}

const int LINE_WIDTH = max(60, getTerminalWidth());

} // namespace

Options::Options(int argc, char **argv)
    : programSettings(new QSettings("Martin Barta", "Alenka")) {
  // clang-format off
  options_description commandLineOnly("Command line options", LINE_WIDTH);
  commandLineOnly.add_options()
  ("help", "help message")
  ("config", value<string>()->value_name("path"), "override default config file path")
  ("spikedet", value<string>()->value_name("OUTPUT_FILE"), "Spikedet only mode")
  ("clInfo", "print OpenCL platform and device info")
  ("glInfo", "print OpenGL info")
  ("version", "print version number")
  ("printTiming", "print the time it took to redraw Canvas")
#ifndef NDEBUG
  ("printBuffers", "dump OpenCL buffers for debugging")
#endif
  ;

  options_description configuration("Configuration", LINE_WIDTH);
  configuration.add_options()
  ("mode", value<string>()->default_value("desktop")->value_name("val"), "desktop|tablet|tablet-full")
  ("locale", value<string>()->default_value("en_us")->value_name("lang"), "mostly controls decimal number format")
  ("uncalibratedGDF", value<bool>()->default_value(false)->value_name("bool"), "assume uncalibrated data in GDF")
  ("autosave", value<int>()->default_value(2*60)->value_name("seconds"), "interval between saves; 0 to disable")
  ("kernelCacheSize", value<int>()->default_value(10000)->value_name("c"), "how many montage kernels are stored in memory")
  ("kernelCachePersist", value<bool>()->default_value(false)->value_name("bool"), "whether to store kernels persistently")
  ("kernelCacheDir", value<string>()->value_name("path"), "default is install dir")
  ("gl20", value<bool>()->default_value(false)->value_name("bool"), "use OpenGL 2.0 instead of 3.0")
  ("gl43", value<bool>()->default_value(false)->value_name("bool"), "use OpenGL 4.3 instead of 3.0; disabled")
  ("cl11", value<bool>()->default_value(false)->value_name("bool"), "use OpenCL 1.1 instead of 1.2")
  ("glSharing", value<bool>()->default_value(true)->value_name("bool"), "use cl_khr_gl_sharing extension")
  ("clPlatform", value<int>()->default_value(0)->value_name("ID"), "select OpenCL platform")
  ("clDevice", value<int>()->default_value(0)->value_name("ID"), "OpenCL device")
  ("blockSize", value<int>()->default_value(16*1024)->value_name("val"), "samples per channel per block")
  ("gpuMemorySize", value<int>()->default_value(0)->value_name("MB"), "allowed GPU memory; 0 means no limit")
  ("parProc", value<int>()->default_value(2)->value_name("val"), "parallel signal processor queue count")
  ("fileCacheSize", value<int>()->default_value(0)->value_name("MB"), "allowed RAM for caching signal files")
  ("notchFrequency", value<double>()->default_value(50)->value_name("f"), "power interference filter")
  ("resOptions", value<string>()->default_value("1 2 5 7.5 10 20 50 75 100 200 500 750", "1 2 ...")->value_name("list"), "resolution combo options")
  ("screenPath", value<string>()->value_name("path"), "screenshot output dir path")
  ("screenType", value<string>()->default_value("png")->value_name("type"), "screenshot file type; jpg, png, or bmp")
  ("matData", value<vector<string>>()->value_name("val..."), "data var names for MAT files; default is 'd'")
  ("matFs", value<string>()->default_value("fs")->value_name("val"), "sample rate var name")
  ("matMults", value<string>()->default_value("mults")->value_name("val"), "channel multipliers var name")
  ("matDate", value<string>()->default_value("tabs")->value_name("val"), "start date var name")
  ("matLabel", value<string>()->default_value("header.label")->value_name("val"), "labels var name")
  ("matEvtPos", value<string>()->default_value("out.pos")->value_name("val"), "event position var name in seconds")
  ("matEvtDur", value<string>()->default_value("out.dur")->value_name("val"), "event duration var name in seconds")
  ("matEvtChan", value<string>()->default_value("out.chan")->value_name("val"), "one-based event channel index var name")
  ;

  options_description spikedet("Spikedet settings", LINE_WIDTH);
  spikedet.add_options()
  ("fl", value<int>()->default_value(10)->value_name("f"), "lowpass filter frequency")
  ("fh", value<int>()->default_value(60)->value_name("f"), "highpass filter frequency")
  ("k1", value<double>()->default_value(3.65, "3.65")->value_name("val"), "K1")
  ("k2", value<double>()->default_value(3.65, "3.65")->value_name("val"), "K2")
  ("k3", value<double>()->default_value(0)->value_name("val"), "K3")
  ("w", value<int>()->default_value(5)->value_name("val"), "winsize")
  ("n", value<double>()->default_value(4)->value_name("val"), "noverlap")
  ("buf", value<int>()->default_value(300)->value_name("seconds"), "buffering")
  ("h", value<int>()->default_value(50)->value_name("f"), "main hum. freq.")
  ("dt", value<double>()->default_value(0.005, "0.005")->value_name("val"), "discharge tol.")
  ("pt", value<double>()->default_value(0.12, "0.12")->value_name("val"), "polyspike union time")
  ("dec", value<int>()->default_value(200)->value_name("f"), "decimation")
  ("sed", value<double>()->default_value(0.1, "0.1")->value_name("seconds"), "spike event duration")
  ("osd", value<bool>()->default_value(true)->value_name("bool"), "use orginal Spikedet implementation")
  ;
  // clang-format on
  // TODO: Find out why there isn't the -ft option like in Matlab version of
  // spikedet.

  configuration.add(spikedet);

  options_description all("", LINE_WIDTH);
  all.add(commandLineOnly).add(configuration);

  stringstream ss;
  ss << all;
  desc = ss.str();

  // Parse the command-line input including the input-file name.
  all.add_options()("filename", value<vector<string>>());

  positional_options_description pos;
  pos.add("filename", -1);

  store(command_line_parser(argc, argv).options(all).positional(pos).run(), vm);
  notify(vm);

  parseConfigFile(configuration);

  validateValues();
}

Options::~Options() {}

QVariant Options::settings(const QString &key) const {
  return programSettings->value(key);
}

void Options::settings(const QString &key, const QVariant &value) {
  programSettings->setValue(key, value);
}

void Options::validateValues() {
  if (get("gl43").as<bool>())
    throwDetailed(runtime_error("Option 'gl43' is disabled at the moment."));

  const int parallelProcessors = get("parProc").as<int>();
  if (parallelProcessors <= 0)
    throwDetailed(validation_error(validation_error::invalid_option_value,
                                   "parProc", to_string(parallelProcessors)));

  const int blockSize = get("blockSize").as<int>();
  if (blockSize <= 0)
    throwDetailed(validation_error(validation_error::invalid_option_value,
                                   "blockSize", to_string(blockSize)));

  const string screenType = get("screenType").as<string>();
  if (!(screenType == "png" || screenType == "jpg" || screenType == "bmp"))
    throwDetailed(validation_error(validation_error::invalid_option_value,
                                   "screenType", screenType));
}

void Options::logConfigFile() const {
  if (configPath.empty())
    return;

  ifstream ifs(configPath);

  if (ifs.good()) {
    string str, line;

    while (getline(ifs, line), ifs.good()) {
      if (!line.empty() && line[0] != '#')
        str += line + '\n';
    }

    logToFile("Config file '" << configPath << "':\n" << str);
  }
}

void Options::parseConfigFile(const options_description &configuration) {
  ifstream ifs;

  if (isSet("config")) {
    configPath = get("config").as<string>();
    ifs.open(configPath);

    if (!ifs.good()) {
      logToFileAndConsole("Config file '" << configPath << "' not found.");
    }
  } else {
    configPath = MyApplication::makeAppSubdir({"options.ini"})
                     .absolutePath()
                     .toStdString();
    ifs.open(configPath);

    if (!ifs.good()) {
      logToFile("Config file '" << configPath << "' not found.");
    }
  }

  if (ifs.good()) {
    store(parse_config_file(ifs, configuration), vm);
    notify(vm);
  } else {
    configPath.clear();
  }
}

unique_ptr<Options> PROGRAM_OPTIONS;
