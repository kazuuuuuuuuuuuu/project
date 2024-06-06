#ifndef _DISPATCHMSGSERVICE_H_
#define _DISPATCHMSGSERVICE_H_

#include <map>
#include <vector>
#include <queue>
#include "iEvent.h"
#include "event_type.h"
#include "iEventHandler.h"
#include "thread_pool/thread_pool.h"
#include "global_define.h"
#include "NetworkInterface.h"

typedef std::vector<iEventHandler*> T_EventHandlers;
typedef std::map<u32, T_EventHandlers> T_EventHandlersMap;

class DispatchMsgService
{
protected:
	// singleton
	static DispatchMsgService* DMS_;
	// thread pool
	thread_pool_t* tp_;
	// keep the key-value pair [the event id : handlers]
	T_EventHandlersMap subscribers_;
	// service condition
	bool ser_exit_;
	// queue for response_events
	static std::queue<iEvent*> response_events_;
	// mutex for the queue
	static pthread_mutex_t queue_mutex_;
	
protected:
	// singleton -> avoiding create more than one object
	DispatchMsgService();

public:
	virtual ~DispatchMsgService();
	
	// get the singleton
	static DispatchMsgService* getInstance();

	// subscribe and unsubscribe the handler for an event
	virtual void subscribe(u32 eid, iEventHandler* handler);
	virtual void unsubscribe(u32 eid, iEventHandler* handler);

	// open and close the thread pool
	virtual bool open();
	virtual void close();

	// post task (svc and event) to thread pool
	virtual i32 enqueue(iEvent* ev);
	// svc() -> process() -> result in the respond event and add it to the queue
	// call back function for thread cycle (static function!)
	static void svc(void* argv);
	// process() -> handler object -> handle() -> specific_handle_method()
	// distribute the task to the handler object subscribed for the event
	virtual iEvent* process(const iEvent* ev);

	// data -> msg object -> event
	iEvent* parseEvent(const char* message, u32 len, u32 eid);

	// send respond events via the interface
	void handlerAllResponseEvent(NetworkInterface* interface);
	void serialize_Event(iEvent* ev);
};

#endif // !_DISPATCHMSGSERVICE_H_


