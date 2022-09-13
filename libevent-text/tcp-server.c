/*************************************************************************
  > File Name: tcp-client.c
  > Author: ma6174
  > Mail: ma6174@163.com 
  > Created Time: 2022年09月12日 星期一 23时23分32秒
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <event2/event.h>
#include <ctype.h>

struct event *connev = NULL;

//typedef void (*event_callback_fn)(evutil_socket_t fd, short events, void *arg) 回调函数原型
void readcb(evutil_socket_t fd, short events, void *arg){
	int n;
	char buf[1024];
	memset(buf,0x00,sizeof(buf));
	n = read(fd , buf, sizeof(buf));
	if(n <=0 ){
		close(fd);
		//将通信文件描述符从对应的地基上删除
		event_del(connev);
	}
	else{
		printf("n == [%d],buf == [%s]\n",n,buf);
		for(int i = 0; i < n ;i++){
			buf[i] = toupper(buf[i]); 
		}
		write(fd , buf,sizeof(buf));
	}
}



void conncb(evutil_socket_t fd, short events, void *arg){
	struct event_base *base = (struct event_base*)arg;

	//接受新的客户端
	int cfd = accept(fd, NULL ,NULL);
	if(cfd > 0){
		//创建通信描述符对应的事件，并且设置回调函数readcb
		connev = event_new(base, cfd ,EV_READ|EV_PERSIST, readcb , NULL);
		if(!connev){
			//退出循环
			event_base_loopexit(base , NULL);
		}

		//将通信文件描述符对应的事件上event_base地基
		event_add(connev , NULL);
	}
}

int main(){
	//创建socket
	int lfd = socket(AF_INET,SOCK_STREAM,0);

	//设置端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET , SO_REUSEADDR , &opt, sizeof(opt));


	//绑定
	struct sockaddr_in serv;
	bzero(&serv , sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(8888);
	bind(lfd , (struct sockaddr*)&serv, sizeof(serv));

	//监听
	listen(lfd,120);

	//创建地基
	struct event_base *base = event_base_new();
	if(!base){
		printf("event_base_new error\n");
		return -1;
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
	event_base_dispatch(base);

	//释放资源
	event_base_free(base);
	event_free(ev);

	close(lfd);

	return 0;
}

