#ifndef _INICONFIG_H_
#define _INICONFIG_H_

#include <string>
#include "configdef.h"

// load parameters of the inifile into the config structure 
class Iniconfig
{
protected:
	Iniconfig(); // singleton

public:
	~Iniconfig();
	static Iniconfig* getInstance();
	bool loadfile(const std::string &path);
	const st_env_config &getconfig() { return config_; }
	
private:
	static Iniconfig* config_singleton;
	st_env_config config_;
	bool isloaded_;
};
#endif