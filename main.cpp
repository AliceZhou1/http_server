#include <WinSock2.h>
#include <iostream>
#include "sockettask.h"
#include "task.h"
#include "threadpool.h"

#define MAX_CLIENTS 10
std::string srvFile = ".\\res";
#pragma comment(lib, "ws2_32.lib")
int total = 0;
int main()
{
	WSADATA wsaData;
	int nRc = WSAStartup(0x0202, &wsaData);

	if (nRc)
	{
		printf("Winsock  startup failed with error!\n");
	}

	if (wsaData.wVersion != 0x0202)
	{
		printf("Winsock version is not correct!\n");
	}

	printf("Winsock  startup Ok!\n");
	//监听socket
	SOCKET srvSocket;

	//服务器地址和客户端地址
	sockaddr_in srvAddr;

	//创建监听socket
	srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");

	//设置服务器的端口和地址
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(80);
	srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//binding
	int rtn = bind(srvSocket, (LPSOCKADDR)&srvAddr, sizeof(srvAddr));
	if (rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");

	//监听
	rtn = listen(srvSocket, SOMAXCONN);
	if (rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");

	//u_long blockMode = 1;//将srvSock设为非阻塞模式以监听客户连接请求

	//if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR))
	//{ //FIONBIO：允许或禁止套接口s的非阻塞模式。
	//	std::cout << "ioctlsocket() failed with error!\n";
	//	return 0;
	//}
	//std::cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	//启动线程池
	ThreadPool pool;
	pool.init(16);
	pool.start();
	//存储当前可用client
	std::list<std::shared_ptr<client_s>> clients;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(srvSocket, &rfds);
	while (true)
	{
		//删除容器内已失效socket
		for (auto it = clients.begin(); it != clients.end();)
		{
			if ((*it)->isExpire)
			{
				closesocket((*it)->socket);
				FD_CLR((*it)->socket, &rfds);
				std::cerr << "remove : " << (*it)->socket << std::endl;
				it = clients.erase(it);
			}
			else
				it++;
		}
		fd_set tmp_fds = rfds;
		rtn = select(0, &tmp_fds, NULL, NULL, NULL);
		if (rtn == SOCKET_ERROR)
		{
			std::cerr << "select failed: " << WSAGetLastError() << std::endl;
			break;
		}
		for (auto it : clients)
		{
			if (FD_ISSET(it->socket, &tmp_fds))
			{
				auto task = std::make_shared<SocketTask>(it);
				//std::cerr << total++ << "-----------------------" << task->m_client->socket << std::endl;
				pool.addTask(task);
			}
		}
		if (FD_ISSET(srvSocket, &tmp_fds))
		{
			SOCKET client_socket = accept(srvSocket, NULL, NULL);
			if (client_socket == INVALID_SOCKET)
			{
				std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
				break;
			}
			std::cout << "listen one request:" << client_socket << std::endl;
			auto client = std::make_shared<client_s>();
			client->socket = client_socket;
			client->isExpire = false;
			clients.emplace_back(client);
			FD_SET(client_socket, &rfds);
		}
	}
	// Cleanup
	clients.clear();
	closesocket(srvSocket);
	pool.stop();
	WSACleanup();
	return 0;
}