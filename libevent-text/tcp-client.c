/*************************************************************************
  > File Name: tcp-client.c
  > Author: ma6174
  > Mail: ma6174@163.com 
  > Created Time: 2022年09月12日 星期一 23时23分32秒
 ************************************************************************/

#include<stdio.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <event2/event.h>

//typedef void (*event_callback_fn)(evutil_socket_t fd, short events, void *arg) 回调函数原型

int main(){
	//创建socket
	int lfd = socket(AF_INET,SOCK_STREAM,0);

	//设置端口复用
	int opt = 1;
	setsockopt(fd, SOL_SOCKET , SO_REUSEADDR , &opt, sizeof(opt));


	//绑定
	struct sockaddr_in serv;
	bzero(&serv , sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(8888);
	bind(lfd , (struct sockaddr*)serv, sizeof(serv));

	//监听
	listen(lfd,120);

	//创建地基
	struct event_base *base = event_base_new();
	if(!base){
		printf("event_base_new error\n");
	}

	//创建监听文件描述符对应事件
	//struct event *event_new(struct event_base *base, evutil_socket_t fd, short events, event_callback_fn cb, void *arg);
	struct event *ev = event_new(base ,lfd ,EV_READ|EV_PERSIST , conncb ,base);
	if(!ev){
		printf("event_new error\n");
		return -1;
	}
		

	//将新的事件节点上base地基
	event_add(ev,NULL);

	//进入事件循环等待
	event_base_dispath(base);

	//释放资源
	event_base_free(base);
	event_free(ev);

	close(lfd);

	return 0;
}

