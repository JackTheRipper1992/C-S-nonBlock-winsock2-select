// server20170824.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <WinSock2.h>
#include <string>
#include <set>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

fd_set g_inset;
set<SOCKET> g_clients;

int InitSock()
{
	WSAData wsa_data;
	WORD version = MAKEWORD(2, 2);
	int err=WSAStartup(version, &wsa_data);
	if (err != 0)
	{
		return	0;
	}
	if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
	{
		return 0;
	}
	return 0;
}

sockaddr_in AddrInCreate(int port)
{
	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	return addr_in;
}

//设置socket为非阻塞
void SetNonBlock(SOCKET s,bool b)
{
	ULONG non_block = b;
	ioctlsocket(s, FIONBIO, &non_block);
}
void CleanSock()
{
	WSACleanup();
}

//处理绑定在select函数里的处于listen状态的socket，将他们accept
void ProcessAccept(SOCKET &s, int &max_fd)
{
	//这里accept是非阻塞的。之前已经设置过
	SOCKET client = accept(s, NULL, NULL);
	if (client != INVALID_SOCKET)//如果接受的socket描述符不是无效的
	{
		g_clients.insert(client);//把一个就绪的socket存入全局set<SOCKET>容器中
		FD_SET(client, &g_inset);
		//同时将accept返回的socket设置为非阻塞，这样如果要recv（）时不会阻塞
		SetNonBlock(client, true);
	}
}

//处理接受
bool ProcessRecv(SOCKET &client)
{
	char buf[512];
	//接受数据并打印
	int n = recv(client, buf, sizeof(buf), 0);//如果一次没有读取完，会继续读取
	if (n > 0)//读取到数据
	{
		printf("Server revv %d byte word:%s\n", n, buf);
		send(client, buf, n, 0);//这里n等于buf 的内容“hello”加上一个结尾符"\0" 共6个字节，也等于strlen（buf）+1
	}
	else if (n == 0)
	{
		printf("Close socket!!\n");
		return false;
	}
	return true;
}



int _tmain(int argc, _TCHAR* argv[])
{
	int err = 0;//返回状态
	int max_fd = 0;
	InitSock();

	//创建套接字
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
	SetNonBlock(server,true);//将server设置为非阻塞模式


	//绑定
	sockaddr_in addr = AddrInCreate(8888);
	err=bind(server, (sockaddr*)&addr, sizeof(addr));
	

	//监听
	err = listen(server, 10);
	
	//设置select时间
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 1000;
	
	//清空g_inset集合，这里建立的是server，所以是可读集合
	FD_ZERO(&g_inset);
	//把server的监听socket添加到集合
	FD_SET(server, &g_inset);

	do
	{
		//每次循环开始都传递g_inset
		fd_set temp_set = g_inset;//在每个循环里不停地向temp_set添加可读的socket，然后对socket的就休状态进行不停检测，可读的记录到全局g_clients
		//监听文件描述的集合set里是否有可读变化，如果在有效时间内监听到可读的套接字，result会得到可读套接字数，如果没有得到有效信息则进入下一个循环
		int result = select(max_fd, &temp_set, NULL, NULL, &t);
		if (result > 0)//如果set可读文件描述集合里有就绪的信息
		{
			if (FD_ISSET(server, &temp_set))//如果就绪的信息是server描述符
			{
				ProcessAccept(server, max_fd);//这里执行accept操作并将返回的SOCKET设置为非阻塞
				--result;
			}
			//判断是否有客户端套接字就绪，有则处理
			for (set<SOCKET>::iterator i = g_clients.begin(); i != g_clients.end()&&result>0;)
			{	
				SOCKET client = *i;
				if (FD_ISSET(client, &temp_set))
				{
					--result;
					//如果没有读取到数据，则说明客户端需要关闭，则移除该socket
					if (!ProcessRecv(client))
					{
						FD_CLR(client, &g_inset);
						g_clients.erase(*i);
						continue;
					}
				}








				++i;
			}
		}

	} while (true);
	CleanSock();
	return 0;
}
