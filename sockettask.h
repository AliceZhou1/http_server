#ifndef __SOCKETTASK_H__
#define __SOCKETTASK_H__

#include <WinSock2.h>
#include "task.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#define MAX_BUFFER_SIZE 4096

extern std::string srvIp, srvFile;
extern int srvPort;

struct client_s
{
	SOCKET socket;
	bool isExpire;//是否失效
	std::mutex mutex;//不需要上锁
};

class SocketTask:public Task
{
public:
	SocketTask(std::shared_ptr<client_s> _client);
	int run();
//private:
	std::shared_ptr<client_s> m_client;
	int writeBuf(char*& buf, std::string& filepath);
	std::string getFilePath(std::string _headrow);//通过首部行获取文件完整路径名
};
#endif //__SOCKETTASK_H__

