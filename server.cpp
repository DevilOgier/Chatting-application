
//informWinServer.cpp：运行时没有参数，使用端口6666进行侦听：
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include<string.h>
#include <winsock2.h>
#include<time.h>
#define SERVER_PORT	6666 //侦听端口
#define MAX_LENTH 200
#define MAX_CLIENT 10

int id;
typedef struct
{
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;

}Time;

struct clients {
	
	//sockaddr_in c[MAX_CLIENT];//0-9个clients
	char addr[MAX_CLIENT][30];
	long port[MAX_CLIENT];
	int valid[MAX_CLIENT];
	int total;
	SOCKET s[MAX_CLIENT];
};
struct clients Client_list;

struct request
{
	int read=1;
	int type;
	int No;
	Time t;
	char content[MAX_LENTH];
};
struct request req;
typedef struct
{
	char message[MAX_LENTH];
	int from;
	int to;
}MES;

typedef struct
{
	MES m[10];
	int head = 0;
	int end =0;
}Queue;
Queue que;
bool isempty()
{
	if (que.end == que.head)
		return true;
	else
		return false;
}
bool isfull()
{
	if (que.end == que.head - 1 || (que.end == 9 && que.head == 0))
		return true;
	else 
		return false;
}
bool inqueue(MES Mes)
{
	if (isfull())
		return false;
	strcpy(que.m[que.end].message, Mes.message);
	que.m[que.end].from = Mes.from;
	que.m[que.end].to = Mes.to;
	if (que.end == 9)
		que.end = 0;
	else
		que.end++;
	return true;
	
}

MES outqueue()
{
	if (que.head != 9)
	{
		que.head++;
		return que.m[que.head--];
	}
	else
	{
		que.head = 0;
		return que.m[9];
	}
}
void Client_list_initial()
{
	for (int i = 0; i < MAX_CLIENT; i++)
		Client_list.valid[i] = 0;
	 Client_list.total = 0;
}

DWORD WINAPI Process(LPVOID lpParam)
{
	SOCKET* sServer = (SOCKET*)lpParam;
	int ret, nLeft;
	nLeft = sizeof(req);
	char *ptr = (char *)&req;
	char buffer[10];
	int threadid;
	threadid = id; //当前客户端的id
	while (true)
	{
		//接收数据：
		ret = recv(*sServer, ptr, nLeft, 0);
		if (req.read != 0) {
			req.read = 0;
			if (req.type == 2)
			{
				Client_list.valid[threadid] = 0;
				printf("connect break\n");
				return 0;
			}
			else if (req.type == 3)
			{
				time_t now;
				time(&now);
				struct tm *tm_now = localtime(&now);

				req.t.year = tm_now->tm_year + 1900;
				req.t.month = tm_now->tm_mon + 1;
				req.t.day = tm_now->tm_mday;
				req.t.hour = tm_now->tm_hour;
				req.t.min = tm_now->tm_min;
				req.t.sec = tm_now->tm_sec;
				req.read = 0;
				send(*sServer, ptr, sizeof(req), NULL);

			}
			else if (req.type == 4)
			{
				strcpy(req.content, "nameOfserver");
				send(*sServer, ptr, sizeof(req), NULL);
			}
			else if (req.type == 5)
			{
				strcpy_s(req.content, "");
				for (int i = 0; i < MAX_CLIENT; i++)
				{
					if (Client_list.valid[i] == 1)
					{
						strcat_s(req.content, "No.");
						itoa(i, buffer, 3);
						
						strcat_s(req.content, buffer);
						strcat_s(req.content, "  IP:");
						strcat_s(req.content, Client_list.addr[i]);
						strcat_s(req.content, "  Port:");
						itoa(Client_list.port[i], buffer, 10);
						strcat_s(req.content,buffer);
						strcat_s(req.content,"\n");
						
						//printf("No.%d IP:%s Port:%d", i, Client_list.addr[i], Client_list.port[i]);
					}
				}

				send(*sServer, ptr, sizeof(req), NULL);
			}
			else {
				int temp= req.No;
				req.No = threadid;
				send(Client_list.s[temp], ptr, sizeof(req), NULL);
			}
		}
	}
		closesocket(*sServer);
		return 0;
	
}
void main()
{
	HANDLE thread[MAX_CLIENT];
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret, nLeft, length;
	SOCKET sListen, sServer[10]; //侦听套接字，连接套接字
	struct sockaddr_in saServer, saClient;//地址信息
	int t = 0;
	Client_list_initial();
	//WinSock初始化：
	wVersionRequested = MAKEWORD(2, 2);//希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return;
	}
	//确认WinSock DLL支持版本2.2：
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return;
	}

	//创建socket，使用TCP协议：
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return;
	}

	//构建本地地址信息：
	saServer.sin_family = AF_INET;//地址家族
	saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//使用INADDR_ANY指示任意地址

													  //绑定：
	ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("bind() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);//关闭套接字
		WSACleanup();
		return;
	}
	int i = 0;
	
	//侦听连接请求：
	ret = listen(sListen, 10);
	if (ret == SOCKET_ERROR)
	{
		printf("listen() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);//关闭套接字
		WSACleanup();
		return;
	}
	while (t<MAX_CLIENT) {
		printf("Waiting for client connecting!\n");
		printf("tips : Ctrl+c to quit!\n");
		//阻塞等待接受客户端连接：
		length = sizeof(saClient);
		sServer[t] = accept(sListen, (struct sockaddr *)&saClient, &length);
		if (sServer[t] == INVALID_SOCKET)
		{
			printf("accept() failed! code:%d\n", WSAGetLastError());
			closesocket(sListen);//关闭套接字
			WSACleanup();
			return;
		}
		//加入连接列表
		for (int i = 0; i < MAX_CLIENT; i++)
		{
			if (Client_list.valid[i] == 0)
			{
				
				Client_list.valid[i] = 1;
				strcpy_s(Client_list.addr[i], inet_ntoa(saClient.sin_addr));
				Client_list.port[i] = ntohs(saClient.sin_port);
				id = i;
				Client_list.s[i] = sServer[i];
					break;
			}
		}

		

		printf("Accepted client: %s:%d\n", inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));	
		thread[t++] = CreateThread(NULL, 0, &Process, &sServer[t], 0, NULL);
		//WaitForSingleObject(thread[t-1], INFINITE);

	}
	closesocket(sListen);//关闭套接字
	//closesocket(sServer);
	WSACleanup();
}