#include <boost/program_options.hpp>
#include <string>

#ifndef OPTIONS_H
#define OPTIONS_H

class Options
{
public:
	Options(int ac, char** av);
	~Options()
	{
		delete description;
	}

	const boost::program_options::variable_value& operator[](const std::string& var) const
	{
		return vm[var];
	}
	bool isSet(const std::string& var) const
	{
		return vm.count(var) == 1;
	}

private:
	boost::program_options::variables_map vm;
	boost::program_options::options_description* description;
};

#endif // OPTIONS_H
