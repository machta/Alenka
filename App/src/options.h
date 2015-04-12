#ifndef OPTIONS_H
#define OPTIONS_H

#include <boost/program_options.hpp>

#include <QSettings>
#include <QString>
#include <QVariant>

#include <string>
#include <sstream>
#include <stdexcept>

class Options
{
public:
	Options(int ac, char** av);

	const boost::program_options::variable_value& operator[](const std::string& var) const
	{
		return get(var);
	}
	const boost::program_options::variable_value& get(const std::string& var) const
	{
		using namespace std;

		if (isSet(var))
		{
			return vm[var];
		}
		else
		{
			stringstream ss;
			ss << "Option '" << var << "' has no value.";
			throw runtime_error(ss.str());
		}
	}
	bool isSet(const std::string& var) const
	{
		return vm.count(var) == 1;
	}
	const boost::program_options::options_description& getDescription() const
	{
		return desc;
	}
	QVariant settings(const QString& key)
	{
		return programSettings.value(key);
	}
	void settings(const QString& key, const QVariant& value)
	{
		programSettings.setValue(key, value);
	}

private:
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc;
	QSettings programSettings;

	void validateValues();
};

extern Options* PROGRAM_OPTIONS_POINTER;

#define PROGRAM_OPTIONS (*PROGRAM_OPTIONS_POINTER)

#endif // OPTIONS_H
