// client20170825.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <WinSock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")
int InitSock()
{
	WSAData wsa_data;
	WORD version = MAKEWORD(2, 2);
	int err = WSAStartup(version, &wsa_data);
	if (err != 0)
	{
		return	-1;
	}
	if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
	{
		return -1;
	}
	return 0;
}

sockaddr_in AddrInCreate(int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	return addr;
}

sockaddr_in AddrInCreate(int port, const char* ip_addr)
{
	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);//host to net short
	addr_in.sin_addr.S_un.S_addr = inet_addr(ip_addr);
	return addr_in;
}
void CleanSock()
{
	WSACleanup();
}

int _tmain(int argc, _TCHAR* argv[])
{
	int err = 0;
	char buf[512];
	InitSock();

	//创建一个socket
	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	

	//链接
	sockaddr_in addr_in = AddrInCreate(8888, "127.0.0.1");
	err = connect(client, (sockaddr*)&addr_in, sizeof(addr_in));
	

	//发送数据
	sprintf(buf, "hello");
	int number = send(client, buf, strlen(buf) + 1, 0);

	//接受数据
	number = recv(client, buf, sizeof(buf), 0);
	printf("Client received %d words:%s", number, buf);

	//关闭链接

	
	closesocket(client);
	getchar();
	CleanSock();








	return 0;
}

