#include <iostream>
#include "protobuf/bike.pb.h"
#include "iEvent.h"
#include "global_define.h"
#include "event_df.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include "log/log.h"
#include "iniparser/iniconfig.h"
#include "sql/sqlconnection.h"
#include "BusProcessor.h"

using namespace std;

const std::string ini_file_path = "/home/kazu/projects/config/shared_bike.ini";
const std::string log_config_file_path = "/home/kazu/projects/config/log.conf";

int main()
{
	// 1 set the log file
	if (!Log::get_instance()->init(log_config_file_path))
	{
		printf("init log module failed\n");
		return -2;
	}

	// 2 loading env parameters
	Iniconfig* config = Iniconfig::getInstance();
	if (!config->loadfile(ini_file_path))
	{
		LOG_ERROR("config.loadfile failed: %s\n");
		return -3;
	}
	st_env_config conf_args = config->getconfig();
	LOG_DEBUG("[database]  db_ip_: %s\n", conf_args.db_ip_.c_str());
	LOG_DEBUG("[database]  db_port_: %hu\n", conf_args.db_port_);
	LOG_DEBUG("[database]  db_user_: %s\n", conf_args.db_user_.c_str());
	LOG_DEBUG("[database]  db_pwd_: %s\n", conf_args.db_pwd_.c_str());
	LOG_DEBUG("[database]  db_name_: %s\n", conf_args.db_name_.c_str());
	LOG_DEBUG("[server]    svr_port_: %hu\n", conf_args.svr_port_);
	printf("loading env parameters success\n");

	// 3 sql initialization
	std::shared_ptr<MysqlConnection> sqlconn(new MysqlConnection);
	bool ret = sqlconn->Init(conf_args.db_ip_.c_str(), conf_args.db_port_, conf_args.db_user_.c_str(), conf_args.db_pwd_.c_str(), conf_args.db_name_.c_str());
	if (!ret)
	{
		LOG_ERROR("sqlconn->Init failed\n");
		return -1;
	}

	// 4 subscribe handler and create tables 
	BusinessProcessor processor(sqlconn);
	processor.init();
	
	// 5 open thread pool
	DispatchMsgService* dms = DispatchMsgService::getInstance();
	dms->open();
	 
	// 6 open the network interface
	NetworkInterface* nitf = new NetworkInterface();
	nitf->start(conf_args.svr_port_);

	while (1)
	{
		nitf->network_event_dispatch();
		sleep(1);
		printf("dispatch..\n");
	}

	nitf->close();
	dms->close();
	sleep(1);
	
	return 0;
}