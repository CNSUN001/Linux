/*************************************************************************
    > File Name: 02-server.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2022年09月13日 星期二 14时25分14秒
 ************************************************************************/

//上一个版本的改进 ，对fd和event做映射关系
//解决关闭客户端出现异常的情况
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <event2/event.h>

#define MAX 1000

//定义一个fd和事件的映射结构体
struct event_fd{
	evutil_socket_t fd;
	struct event*ev; 
}event[MAX];

void init_ev_fd(){
	for(int i = 0 ; i < MAX;i++){
		event[i].fd = -1;
		event[i].ev = NULL;
	}
}




void setEventFd(evutil_socket_t fd, struct event*ev){
	//查找存放的位置
	int i = 0;
	for( i = 0 ; i< MAX;i++){
		if(event[i].fd == -1)
			break; //查找到存放的位置
	}
	if(i == MAX)//找不到合适的位置
		exit(1);
	event[i].fd = fd;
	event[i].ev = ev;
}

int findEv(int fd){
	int i = 0;
	for( i = 0 ;i < MAX ;i++){
		if(event[i].fd == fd){
			break;
		}
	}
	if(i == MAX){
		printf("not find fd\n");
		exit(1);
	}
	return i;
}




void readcb(evutil_socket_t connfd ,short events,void *args){
	char buf[1024];
	memset(buf,0x00,sizeof(buf));

	int num = findEv(connfd);//找到数组中的位置
	int n = read(connfd ,buf ,sizeof(buf));
	if(n < 0){
		perror("read error");
		close(connfd);
		event_del(event[num].ev);//将通信文件描述符从对应的base地基上删除
		event[num].fd = -1;
		event[num].ev = NULL;
	}
	else if(n == 0){
		printf("client close\n");
		close(connfd);
		event_del(event[num].ev);
		printf("[%p],[%d]\n",event[num].ev,num);
		event[num].fd = -1;
		event[num].ev = NULL;
	}
	else{
		for(int i = 0 ; i<n;i++){
			buf[i] = toupper(buf[i]);
		}
		write(connfd,buf,sizeof(buf));
	}
}




void conncb(evutil_socket_t sockfd ,short event, void *args){
	struct event_base *base = (struct event_base*)args;

	//有新的连接请求，调用 accept接受客户端连接
	struct sockaddr_in cliaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	//接受新的客户端连接
	int connfd = accept(sockfd,(struct sockaddr*)&cliaddr ,&addrlen);
	if(connfd < 0){
		perror("accept error");
		exit(1);
	}
	//打印客户端IP和PORT
	char sIP[128];
	memset(sIP,0x00,sizeof(sIP));
	printf("received from %s at port %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr,sIP,sizeof(sIP)),ntohs(cliaddr.sin_port));

	//将新的文件描述符上树
	struct event *readev = event_new(base,connfd,EV_READ|EV_PERSIST,readcb,base);
	event_add(readev,NULL);
	setEventFd(connfd,readev);
}



int main(){
	//初始化事件和文件描述符映射表
	init_ev_fd();

	//创建socket
	int lfd = socket(AF_INET , SOCK_STREAM, 0);

	//设置端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET ,SO_REUSEADDR ,&opt ,sizeof(opt));

	//绑定
	struct sockaddr_in serv;
	bzero(&serv ,sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(8888);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(lfd,(struct sockaddr*)&serv,sizeof(serv));	

	//监听
	listen(lfd , 120);

	//创建地基
	struct event_base *base = event_base_new();
	if(!base){
		printf("event_base_new error\n");
		return -1;
	}

	//创建监听文件描述符号对应的事件
	////struct event *event_new(struct event_base *base, evutil_socket_t fd, short events, event_callback_fn cb, void *arg);
	struct event *ev = event_new(base , lfd ,EV_READ | EV_PERSIST , conncb ,base);
	if(!ev){
		printf("event_new error\n");
		return -1;
	}

	//将新的事件节点上base地基
	event_add(ev, NULL);

	//进入事件循环等待
	event_base_dispatch(base);	

	//释放资源
	event_base_free(base);
	event_free(ev);

	close(lfd);

	return 0;
}

