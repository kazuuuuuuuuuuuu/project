#ifndef _NETWORKINTERFACE_H_
#define _NETWORKINTERFACE_H_

#include <event.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <string>
#include "iEvent.h"
#include "global_define.h"

// header to avoid TCP packet splitting and sticking
// header identifier(4 bytes), event type(2 bytes), message length(4 bytes), message
#define MESSAGE_HEADER_LEN 10
#define MESSAGE_HREADER_ID "FBEB"

enum class SESSION_STATUS
{
	SS_REQUEST,
	SS_RESPONSE
};

enum class MESSAGE_STATUS
{
	MS_READ_HEADER,
	MS_READ_MESSAGE,
	MS_READ_DONE,
	MS_SENDING
};

typedef struct ConnectSession_
{
	// connection info
	i32 fd;
	char remote_ip[32];

	// read/write buffer
	struct bufferevent* bev;

	// event info
	u32 eid;
	SESSION_STATUS session_stat;

	// request status
	MESSAGE_STATUS req_status;
	// response status
	MESSAGE_STATUS rsp_status;

	// header
	char header[MESSAGE_HEADER_LEN + 1];
	u32 read_header_len;

	// request messgae
	char* read_buf;
	u32 message_len;
	u32 read_message_len;

	// response message
	char* write_buf;
	u32 sent_len;

	ConnectSession_() :
		bev(nullptr), read_buf(nullptr), write_buf(nullptr)
	{
	}

}ConnectSession;

// wrapper class for libevent -> Single - Threaded Application
class NetworkInterface
{
private:
	struct event_base* base_;
	struct evconnlistener* listener_;

public:
	NetworkInterface();
	~NetworkInterface();

	// start the server
	bool start(int port);
	// close the server
	void close();

	// static function -> callback function -> avoid this as the first parameter
	// connection
	static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sock, int socklen, void* arg);
	// request
	static void handle_request(struct bufferevent* bev, void* arg);
	// response
	static void handle_response(struct bufferevent* bev, void* arg);
	// disconnection or error
	static void handle_error(struct bufferevent* bev, short event, void* arg);

	// dispatch once
	void network_event_dispatch();
	// send data back to client
	void send_response_message(ConnectSession* cs);
};

#endif // !_NETWORKINTERFACE_H_

