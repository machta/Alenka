/**
 * @brief The header with the Options class definition.
 *
 * MyApplication's constructor creates one Options object and stores its pointer
 * in PROGRAM_OPTIONS_POINTER. All code that needs to access the options needs
 * to include this header.
 *
 * Then PROGRAM_OPTIONS macro can be used to retrieve the values like this:
 * @code{.cpp}
int64_t memoryAvailable = PROGRAM_OPTIONS["dataFileCacheSize"].as<int64_t>();
 * @endcode
 *
 * @file
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <boost/program_options.hpp>

#include <QSettings>
#include <QString>
#include <QVariant>

#include <string>
#include <sstream>
#include <stdexcept>

/**
 * @brief This class makes the options and settings available for use in code.
 *
 * In MyApplication constructor one object of this type is created. This object
 * can be accessed via  PROGRAM_OPTIONS macro defined in this header.
 *
 * boost::program_options library is used for the command line options and
 * config file parsing.
 *
 * This class also offers convenience methods for accessing the settings database
 * provided by QSettings. Qt handles how this data is stored.
 * This is handy for storing and retrieving state of various Qt controls.
 */
class Options
{
public:
	/**
	 * @brief Constructor. Parameters from main are redirected here.
	 */
	Options(int argc, char** argv);

	/**
	 * @brief Redirects to get().
	 */
	const boost::program_options::variable_value& operator[](const std::string& key) const
	{
		return get(key);
	}

	/**
	 * @brief Retrieves the value designated by the key.
	 * @param var The string key identifying the option.
	 * @return This general type must be cast to the appropriate concrete type by calling as().
	 */
	const boost::program_options::variable_value& get(const std::string& key) const
	{
		using namespace std;

		if (isSet(key))
		{
			return vm[key];
		}
		else
		{
			throw runtime_error("Option '" + key + "' has no value.");
		}
	}

	/**
	 * @brief Returns true if the option designated by the key has a value.
	 *
	 * For command line only option this function tests whether they were specified.
	 */
	bool isSet(const std::string& key) const
	{
		return vm.count(key) == 1;
	}

	/**
	 * @brief Returns a reference to a description object.
	 */
	const boost::program_options::options_description& getDescription() const
	{
		return desc;
	}

	/**
	 * @brief Returns the value previously stored under key.
	 */
	QVariant settings(const QString& key)
	{
		return programSettings.value(key);
	}

	/**
	 * @brief Store value under key.
	 */
	void settings(const QString& key, const QVariant& value)
	{
		programSettings.setValue(key, value);
	}

	/**
	 * @brief Logs the content of the config file.
	 */
	void logConfigFile() const;

private:
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc;
	QSettings programSettings;

	/**
	 * @brief In this method should be defined all tests of the options that have limited accepted values.
	 */
	void validateValues();
};

/**
 * @brief This pointer is used from the rest of the code to access the global Options object.
 */
extern Options* PROGRAM_OPTIONS_POINTER;

#define PROGRAM_OPTIONS (*PROGRAM_OPTIONS_POINTER)

#endif // OPTIONS_H
