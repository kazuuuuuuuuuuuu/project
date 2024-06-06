#ifndef _LOG_H_
#define _LOG_H_ 

#include <string>
#include <log4cpp/Category.hh>

class Log
{
public:
	// 1 return the pointer of the singleton
	static Log *get_instance() 
	{
		return &instance_;
	}	
	// 2 using the config file given by the file path to initialize the singleton
	bool init(const std::string &log_conf_path);
	// 3 using the singleton to log
	log4cpp::Category *get_handle() 
	{
		return category_;
	}

protected:
	// singleton
	static Log instance_;
	log4cpp::Category *category_;
};

// 3 using macro to simplify the code
// Log::get_instance() -> the usage of static function -> obtaining the singleton
#define LOG_INFO Log::get_instance()->get_handle()->info
#define LOG_DEBUG Log::get_instance()->get_handle()->debug
#define LOG_ERROR Log::get_instance()->get_handle()->error
#define LOG_WARN Log::get_instance()->get_handle()->warn

#endif