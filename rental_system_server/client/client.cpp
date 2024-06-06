// ip地址和端口号使用硬编码
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <event.h>
#include <event2/event.h>
#include <assert.h> 
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "bike.pb.h"

int icode = 0;

int connect_server();
void event_cb(struct bufferevent *bev, short events, void *arg);
void socket_read_data(struct bufferevent *bev, void *arg);
void cmd_read_data(int fd, short events, void *arg);

int main(int argc, char const *argv[])
{
	int connfd = connect_server();
	if(connfd==-1)
		return -1;

	printf("connect to server successfully\n");
	struct event_base *base = event_base_new();;

	// bev 绑定fd和base
	struct bufferevent* bev = bufferevent_socket_new(base, connfd, BEV_OPT_CLOSE_ON_FREE);

	// 监听stdin上的读事件 
	struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ|EV_PERSIST, cmd_read_data, (void *)bev);
	event_add(ev_cmd, NULL);	

	// bev 绑定回调函数
	bufferevent_setcb(bev, socket_read_data, NULL, event_cb, (void*)base);
	// bev激活 在base上监听它的读事件
	bufferevent_enable(bev, EV_READ | EV_PERSIST);  

	event_base_dispatch(base);

	printf("finished\n");
	return 0;
}

int connect_server()
{
	struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8888);
	server_addr.sin_addr.s_addr = inet_addr("192.168.226.129");	

	int connfd = socket(AF_INET, SOCK_STREAM, 0);
	if(connfd==-1)
	{
		perror("socket failed");
		return -1;
	}

	if(connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0)
	{
		perror("connect failed");
		close(connfd);
		return -1;		
	}

	return connfd;
}

void socket_read_data(struct bufferevent *bev, void *arg)
{
	printf("receiving now..\n");
	char msg[1024];

	int len = bufferevent_read(bev, msg, 1024-1);
	msg[len] = '\0';

	
	if(strncmp(msg, "FBEB", 4)==0)
	{
		unsigned short code = *(unsigned short*)(msg+4);
		int len = *(int*)(msg+6);

		if(code==2)
		{
			tutorial::mobile_response mr;
			mr.ParseFromArray(msg+10, len);
			icode = mr.icode();
			printf("mobile_response: code: %d, icode: %d, data: %s\n", mr.code(), mr.icode(), mr.data().c_str());			
		}
		else if(code==4)
		{
			tutorial::login_response mr;
			mr.ParseFromArray(msg+10, len);
			printf("login_response: code: %d, data: %s\n", mr.code(), mr.desc().c_str());		
		}

	}
}

void cmd_read_data(int fd, short events, void *arg)
{
	char msg[1024];

	int len = read(fd, msg, sizeof(msg)-1);
	if(len<=0)
	{
		printf("reading from cmd failed\n");
		exit(1);
	}
	msg[len] = '\0';

	if(strncmp(msg,"ok2",3)==0)
	{
		std::cout << "sending ok2" << std::endl;
		struct bufferevent* bev = (struct bufferevent*)arg;
	
		tutorial::login_request mr;
		mr.set_mobile("13501293036");
		mr.set_icode(icode);
		len = mr.ByteSize();
		
		memcpy(msg, "FBEB", 4);
		*(unsigned short*)(msg+4) = 3;
		*(int*)(msg+6) = len;
		mr.SerializeToArray(msg+10, len);

		bufferevent_write(bev, msg, len+10);
		return;
	}

	if(strncmp(msg,"ok",2)==0)
	{
		std::cout << "sending ok" << std::endl;
		struct bufferevent* bev = (struct bufferevent*)arg;
	
		tutorial::mobile_request mr;
		mr.set_mobile("13501293036");
		len = mr.ByteSize();
		
		memcpy(msg, "FBEB", 4);
		*(unsigned short*)(msg+4) = 1;
		*(int*)(msg+6) = len;
		mr.SerializeToArray(msg+10, len);

		bufferevent_write(bev, msg, len+10);
		return;
	}

	if(strncmp(msg,"er1",3)==0)
	{
		struct bufferevent* bev = (struct bufferevent*)arg;
	
		tutorial::mobile_request mr;
		mr.set_mobile("13501293036");
		len = mr.ByteSize();
		
		memcpy(msg, "FBEA", 4); // modification
		*(unsigned short*)(msg+4) = 1;
		*(int*)(msg+6) = len;
		mr.SerializeToArray(msg+10, len);

		bufferevent_write(bev, msg, len+10);
		return;
	}

	if(strncmp(msg,"er2",3)==0)
	{
		struct bufferevent* bev = (struct bufferevent*)arg;
	
		tutorial::mobile_request mr;
		mr.set_mobile("13501293036");
		len = mr.ByteSize();
		
		memcpy(msg, "FBEB", 4); 
		*(unsigned short*)(msg+4) = 2; // modification
		*(int*)(msg+6) = len;
		mr.SerializeToArray(msg+10, len);

		bufferevent_write(bev, msg, len+10);
		return;
	}

	if(strncmp(msg,"er3",3)==0)
	{
		struct bufferevent* bev = (struct bufferevent*)arg;
	
		tutorial::mobile_request mr;
		mr.set_mobile("13501293036");
		len = mr.ByteSize();
		
		memcpy(msg, "FBEB", 4); 
		*(unsigned short*)(msg+4) = 1;
		*(int*)(msg+6) = INT_MAX; // modification
		mr.SerializeToArray(msg+10, len);

		bufferevent_write(bev, msg, len+10);
		return;
	}

}

void event_cb(struct bufferevent *bev, short events, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	if(events & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if(events & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}

	// 自动关闭套接字和free读写缓冲区
	bufferevent_free(bev);
	event_base_free(base);
}