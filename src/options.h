/**
 * @brief The header with the Options class definition.
 *
 * MyApplication's constructor creates one Options object and stores its pointer
 * in PROGRAM_OPTIONS_POINTER and SET_PROGRAM_OPTIONS_POINTER. All code that
 * needs to access the options needs to include this header.
 *
 * @file
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <boost/program_options.hpp>

#include <QString>
#include <QVariant>

#include <memory>
#include <stdexcept>
#include <string>

#include <detailedexception.h>

class QSettings;

/**
 * @brief This class makes the options and settings available for use in code.
 *
 * boost::program_options library is used for the command line options and
 * config file parsing.
 *
 * This class also offers convenience methods for accessing the settings
 * database provided by QSettings. Qt handles how this data is stored.
 * This is handy for storing and retrieving state of various Qt controls.
 */
class Options {
  boost::program_options::variables_map vm;
  std::string desc, configPath;
  std::unique_ptr<QSettings> programSettings;

public:
  /**
   * @brief Constructor. Parameters from main() are redirected here.
   */
  Options(int argc, char **argv);
  ~Options();

  /**
   * @brief Redirects to get().
   */
  const boost::program_options::variable_value &
  operator[](const std::string &key) const {
    return get(key);
  }

  /**
   * @brief Retrieves the value designated by the key.
   * @param var The string key identifying the option.
   * @return This general type must be cast to the appropriate concrete type by
   * calling as().
   */
  const boost::program_options::variable_value &
  get(const std::string &key) const {
    if (isSet(key))
      return vm[key];
    else
      throwDetailed(std::runtime_error("Option '" + key + "' has no value."));
  }

  /**
   * @brief Returns true if the option designated by the key has a value.
   *
   * For command line only option this function tests whether they were
   * specified.
   */
  bool isSet(const std::string &key) const { return vm.count(key) == 1; }

  /**
   * @brief Returns options description text. This is used to print help.
   */
  const std::string &getDescription() const { return desc; }

  /**
   * @brief Returns the value previously stored under key.
   */
  QVariant settings(const QString &key) const;

  /**
   * @brief Store value under key.
   */
  void settings(const QString &key, const QVariant &value);

  /**
   * @brief Logs the content of the config file.
   */
  void logConfigFile() const;

private:
  /**
   * @brief Here are all tests of the options that have limited accepted values.
   */
  void validateValues();
  void parseConfigFile(
      const boost::program_options::options_description &configuration);
};

extern std::unique_ptr<Options> PROGRAM_OPTIONS;

inline bool isProgramOptionSet(const std::string &name) {
  return PROGRAM_OPTIONS->isSet(name);
}

/**
 * @brief Use this to read global program options.
 */
template <class T> T programOption(const std::string &name) {
  return (*PROGRAM_OPTIONS)[name].as<T>();
}

/**
 * @brief Use this to assign global program options to variables.
 *
 * This has the advantage over the other function that you don't have to specify
 * the type as a template parameter.
 */
template <class T> void programOption(const std::string &name, T &var) {
  var = (*PROGRAM_OPTIONS)[name].as<T>();
}

#endif // OPTIONS_H
