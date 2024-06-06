#include "event_type.h"
#include <unordered_map>

static std::unordered_map<i32, const char*> ERRC =
{
	{ERRC_SUCCESS, "Ok."},
	{ERRC_INVALID_MSG, "Invalid message."},
	{ERRC_INVALID_DATA, "Invalid data."},
	{ERRC_METHOD_NOT_ALLOWED, "Method not allowed."},
	{ERRC_PROCCESS_FAILED, "Pocess failed."},
	{ERRC_BIKE_IS_TOOK, "Bike is took."},
	{ERRC_BIKE_IS_RUNNING, "Bike is running."},
	{ERRC_BIKE_IS_DAMAGED, "Bike is damaged."},
	{ERRC_UNDEFINED, "Undefined."}
};

const char* getErrorMsg(i32 code)
{
	if (ERRC.find(code) == ERRC.end())
	{
		return ERRC[ERRC_UNDEFINED];
	}
	return ERRC[code];
}