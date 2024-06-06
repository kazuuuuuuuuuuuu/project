#include "DispatchMsgService.h"
#include "protobuf/bike.pb.h"
#include "NetworkInterface.h"
#include "event_df.h"
#include <algorithm>

// static member
DispatchMsgService* DispatchMsgService::DMS_ = nullptr;
std::queue<iEvent*> DispatchMsgService::response_events_;
pthread_mutex_t DispatchMsgService::queue_mutex_;

DispatchMsgService::DispatchMsgService() :
	tp_(nullptr), ser_exit_(true)
{
}

DispatchMsgService::~DispatchMsgService()
{

}

bool DispatchMsgService::open()
{
	thread_mutex_create(&queue_mutex_);

	tp_ = thread_pool_init();

	ser_exit_ = tp_ ? false : true;

	return tp_ ? true : false;
}

void DispatchMsgService::close()
{
	thread_mutex_destroy(&queue_mutex_);

	thread_pool_destroy(tp_);
	tp_ = nullptr;

	ser_exit_ = true;

	subscribers_.clear();
}

DispatchMsgService* DispatchMsgService::getInstance()
{
	if (DMS_ == nullptr)
	{
		DMS_ = new DispatchMsgService();
	}
	return DMS_;
}

i32 DispatchMsgService::enqueue(iEvent* ev)
{
	if (ev == nullptr)
	{
		return -1;
	}

	// 1 create the task
	thread_task_t* task = thread_task_alloc(0);
	task->handler = svc;
	task->ctx = (void*)ev;
	// 2 post it to the task queue
	return (i32)thread_task_post(tp_, task);
}

void DispatchMsgService::svc(void* argv)
{
	DispatchMsgService* dms = DispatchMsgService::getInstance();
	iEvent* ev = (iEvent*)argv;
	if (!dms->ser_exit_)
	{
		LOG_DEBUG("DispatchMsgService::svc is running\n");
		iEvent* rsp = dms->process(ev);
		// normal response event
		if (rsp)
		{
			// bind the connection structure with the response event
			rsp->set_args(ev->get_args());
			rsp->dump(std::cout);
		}
		// no need to reply -> generate response termination event
		else
		{
			// bind the connection structure with the exit event
			rsp = new ExitRspEv(ev->get_args());
			rsp->dump(std::cout);
		}
		thread_mutex_lock(&queue_mutex_);
		response_events_.push(rsp);
		thread_mutex_unlock(&queue_mutex_);
	}
	delete ev;
}

iEvent* DispatchMsgService::process(const iEvent* ev)
{
	if (ev == nullptr)
	{
		return nullptr;
	}
	
	u32 eid = ev->get_eid();
	LOG_DEBUG("DispatchMsgService::process - eid: %d\n", eid);
	if (eid == EVENT_UNKOWN)
	{
		LOG_WARN("DispatchMsgService::process - unknown eid: %d\n", eid);
		return nullptr;
	}

	T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
	if (handlers == subscribers_.end())
	{
		LOG_WARN("DispatchMsgService::process - no any event handler subscribed: %d\n", eid);
		return nullptr;
	}

	// traverse the handler objects subscribed
	iEvent* rsp = nullptr;
	for (T_EventHandlers::iterator iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
	{
		iEventHandler* handler = *iter;
		LOG_DEBUG("DispatchMsgService::process - get handler: %s\n", handler->get_name().c_str());
		// call virtual function handle() -> specific_handle_method()
		rsp = handler->handle(ev);

		// if there are more handler objects -> should return vector<rsp>
		break;
	}
	return rsp;
}

// subscribers_[eid] = vector<handler object>
void DispatchMsgService::subscribe(u32 eid, iEventHandler* handler)
{
	LOG_DEBUG("DispatchMsgService::subscribe eid: %u, handler: %s\n", eid, handler->get_name().c_str());
	// check if the handler already exists -> if not, push back it
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())
	{
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
		if (hdl_iter == iter->second.end())
		{
			iter->second.push_back(handler);
		}
	}
	else
	{
		subscribers_[eid].push_back(handler);
	}
}

void DispatchMsgService::unsubscribe(u32 eid, iEventHandler* handler)
{
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())
	{
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
		if (hdl_iter != iter->second.end())
		{
			iter->second.erase(hdl_iter);
		}
	}
}

iEvent* DispatchMsgService::parseEvent(const char* message, u32 len, u32 eid)
{
	if (message == nullptr)
	{
		LOG_ERROR("DispatchMsgService::parseEvent - message is nullptr, eid: %d\n", eid);
		return nullptr;
	}

	if (eid == EVENT_GET_MOBILE_CODE_REQ)
	{
		rental_system::mobile_request mr;
		if (mr.ParseFromArray(message, len))
		{
			MobileCodeReqEv* ev = new MobileCodeReqEv(mr.mobile());
			return ev;
		}
	}
	else if (eid == EVENT_LOGIN_REQ)
	{
		rental_system::login_request lr;
		if (lr.ParseFromArray(message, len))
		{
			LoginReqEv* ev = new LoginReqEv(lr.mobile(), lr.icode());
			return ev;
		}
	}

	// other events
	LOG_ERROR("DispatchMsgService::parseEvent - event is not implemented, eid: %d\n", eid);
	return nullptr;
}

// response event -> data -> call NetworkInterface::send_response_message() -> buf_write
void DispatchMsgService::handlerAllResponseEvent(NetworkInterface* interface)
{
	while (1)
	{
		// 1 take out one event from the queue
		iEvent* ev = nullptr;
		thread_mutex_lock(&queue_mutex_);
		if (response_events_.empty())
		{
			thread_mutex_unlock(&queue_mutex_);
			break;
		}
		else
		{
			ev = response_events_.front();
			response_events_.pop();
			thread_mutex_unlock(&queue_mutex_);
		}

		// 2 get cs
		ConnectSession* cs = (ConnectSession*)ev->get_args();

		// 3 serialize for each kind of event and set up the send buffer
		serialize_Event(ev);

		// 3 send by network interface
		interface->send_response_message(cs);
	}
}

void DispatchMsgService::serialize_Event(iEvent* ev)
{
	LOG_DEBUG("DispatchMsgService::deserialize_Event - eid: % d\n", ev->get_eid());
	// 1 get the cs
	ConnectSession* cs = (ConnectSession*)ev->get_args();

	// 2 update info
	cs->eid = ev->get_eid();
	cs->sent_len = ev->ByteSize();
	cs->write_buf = new char[MESSAGE_HEADER_LEN + cs->sent_len];

	// 3 prepare header
	memcpy(cs->write_buf, MESSAGE_HREADER_ID, strlen(MESSAGE_HREADER_ID));
	*(u16*)(cs->write_buf + 4) = (u16)ev->get_eid();
	*(u32*)(cs->write_buf + 6) = cs->sent_len;

	// 4 prepare message
	ev->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->sent_len);

	// 5 delete the response event
	delete ev;
}