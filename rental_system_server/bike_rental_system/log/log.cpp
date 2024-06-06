#include "log.h"
#include <iostream>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RemoteSyslogAppender.hh>
#include <log4cpp/PropertyConfigurator.hh>

// define the singleton
Log Log::instance_;

// 2 using the config file given by the file path to initialize the singleton (set the data member -> category_)
bool Log::init(const std::string &log_conf_path)
{
	try
	{
		log4cpp::PropertyConfigurator::configure(log_conf_path);
	}
	catch(log4cpp::ConfigureFailure &f)
	{
		std::cerr << "load log config file" << log_conf_path << "failed: " << f.what() << std::endl;
		return false;
	}
	category_ = &log4cpp::Category::getRoot(); 
	return true;
}