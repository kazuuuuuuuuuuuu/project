#ifndef _SQLCONNECTION_H_
#define _SQLCONNECTION_H_

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <string>
#include <assert.h>
#include <memory>
#include "../global_define.h"

// The result set returned by database operations
class SqlRecordSet
{
private:
	MYSQL_RES* record_;

public:
	SqlRecordSet() :
		record_(nullptr)
	{
	}

	explicit SqlRecordSet(MYSQL_RES* record)
	{
		record_ = record;
	}

	~SqlRecordSet()
	{
		if (record_)
		{
			mysql_free_result(record_);
		}
	}

	MYSQL_RES* GetRecord()
	{
		return record_;
	}

	void SetResult(MYSQL_RES* record)
	{
		assert(record_ == nullptr);
		if (record_)
		{
			LOG_WARN("SqlRecordSet::SetResult - the MYSQL_RES has a result already\n");
			return;
		}
		record_ = record;
	}

	void FetchRow(MYSQL_ROW& row)
	{
		row = mysql_fetch_row(record_);
	}

	// return the number of rows in the result set
	i32 GetRowCount()
	{
		return record_->row_count;
	}
};

// sql handle
class MysqlConnection
{
private:
	MYSQL* mysql_;

public:
	MysqlConnection(); // allocate the memory for mysql_
	~MysqlConnection(); // close the database and release resource

	MYSQL* get_mysql() { return mysql_; }

	// connect to the sql
	bool Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);
	// operation without a result set 
	bool Execute(const char* szSql);
	// operation with a result set 
	bool Execute(const char* szSql, SqlRecordSet& recordSet); // MYSQL_RES -> the type of the result set
	// reconnect 
	void Reconnect();
	// get error msg
	const char* GetErrInfo();
	// escape some special character to avoid sql injection attacks
	int EscapeString(const char* pSrc, int nSrcLen, char* pDest);
};

// create two tables if not exist
class SqlTables
{
private:
	std::shared_ptr<MysqlConnection> sqlconn_;

public:
	SqlTables(std::shared_ptr<MysqlConnection> sqlconn) :
		sqlconn_(sqlconn)
	{
	}

	bool CreateUserInfo()
	{
		const char* pUserInfoTable = "\
			CREATE TABLE IF NOT EXISTS userinfo(\
			id          int(16)          NOT NULL primary key auto_increment,\
			mobile      varchar(16)      NOT NULL default '13000000000',\
			username    varchar(128)     NOT NULL default '',\
			verify      int(4)           NOT NULL default '0',\
			registertm  timestamp        NOT NULL default CURRENT_TIMESTAMP,\
			money       int(4)           NOT NULL default 0,\
			INDEX       mobile_index(mobile)\
			)";

		if (!sqlconn_->Execute(pUserInfoTable))
		{
			LOG_ERROR("CreateUserInfo failed: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}

	bool CreateBikeTable()
	{
		const char* pBikeInfoTable = "\
			CREATE TABLE IF NOT EXISTS bikeinfo(\
			id          int              NOT NULL primary key auto_increment,\
			devno       int              NOT NULL,\
			status      tinyint(1)       NOT NULL default 0,\
			trouble     int              NOT NULL default 0,\
			tmsg        varchar(256)     NOT NULL default '',\
			latitude    double(10,6)     NOT NULL default 0,\
			longtitude  double(10,6)     NOT NULL default 0,\
			unique(devno)\
			)";

		if (!sqlconn_->Execute(pBikeInfoTable))
		{
			LOG_ERROR("CreateBikeTable failed: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}
};

// insert user info to the database if not exist
class UserService
{
private:
	std::shared_ptr<MysqlConnection> sqlconn_;

public:
	UserService(std::shared_ptr<MysqlConnection> sqlconn);
	// find the user by mobile
	bool exist(const std::string& mobile);
	bool insert(const std::string& mobile);
};

#endif
