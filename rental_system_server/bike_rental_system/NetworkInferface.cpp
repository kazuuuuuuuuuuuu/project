#include "NetworkInterface.h"
#include "DispatchMsgService.h"
#include <cstring>
#include <cstdlib>
//#include "log.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static ConnectSession* session_init(int fd, struct bufferevent* bev, struct sockaddr* sock)
{
	ConnectSession* temp = nullptr;
	temp = new ConnectSession();
	if (temp == nullptr)
	{
		LOG_ERROR("session_init - new failed\n");
		return nullptr;
	}

	temp->fd = fd;
	temp->bev = bev;
	strcpy(temp->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));
	temp->session_stat = SESSION_STATUS::SS_REQUEST;
	temp->req_status = MESSAGE_STATUS::MS_READ_HEADER;
	temp->read_header_len = 0;
	temp->message_len = 0;
	temp->read_message_len = 0;
	temp->sent_len = 0;
	return temp;
}

static void session_free(ConnectSession* cs)
{
	if (cs)
	{
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
		delete cs;
		cs = nullptr;
	}
}

static void session_reset(ConnectSession* cs)
{
	if (cs)
	{
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
		cs->session_stat = SESSION_STATUS::SS_REQUEST;
		cs->req_status = MESSAGE_STATUS::MS_READ_HEADER;
		cs->read_header_len = 0;
		cs->message_len = 0;
		cs->read_message_len = 0;
		cs->sent_len = 0;
	}
}

NetworkInterface::NetworkInterface() :
	listener_(nullptr), base_(nullptr)
{
}

NetworkInterface::~NetworkInterface()
{
	close();
}

bool NetworkInterface::start(int port)
{
	struct sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	base_ = event_base_new();
	listener_ = evconnlistener_new_bind(base_, listener_cb, base_, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 512, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	return true;
}

void NetworkInterface::close()
{
	if (base_)
	{
		event_base_free(base_);
		base_ = nullptr;
	}
	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

void NetworkInterface::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sock, int socklen, void* arg)
{
	struct event_base* base = (struct event_base*)arg;
	
	// 1 new a bev (BEV_OPT_CLOSE_ON_FREE -> close fd when free bev)
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	// 2 new a session structure
	ConnectSession* cs = session_init(fd, bev, sock);

	// 3 set the event callback functions
	bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);

	// 4 set timtout -> time out -> call handle_error
	bufferevent_settimeout(bev, 10, 10);

	// 5 register this event
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
	
	LOG_DEBUG("NetworkInterface::listener_cb - accept a client: %d, remote ip: %s\n", cs->fd, cs->remote_ip);
}

void NetworkInterface::handle_request(struct bufferevent* bev, void* arg)
{
	ConnectSession* cs = (ConnectSession*)arg;

	if (cs->session_stat != SESSION_STATUS::SS_REQUEST)
	{
		char temp[4096];
		while (bufferevent_read(bev, temp, sizeof(temp)) > 0)
		{
			// read and discard data
		}
		LOG_ERROR("NetworkInterface::handle_request - wrong session state[%d]\n", cs->session_stat);
		return;
	}

	// read header
	if (cs->req_status == MESSAGE_STATUS::MS_READ_HEADER)
	{
		u32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN - cs->read_header_len);
		cs->read_header_len += len;
		cs->header[cs->read_header_len] = '\0';
		LOG_DEBUG("NetworkInterface::handle_request - received header: %s\n", cs->header);

		// finish
		if (cs->read_header_len == MESSAGE_HEADER_LEN)
		{
			if (strncmp(cs->header, MESSAGE_HREADER_ID, strlen(MESSAGE_HREADER_ID)) == 0)
			{
				// 1 parse header
				cs->eid = *((u16*)(cs->header + 4));
				cs->message_len = *((u32*)(cs->header + 6));
				LOG_DEBUG("NetworkInterface::handle_request - read_header_len: %d, message_len: %d\n", cs->read_header_len, cs->message_len);

				if (cs->message_len<1 || cs->message_len>MAX_MESSAGE_LEN)
				{
					LOG_ERROR("NetworkInterface::handle_request - message length is invalid: %d\n", cs->message_len);
					bufferevent_free(bev);
					session_free(cs);
					return;
				}

				cs->read_buf = new char[cs->message_len];
				cs->req_status = MESSAGE_STATUS::MS_READ_MESSAGE;
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request - invalid header from %s\n", cs->remote_ip);
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
	}
	// read message
	if (cs->req_status == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0)
	{
		u32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
		cs->read_message_len += len;
		LOG_DEBUG("NetworkInterface::handle_request - read %d bytes, total read len: %d, message_len: %d\n", len, cs->read_message_len, cs->message_len);

		// finish
		if (cs->message_len == cs->read_message_len)
		{
			// data -> message object -> event 
			iEvent* ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->message_len, cs->eid);

			delete[] cs->read_buf;
			cs->read_buf = nullptr;
			cs->session_stat = SESSION_STATUS::SS_RESPONSE;

			if (ev)
			{
				// bind the connect session to the event
				ev->set_args(cs);
				// post the ev to the thread pool
				DispatchMsgService::getInstance()->enqueue(ev);
				return;
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request - ev is nullptr, ip: %s, eid: %d\n", cs->remote_ip, cs->eid);
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
	}
}

void NetworkInterface::handle_response(struct bufferevent* bev, void* arg)
{
	LOG_DEBUG("NetworkInterface::handle_response\n");
}

void NetworkInterface::handle_error(struct bufferevent* bev, short event, void* arg)
{
	// connection closed
	if (event & BEV_EVENT_EOF)
	{
		LOG_DEBUG("NetworkInterface::handle_error - connection closed\n");
	}
	// read time out
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING))
	{
		LOG_WARN("NetworkInterface::handle_error - reading timeout\n");
	}
	// write time out
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING))
	{
		LOG_WARN("NetworkInterface::handle_error - writing timeout\n");
	}
	// error
	else if (event & BEV_EVENT_ERROR)
	{
		LOG_ERROR("NetworkInterface::handle_error - other errors\n");
	}

	ConnectSession* cs = (ConnectSession*)arg;
	session_free(cs);
	bufferevent_free(bev);
}

void NetworkInterface::network_event_dispatch()
{
	// nonblock dispatch for once 
	event_base_loop(base_, EVLOOP_NONBLOCK); 

	// handle response
	DispatchMsgService::getInstance()->handlerAllResponseEvent(this);
}

void NetworkInterface::send_response_message(ConnectSession* cs)
{
	if (cs->eid == EVENT_EXIT_RSP)
	{
		bufferevent_free(cs->bev);
		session_free(cs);
	}
	else
	{
		bufferevent_write(cs->bev, cs->write_buf, cs->sent_len + MESSAGE_HEADER_LEN);
		session_reset(cs); // back to read status
	}
}