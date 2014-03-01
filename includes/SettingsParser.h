/*
 * Settings.h

 *
 *  Created on: Jan 11, 2014
 *      Author: denia
 */
#include <map>
#include <memory>
#include <string>
#include <libconfig.h++>

#ifndef SettingsParser_H
#define SettingsParser_H

namespace ydle {

class SettingsParser {
public:
private:
	std::unique_ptr<libconfig::Config> _pConfiguration;
	std::string config_file;
public:
	virtual ~SettingsParser();
	int	parseCommandLine(int argc, char **argv);
	int	parseConfigFile(std::string config_file);
	int	parseConfigFile();
	int	writeConfigFile();
	int	parseSettings();
	bool		configIsOk();
	void		showUsage();
	void		showConfig();
	int			Int(std::string s) ;
	std::string	Str(std::string s) ;
	static SettingsParser * Instance () ;
private:
	SettingsParser();
	static SettingsParser * _pInstance ;
};
#undef  PARAM_STR
#undef PARAM_INT
#define PARAM_STR(name) SettingsParser::Instance()->Str(name)
#define PARAM_INT(name) SettingsParser::Instance()->Int(name)

} /* namespace ydle */

#endif /* SettingsParser_H */
