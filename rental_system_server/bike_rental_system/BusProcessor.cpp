#include "BusProcessor.h"

BusinessProcessor::BusinessProcessor(std::shared_ptr<MysqlConnection> sqlconn):
	sqlconn_(sqlconn), ueh_(new UserEventHandler())
{

}

BusinessProcessor::~BusinessProcessor()
{
	// 智能指针不需要显示delete 
	// 智能指针delete方法 ueh_.reset();
}

bool BusinessProcessor::init()
{
	SqlTables tables(sqlconn_);
	tables.CreateUserInfo();
	tables.CreateBikeTable();
	return true;
}