#ifndef _CONFIGDEF_H_
#define _CONFIGDEF_H_ 

#include <string>

struct st_env_config
{
	// database configuration
	std::string db_ip_;
	unsigned short db_port_;
	std::string db_user_;
	std::string db_pwd_;
	std::string db_name_;

	// server configuration
	unsigned short svr_port_;

	st_env_config()
	{
	}

	st_env_config(const std::string &db_ip, const unsigned short db_port, const std::string &db_user, const std::string &db_pwd, const std::string &db_name, const unsigned short svr_port)
	{
		db_ip_ = db_ip;
		db_port_ = db_port;
		db_user_ = db_user;
		db_pwd_ = db_pwd;
		db_name_ = db_name;
		svr_port_ = svr_port;
	}

	st_env_config &operator=(const st_env_config &config)
	{
		if(this==&config)
			return *this;
		db_ip_ = config.db_ip_;
		db_port_ = config.db_port_;
		db_user_ = config.db_user_;
		db_pwd_ = config.db_pwd_;
		db_name_ = config.db_name_;
		svr_port_ = config.svr_port_;
		return *this;
	}
};
#endif