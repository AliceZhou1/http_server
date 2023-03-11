#include "sockettask.h"


SocketTask::SocketTask(std::shared_ptr<client_s> _client)
	:m_client(_client)
{
}

int SocketTask::run()
{
	//读入缓冲区
	std::cerr <<"start :" << std::this_thread::get_id() << std::endl;
	char buf[MAX_BUFFER_SIZE];
	memset(buf, '\0', MAX_BUFFER_SIZE);
	recv(m_client->socket, buf, MAX_BUFFER_SIZE, 0);
	//取第一行
	std::string rcvMessage(buf);
	//std::cerr << rcvMessage << std::endl;
	std::string firstRow = rcvMessage.substr(0, rcvMessage.find("\r\n"));
	//std::cerr << "Http request command line:\n" << firstRow << std::endl;
	//读入长度为0需要关闭该socket
	if (rcvMessage.length() != 0)
	{
		//读入二进制文件
		std::string absolutePath = getFilePath(firstRow);
		char* buf = nullptr;
		int bufLen = writeBuf(buf, absolutePath);

		//封装http首部
		std::string contentType, contentLength;
		if (absolutePath.find(".html") != absolutePath.npos)
			contentType = "text/html";
		else if (absolutePath.find(".jpg") != absolutePath.npos)
			contentType = "image/jpeg";
		else
			contentType = "text/plain";
		contentLength = std::to_string(bufLen);
		std::string resFirstLine = "HTTP/1.1 200 OK\r\n";
		std::string resHeader = "Content-Type:" + contentType + "\r\n";
		std::string resLength = "Content-Length:" + contentLength + "\r\n\r\n";
		std::string resMessage = resFirstLine + resHeader + resLength;

		send(m_client->socket, resMessage.c_str(), resMessage.length(), 0);//发送首部
		send(m_client->socket, buf, bufLen, 0);
		if (absolutePath != srvFile + "\\404.html")
			std::cerr << "\n200 OK\n" << std::endl;
		else
			std::cerr << "\n404 not found\n" << std::endl;
		if (buf)
			delete[] buf;
	}
	else
	{
		closesocket(m_client->socket);
		m_client->isExpire = true;
		std::cerr << "close :" << std::this_thread::get_id() << std::endl;

	}
	std::cerr << "end :" << std::this_thread::get_id() << std::endl;

	return 0;
}

int SocketTask::writeBuf(char*& resBuf, std::string filepath)
{
	std::ifstream f(filepath, std::ios::binary);
	//如果没找到文件，返回404.html
	if (!f.is_open())
	{
		filepath = srvFile + "\\404.html";
		return writeBuf(resBuf, filepath);
	}

	f.seekg(0, f.end);
	int bufLen = f.tellg();
	f.seekg(0, f.beg);
	resBuf = new char[bufLen];
	memset(resBuf, '\0', bufLen);
	f.read(resBuf, bufLen);
	f.close();
	return bufLen;

}

std::string SocketTask::getFilePath(std::string _headRow)
{
	//获取查找文件名
	int pos = _headRow.find('/');
	int tail = _headRow.find(" HTTP");
	int len = tail - pos;
	std::string relativePath = _headRow.substr(pos, len);
	std::string absolutePath = srvFile + relativePath.replace(0, 1, "\\");
	return absolutePath;
}

