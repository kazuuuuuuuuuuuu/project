#ifndef _EVENT_TYPE_H_
#define _EVENT_TYPE_H_

#include "global_define.h"

// event type id
enum EventID
{
	EVENT_GET_MOBILE_CODE_REQ      = 0x01,
	EVENT_GET_MOBILE_CODE_RSP      = 0x02,

	EVENT_LOGIN_REQ                = 0x03,
	EVENT_LOGIN_RSP                = 0x04,

	EVENT_RECHARGE_REQ             = 0x05,
	EVENT_RECHARGE_RSP             = 0x06,

	EVENT_GET_ACCOUNT_BALANCE_REQ  = 0x07,
	EVENT_GET_ACCOUNT_BALANCE_RSP  = 0x08,

	EVENT_LIST_ACCOUNT_RECORDS_REQ = 0x09,
	EVENT_LIST_ACCOUNT_RECORDS_RSP = 0x10,

	EVENT_LIST_TRAVELS_REQ         = 0x11,
	EVENT_LIST_TRAVELS_RSP         = 0x12,

	EVENT_EXIT_RSP                 = 0xFE,
	EVENT_UNKOWN                   = 0xFF
};

// response status code
enum ErrorCode
{
	ERRC_SUCCESS                    = 200,
	ERRC_INVALID_MSG                = 400,
	ERRC_INVALID_DATA               = 404,
	ERRC_METHOD_NOT_ALLOWED         = 405,
	ERRC_PROCCESS_FAILED            = 406,
	ERRC_BIKE_IS_TOOK               = 407,
	ERRC_BIKE_IS_RUNNING            = 408,
	ERRC_BIKE_IS_DAMAGED            = 409,
	ERRC_UNDEFINED                  = 0
};

const char* getErrorMsg(i32 code);

#endif // !_EVENT_TYPE_H_


