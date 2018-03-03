
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <winsock2.h>
#include<time.h>
#include<conio.h>
#define SERVER_PORT	6666 //侦听端口
#define MAX_LENTH 200

//客户端向服务器传送的结构：

typedef struct
{
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;

}Time;
struct request
{
	int read=1; //0表示未读
	int type;
	int No;
	Time t;
	char content[MAX_LENTH];
};
struct request send_req;
struct request rec_req;

int connected = 0;
CRITICAL_SECTION cs;


DWORD WINAPI Receive(LPVOID lpParam);

int start()
{
	int commond;
	char ch;
	printf("**********************\nInput your commond:\n1:connect\n2:break up\n3:get currunt time\n4:get server name\n5:get client list\n6:send message\n7:quit\n0:flash the message\n**********************\n");
	scanf_s("%d", &commond);
	return commond;
}
HANDLE thread;
void connect(SOCKET sClient)
{
	int ret;
	struct sockaddr_in saServer;//地址信息
	char *addr = (char*)malloc(sizeof(char) * 20);
	printf(">>Please input the server address:\n");
	scanf_s("%s", addr,20);
	//构建服务器地址信息：
	saServer.sin_family = AF_INET;//地址家族
	saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = inet_addr(addr);


	//连接服务器：
	ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));

	if (ret == SOCKET_ERROR)
	{
		printf(">>connect() failed!\n");
		closesocket(sClient);//关闭套接字
		WSACleanup();
		return;
	}
	connected = 1;
	printf(">>connect() succeed!\n");
    thread = CreateThread(NULL, 0, &Receive, &sClient, 0, NULL); //receive
	//WaitForSingleObject(thread, INFINITE);

}
void close(SOCKET sClient)
{
	struct request quit;
	quit.type = 2;
	send(sClient,(char*)&quit,5, 0);
	closesocket(sClient);//关闭套接字
	connected = 0;
}
void send(int commond,SOCKET sClient)
{
	
	if (send_req.type == 6)
	{
		printf(">>Please input the number of client :\n");
		scanf_s("%d",&send_req.No); 
		printf(">>Please input the message:\n");
		//scanf_s("%s", send_req.content,MAX_LENTH);
		getchar();
		gets_s(send_req.content);
		
	}
	int ret;

	ret=send(sClient, (char*)(&send_req), sizeof(struct request), 0);
	if (ret == SOCKET_ERROR)
	{
		printf(">>send() failed!\n");
	}
	else if(send_req.type!=0)
		printf(">>request has been sent!\n");

}
void quit(SOCKET sClient)
{
	if (connected == 0)
		exit(0);
	else
	{
		close(sClient);
		exit(0);
	}
}
void process(int commond, SOCKET sClient)
{
	if (commond < 0 || commond >= 7)
	{
		printf(">>Please input a valid commond ( 0 - 7 )\n");
		return;
	}
	send_req.type = commond;//3:时间 4:名字 5:列表 6:消息
	switch (commond)
	{
	case 1:connect(sClient); break;
	case 2:if (connected) close(sClient); else printf("Please build a connection first!\n"); break;//关闭套接字
	case 7:quit(sClient); break;
	default:if (connected) send(commond, sClient); else printf("Please build a connection first!\n");
	}
}

void main(int argc, char *argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	SOCKET sClient, sServer; //连接套接字
	BOOL fSuccess = TRUE;
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
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return;
	}
	int commond;
	while(true){
		commond = start();
		process(commond, sClient);
	}
	closesocket(sClient);//关闭套接字
	WSACleanup();

}
#include<Windows.h>
DWORD WINAPI Receive(LPVOID lpParam)
{
	SOCKET* sClient = (SOCKET*)lpParam;

	int ret, nLeft;
	nLeft = sizeof(rec_req);
	while (true)
	{
	
		ret = recv(*sClient, (char*)&rec_req, nLeft, 0);
	
		if (rec_req.read == 0) {
			if (rec_req.type == 3) {
				printf("Currunt Datetime: %d-%d-%d %d:%d:%d\n",
					rec_req.t.year, rec_req.t.month, rec_req.t.day, rec_req.t.hour, rec_req.t.min, rec_req.t.sec);
			}
			else if (rec_req.type == 4)
			{
				printf("Server Name:%s\n", rec_req.content);
			}
			else if (rec_req.type == 5)
			{
				printf("%s", rec_req.content);
				
			}
			else if (rec_req.type == 6)
			{
			
				printf("Message From Client No.%d: %s\n",rec_req.No, rec_req.content);
				
				
			}
			rec_req.read = 1;
		}
	
	}
	closesocket(*sClient);
	return 0;

}