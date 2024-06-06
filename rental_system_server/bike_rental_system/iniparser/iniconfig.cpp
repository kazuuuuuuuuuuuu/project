#include "iniconfig.h"
// ~/shared_bike/third/include/iniparser/iniparser.h
#include <iniparser/iniparser.h>

Iniconfig* Iniconfig::config_singleton = nullptr;

Iniconfig* Iniconfig::getInstance()
{
    if (config_singleton == nullptr)
    {
        config_singleton = new Iniconfig();
    }
    return config_singleton;
}

Iniconfig::Iniconfig():
isloaded_(false)
{
}

Iniconfig::~Iniconfig()
{
}

bool Iniconfig::loadfile(const std::string &path)
{
    if (isloaded_ == true) return true;

	dictionary *ini; 
	ini = iniparser_load(path.c_str());
    if(ini==NULL) 
    {
	    fprintf(stderr, "cannot parse file: %s\n", path.c_str());
	    return false;
    }

    const char *ip = iniparser_getstring(ini, "database:ip", "127.0.0.1"); // default -> "127.0.0.1"
    int port = iniparser_getint(ini, "database:port", 3306);
    const char *user = iniparser_getstring(ini, "database:user", "root");
    const char *pwd = iniparser_getstring(ini, "database:pwd", "123456");
    const char *db = iniparser_getstring(ini, "database:db", "shared_bike");
    int server_port = iniparser_getint(ini, "server:port", 9090);

    config_ = st_env_config(std::string(ip), (unsigned short) port, std::string(user), std::string(pwd), std::string(db), (unsigned short) server_port);

    iniparser_freedict(ini);
    isloaded_ = true;
    return true;
}
