#include "BusProcessor.h"

BusinessProcessor::BusinessProcessor(std::shared_ptr<MysqlConnection> sqlconn):
	sqlconn_(sqlconn), ueh_(new UserEventHandler())
{

}

BusinessProcessor::~BusinessProcessor()
{
	// ����ָ�벻��Ҫ��ʾdelete 
	// ����ָ��delete���� ueh_.reset();
}

bool BusinessProcessor::init()
{
	SqlTables tables(sqlconn_);
	tables.CreateUserInfo();
	tables.CreateBikeTable();
	return true;
}