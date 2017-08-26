// server20170824.cpp : �������̨Ӧ�ó������ڵ㡣
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

//����socketΪ������
void SetNonBlock(SOCKET s,bool b)
{
	ULONG non_block = b;
	ioctlsocket(s, FIONBIO, &non_block);
}
void CleanSock()
{
	WSACleanup();
}

//�������select������Ĵ���listen״̬��socket��������accept
void ProcessAccept(SOCKET &s, int &max_fd)
{
	//����accept�Ƿ������ġ�֮ǰ�Ѿ����ù�
	SOCKET client = accept(s, NULL, NULL);
	if (client != INVALID_SOCKET)//������ܵ�socket������������Ч��
	{
		g_clients.insert(client);//��һ��������socket����ȫ��set<SOCKET>������
		FD_SET(client, &g_inset);
		//ͬʱ��accept���ص�socket����Ϊ���������������Ҫrecv����ʱ��������
		SetNonBlock(client, true);
	}
}

//�������
bool ProcessRecv(SOCKET &client)
{
	char buf[512];
	//�������ݲ���ӡ
	int n = recv(client, buf, sizeof(buf), 0);//���һ��û�ж�ȡ�꣬�������ȡ
	if (n > 0)//��ȡ������
	{
		printf("Server revv %d byte word:%s\n", n, buf);
		send(client, buf, n, 0);//����n����buf �����ݡ�hello������һ����β��"\0" ��6���ֽڣ�Ҳ����strlen��buf��+1
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
	int err = 0;//����״̬
	int max_fd = 0;
	InitSock();

	//�����׽���
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
	SetNonBlock(server,true);//��server����Ϊ������ģʽ


	//��
	sockaddr_in addr = AddrInCreate(8888);
	err=bind(server, (sockaddr*)&addr, sizeof(addr));
	

	//����
	err = listen(server, 10);
	
	//����selectʱ��
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 1000;
	
	//���g_inset���ϣ����ｨ������server�������ǿɶ�����
	FD_ZERO(&g_inset);
	//��server�ļ���socket��ӵ�����
	FD_SET(server, &g_inset);

	do
	{
		//ÿ��ѭ����ʼ������g_inset
		fd_set temp_set = g_inset;//��ÿ��ѭ���ﲻͣ����temp_set��ӿɶ���socket��Ȼ���socket�ľ���״̬���в�ͣ��⣬�ɶ��ļ�¼��ȫ��g_clients
		//�����ļ������ļ���set���Ƿ��пɶ��仯���������Чʱ���ڼ������ɶ����׽��֣�result��õ��ɶ��׽����������û�еõ���Ч��Ϣ�������һ��ѭ��
		int result = select(max_fd, &temp_set, NULL, NULL, &t);
		if (result > 0)//���set�ɶ��ļ������������о�������Ϣ
		{
			if (FD_ISSET(server, &temp_set))//�����������Ϣ��server������
			{
				ProcessAccept(server, max_fd);//����ִ��accept�����������ص�SOCKET����Ϊ������
				--result;
			}
			//�ж��Ƿ��пͻ����׽��־�����������
			for (set<SOCKET>::iterator i = g_clients.begin(); i != g_clients.end()&&result>0;)
			{	
				SOCKET client = *i;
				if (FD_ISSET(client, &temp_set))
				{
					--result;
					//���û�ж�ȡ�����ݣ���˵���ͻ�����Ҫ�رգ����Ƴ���socket
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
