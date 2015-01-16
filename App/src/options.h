#include <boost/program_options.hpp>
#include <string>

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
        return (*this)[var];
    }
	bool isSet(const std::string& var) const
	{
		return vm.count(var) == 1;
	}

private:
    boost::program_options::variables_map vm;
};

extern const Options* PROGRAM_OPTIONS;

#endif // OPTIONS_H
