#include "sqlconnection.h"
#include <string.h>

MysqlConnection::MysqlConnection()
{
	mysql_ = (MYSQL*)malloc(sizeof(MYSQL)); // sql interface is using c 
}

MysqlConnection::~MysqlConnection()
{
	if (mysql_ != nullptr)
	{
		mysql_close(mysql_);
		free(mysql_);
		mysql_ = nullptr;
	}
}

bool MysqlConnection::Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb)
{
	// 1 initialize sql handle
	if ((mysql_ = mysql_init(mysql_)) == NULL)
	{
		LOG_ERROR("MysqlConnection::Init - mysql_init failed: %s, %d", GetErrInfo(), errno);
		return false;
	}

	// 2 reconnection enabled
	char cAuto = 1;
	if (mysql_options(mysql_, MYSQL_OPT_RECONNECT, &cAuto)!=0) // 成功返回0
	{
		LOG_ERROR("MysqlConnection::Init - MYSQL_OPT_RECONNECT failed: %s, %d", GetErrInfo(), errno);
	}

	// 3 connect
	if (mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb, nPort, NULL, 0) == NULL)
	{
		LOG_ERROR("MysqlConnection::Init - mysql_real_connect failed: %s, %d", GetErrInfo(), errno);
		return false;
	}

	LOG_INFO("sql initialisation is successful\n");
	return true;
}

bool MysqlConnection::Execute(const char* szSql)
{
	if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
	{
		// disconnected -> reconnect
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
		{
			Reconnect();
		}
		return false;
	}
	return true;
}

bool MysqlConnection::Execute(const char* szSql, SqlRecordSet& recordSet) // MYSQL_RES
{
	if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
	{
		// disconnected -> reconnect
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
		{
			Reconnect();
		}
		return false;
	}

	// get the result
	MYSQL_RES* record = mysql_store_result(mysql_);
	if (!record)
	{
		return false;
	}
	recordSet.SetResult(record);
	return true;
}

void MysqlConnection::Reconnect()
{
	mysql_ping(mysql_);
}

const char* MysqlConnection::GetErrInfo()
{
	return mysql_error(mysql_);
}

int MysqlConnection::EscapeString(const char* pSrc, int nSrcLen, char* pDest)
{
	if (!mysql_)
	{
		return 0;
	}

	return mysql_real_escape_string(mysql_, pDest, pSrc, nSrcLen);
}

UserService::UserService(std::shared_ptr<MysqlConnection> sqlconn) :
	sqlconn_(sqlconn)
{
}

bool UserService::exist(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "select * from userinfo where mobile = %s", mobile.c_str());

	SqlRecordSet record_set;
	if (!sqlconn_->Execute(sql_content, record_set))
	{
		return false;
	}

	return (record_set.GetRowCount() != 0);
}

bool UserService::insert(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "insert into userinfo (mobile) values (%s)", mobile.c_str());
	return sqlconn_->Execute(sql_content);
}