/*************************************************************************
    > File Name: web_server.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2022年09月21日 星期三 00时06分51秒
 ************************************************************************/
//web服务端程序--使用epoll模型
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>

#include "pub.h"
#include "wrap.h"

int http_request(int cfd);

int main()
{
	//改变当前进程的工作目录
	char path[255] = {0};
	sprintf(path, "%s/%s", getenv("HOME"), "webpath");
	chdir(path);
	
	//创建socket--设置端口复用---bind
	int lfd = tcp4bind(9999, NULL);
	
	//设置监听
	Listen(lfd, 128);

	//创建epoll树
	int epfd = epoll_create(1024);
	if(epfd<0)
	{
		perror("epoll_create error");
		close(lfd);
		return -1;
	}
	
	//将监听文件描述符lfd上树
	struct epoll_event ev;
	ev.data.fd = lfd;
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	
	int i;
	int cfd;
	int nready;
	int sockfd;
	struct epoll_event events[1024];
	while(1)
	{
		//等待事件发生
		nready = epoll_wait(epfd, events, 1024, -1);
		if(nready<0)
		{
			if(errno==EINTR)
			{
				continue;
			}
			break;
		}
		
		for(i=0; i<nready; i++)
		{
			sockfd = events[i].data.fd;
			//有客户端连接请求
			if(sockfd==lfd)
			{
				//接受新的客户端连接
				cfd = Accept(lfd, NULL, NULL);
				
				//设置cfd为非阻塞
				int flag = fcntl(cfd, F_GETFL);
				flag |= O_NONBLOCK;
				fcntl(cfd, F_SETFL, flag);
				
				//将新的cfd上树
				ev.data.fd = cfd;
				ev.events = EPOLLIN;
				epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
			}
			else 
			{
				//有客户端数据发来
				http_request(cfd);
			}			
		}		
	}
}
