#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "global_define.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "sql/sqlconnection.h"
#include "iniparser/iniconfig.h"

UserEventHandler::UserEventHandler() :
	iEventHandler(std::string("UserEventHandler"))
{
	srand((unsigned int)time(NULL));
	thread_mutex_create(&pm_);
	// subscribe
	DispatchMsgService::getInstance()->subscribe(EVENT_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EVENT_LOGIN_REQ, this);
	/*
	* DispatchMsgService::getInstance()->subscribe(EVENT_RECHARGE_REQ, this);
	* DispatchMsgService::getInstance()->subscribe(EVENT_GET_ACCOUNT_BALANCE_REQ, this);
	* DispatchMsgService::getInstance()->subscribe(EVENT_LIST_ACCOUNT_RECORDS_REQ, this);
	*/
}

UserEventHandler::~UserEventHandler()
{
	thread_mutex_destroy(&pm_);
	// unsubscribe
	DispatchMsgService::getInstance()->unsubscribe(EVENT_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EVENT_LOGIN_REQ, this);
	/*
	* DispatchMsgService::getInstance()->unsubscribe(EVENT_RECHARGE_REQ, this);
	* DispatchMsgService::getInstance()->unsubscribe(EVENT_GET_ACCOUNT_BALANCE_REQ, this);
	* DispatchMsgService::getInstance()->unsubscribe(EVENT_LIST_ACCOUNT_RECORDS_REQ, this);
	*/
}

// distribute the task to the method corresponding to the event 
iEvent* UserEventHandler::handle(const iEvent* ev)
{
	if (ev == nullptr)
	{
		LOG_ERROR("UserEventHandler::handle - ev == nullptr\n");
		return nullptr;
	}

	// call the corresponding handler function for the eid)
	u32 eid = ev->get_eid();
	if (eid == EVENT_GET_MOBILE_CODE_REQ)
	{
		return handle_mobile_code_req((MobileCodeReqEv*)ev);
	}
	else if (eid == EVENT_LOGIN_REQ)
	{
		return handle_login_req((LoginReqEv*)ev);
	}
	else if (eid == EVENT_RECHARGE_REQ)
	{
		return nullptr;
	}
	else if (eid == EVENT_GET_ACCOUNT_BALANCE_REQ)
	{
		return nullptr;
	}
	else if (eid == EVENT_LIST_ACCOUNT_RECORDS_REQ)
	{
		return nullptr;
	}
	else if (eid == EVENT_LIST_TRAVELS_REQ)
	{
		return nullptr;
	}

	return nullptr;
}

MobileCodeRspEv* UserEventHandler::handle_mobile_code_req(MobileCodeReqEv* ev)
{
	// 1 get the mobile
	std::string mobile = ev->get_mobile();
	// 2 get the verification code
	i32 icode = icode_gen();
	// 3 synchronize and recode [mobile:verification code]
	pthread_mutex_lock(&pm_);
	m2c_[mobile] = icode;
	pthread_mutex_unlock(&pm_);

	LOG_DEBUG("UserEventHandler::handle_mobile_code_req - mobile phone: %s, valiadate code: %d\n", mobile.c_str(), icode);
	return new MobileCodeRspEv(ERRC_SUCCESS, icode);
}

i32 UserEventHandler::icode_gen()
{
	// generate a number from 100000 to 999999 
	i32 code = (i32)(rand() % (900000) + 100000);
	return code;
}

LoginResEv* UserEventHandler::handle_login_req(LoginReqEv* ev)
{
	// 1 check the valiadate code in the m2c_ map
	std::string mobile = ev->get_mobile();
	i32 icode = ev->get_icode();
	LOG_DEBUG("UserEventHandler::handle_login_req - mobile phone: %s, valiadate code: %d\n", mobile.c_str(), icode);

	pthread_mutex_lock(&pm_);
	auto iter = m2c_.find(mobile);
	if (iter == m2c_.end() || iter != m2c_.end() && icode != iter->second)
	{
		pthread_mutex_unlock(&pm_);
		return new LoginResEv(ERRC_INVALID_DATA);
	}
	pthread_mutex_unlock(&pm_);

	// 2 check the user info in the database
	// create a new connection to the database (under multiple threads)
	// database operations are atomic operations
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	st_env_config conf_args = Iniconfig::getInstance()->getconfig();

	bool ret = mysqlconn->Init(conf_args.db_ip_.c_str(), conf_args.db_port_, conf_args.db_user_.c_str(), conf_args.db_pwd_.c_str(), conf_args.db_name_.c_str());
	if (!ret)
	{
		LOG_ERROR("UserEventHandler::handle_login_req - mysqlconn->Init failed\n");
		return new LoginResEv(ERRC_PROCCESS_FAILED);
	}

	// check if it exists
	UserService user_svc(mysqlconn);
	if (!user_svc.exist(mobile))
	{
		// not exists -> insert
		ret = user_svc.insert(mobile);
		if (!ret)
		{
			LOG_ERROR("UserEventHandler::handle_login_req - insert failed\n");
			return new LoginResEv(ERRC_PROCCESS_FAILED);
		}
	}

	return new LoginResEv(ERRC_SUCCESS);
}
