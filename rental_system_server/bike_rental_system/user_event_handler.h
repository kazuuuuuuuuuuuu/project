#ifndef _USER_EVENT_HANDLER_H_
#define _USER_EVENT_HANDLER_H_

#include "global_define.h"
#include "iEventHandler.h"
#include "event_df.h"
#include "thread_pool/thread.h"

#include <string>
#include <map>
#include <memory>

class UserEventHandler : public iEventHandler
{
private:
	// map[mobile] = verification code
	std::map<std::string, i32> m2c_;
	// mutex for the map
	pthread_mutex_t pm_;

public:
	UserEventHandler();
	virtual ~UserEventHandler();
	virtual iEvent* handle(const iEvent* ev);

private:
	// genertate the verification code
	i32 icode_gen();
	// convert request ev to respond ev
	MobileCodeRspEv* handle_mobile_code_req(MobileCodeReqEv* ev);
	LoginResEv* handle_login_req(LoginReqEv* ev);
};

#endif // !_USER_EVENT_HANDLER_H_

