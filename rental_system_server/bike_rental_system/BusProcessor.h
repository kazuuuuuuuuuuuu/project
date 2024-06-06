#ifndef _BUSPROCESSOR_H_
#define _BUSPROCESSOR_H_

#include "user_event_handler.h"
#include "sql/sqlconnection.h"
#include <memory>

class BusinessProcessor
{
private:
	std::shared_ptr<MysqlConnection> sqlconn_;
	std::shared_ptr<UserEventHandler> ueh_;

public:
	BusinessProcessor(std::shared_ptr<MysqlConnection> sqlconn);
	virtual ~BusinessProcessor();
	bool init();
};



#endif
