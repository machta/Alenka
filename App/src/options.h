#include <boost/program_options.hpp>
#include <string>
#include <sstream>
#include <stdexcept>

#ifndef OPTIONS_H
#define OPTIONS_H

class Options
{
public:
	Options(int ac, char** av);

	const boost::program_options::variable_value& operator[](const std::string& var) const
	{
		return vm[var];
	}
	const boost::program_options::variable_value& get(const std::string& var) const
	{
		if (isSet(var))
		{
			return (*this)[var];
		}
		else
		{
			std::stringstream ss;
			ss << "Option '" << var << "' has no value.";
			throw std::runtime_error(ss.str());
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

private:
	boost::program_options::variables_map vm;
	boost::program_options::options_description desc;
};

extern const Options* PROGRAM_OPTIONS;

#endif // OPTIONS_H
