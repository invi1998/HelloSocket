#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

#include <vector>

// #pragma comment(lib, "ws2_32.lib")

/*
在Windows编程中，同时引入上面两个头文件，编译的时候会报很多的宏定义重复定义的错误，
#include <Windows.h>
#include <WinSock2.h>
因为在早期的Windows.h中，它写了第一批的socket编程的API，#include <Windows.h>的时候，这些API就已经被引用到了，那么在这里我们需要使用最新的#include <WinSock2.h> 2.x的Socket API，
这样就会导致重定义的问题出现，
解决方式有两种：
1：
#include <WinSock2.h>
#include <Windows.h>
将socket头文件放在Windows头文件之前，但是这种写法在工程量比较大的时候，我们就没有办法保证socket头文件一定在Windows头文件之前引用。
2:
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
第二个方法就是我们文件开头加入一个宏定义 WIN32_LEAN_AND_MEAN （微软官方提供的方法）
这个宏定义的作用就是让程序尽力避免引入一些容易冲突的早期实现的库，减少其他依赖库的引用
- WIN32_LEAN_AND_MEAN用来排除一些不常用的API用于减小头文件大小，例如Cryptography, DDE, RPC, Shell, and Windows Sockets之类的头文件。
- Windows.h头文件中包含的Winsock.h与Winsock2.h冲突，编译会报错，可以通过定义WIN32_LEAN_AND_MEAN解决。
- 还可以使用NOXXX宏定义排除指定的API，例如NOCOMM排除了串行通信API。

*/

/*
WSAStartup(ver, &dat); 这个函数在能正常通过编译，但是在链接的时候就会报错，是因为这个函数实际上是调用了一个Windows的动态库，所以还需要加入库文件

加入静态链接库有两种方法：
1：
直接在代码中进行添加
#pragma comment(lib, "ws2_32.lib")
像这种写法在Windows里面是没问题的，但是如果需要跨平台，那么这种写法就会有问题，在非Windows系统下是不支持这种写法的
2：
另外一种是在项目的属性->链接器->输入->附加依赖项->在这里添加静态链接库

*/

//using namespace std;

enum CMD {
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_LOGIN_RET,
	CMD_LOGOUT_RET,
	CMD_ERROR
};

struct DataHeader {
	short dataLength;		// 数据长度
	short cmd;	// 命令
};

struct Login : public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};

struct LoginRet : public DataHeader {
	LoginRet() {
		dataLength = sizeof(LoginRet);
		cmd = CMD_LOGIN_RET;
	}
	int res;
	char msg[32];
};

struct Logout : public DataHeader {
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutRet : public DataHeader {
	LogoutRet() {
		dataLength = sizeof(LogoutRet);
		cmd = CMD_LOGOUT_RET;
	}
	int res;
	char msg[32];
};

std::vector<SOCKET> g_clients;

bool processor(SOCKET _cSock) {
	// 定义一个接收缓冲区
	char recvbuff[4096] = {};

	// 5 接收客户端的请求报文
	int len = recv(_cSock, recvbuff, sizeof(DataHeader), 0);
	if (len <= 0) {
		printf("客户端退出！\n");
		return false;
	}
	// 定义消息头
	DataHeader* dh = (DataHeader*)(recvbuff);

	switch (dh->cmd)
	{
	case CMD_LOGIN:
	{
		int log_len = recv(_cSock, recvbuff + sizeof(DataHeader), dh->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)(recvbuff);
		if (log_len > 0) {
			printf("收到指令: %d， 数据长度: %d\n", login->cmd, login->dataLength);
			printf("登录用户：%s, 密码：%s\n", login->userName, login->passWord);
		}

		// 回应报文
		LoginRet loginret;
		loginret.res = 200;
		strcpy_s(loginret.msg, "登录成功\n");
		send(_cSock, (const char*)(&loginret), sizeof(loginret), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		int log_len = recv(_cSock, recvbuff + sizeof(DataHeader), dh->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)(recvbuff);
		if (log_len > 0) {
			printf("收到指令: %d， 数据长度: %d\n", logout->cmd, logout->dataLength);
			printf("用户 【%s】请求退出登录\n", logout->userName);
		}

		// 回应报文
		LogoutRet logoutret;
		logoutret.res = 200;
		strcpy_s(logoutret.msg, "退出登录成功\n");
		send(_cSock, (const char*)(&logoutret), sizeof(logoutret), 0);
	}
	break;
	default:
		DataHeader dh_err = { 0, CMD_ERROR };
		send(_cSock, (const char*)(&dh_err), sizeof(dh_err), 0);
		break;
	}

	return true;
}

int main(int agrs, const char* argv[]) {
	// 启动Windows socket 2.x环境

	// winows启动socket的函数 WSAStartup 需要传入一个版本号
	// 创建版本号
	WORD ver = MAKEWORD(2, 2);

	WSADATA dat;
	// 完成socket启动的函数调用
	WSAStartup(ver, &dat);

	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		printf("socket创建失败\n");
	}
	else {
		printf("socket创建成功\n");
	}

	// 2 绑定接受客户端连接的端口 bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);	// host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr("127.0.0.0")
	if (bind(_sock, (sockaddr*)(&_sin), sizeof(sockaddr_in)) == SOCKET_ERROR) {
		printf("IP地址和端口绑定失败\n");
	}
	else {
		printf("IP地址和端口绑定成功\n");
	}

	//3 监听网络端口 listen
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("端口监听失败\n");
	}
	else {
		printf("端口监听 ing...\n");
	}

	while (true)
	{
		// select
		// 伯克利 socket
		/*select(
			_In_ int nfds,			第一个参数：最大描述符+1（这个在Windows里没有太大意义，在Unix/Linux系统里才有用）
			_Inout_opt_ fd_set FAR * readfds,	可读集合 可读性检查(有数据可读入，连接关闭，重设，终止)，为空则不检查可读性
			_Inout_opt_ fd_set FAR * writefds,	可写集合 可写性检查(有数据可发出)，为空则不检查可写性
			_Inout_opt_ fd_set FAR * exceptfds,		异常集合 带外数据检查(带外数据)，为空则不检查
			_In_opt_ const struct timeval FAR * timeout
			);

			select函数参数详解：　　

			三个 fd_set参数：一个用于检查可读性（readfds），一个用于检查可写性（writefds），另一个用于例外数据（ excepfds）。

			从根本上说，fdset数据类型代表着一系列特定套接字的集合。其中，

			readfds集合包括符合下述任何一个条件的套接字：

			■ 有数据可以读入。
			■ 连接已经关闭、重设或中止。
			■ 假如已调用了listen，而且一个连接正在建立，那么accept函数调用会成功。

			writefds集合包括符合下述任何一个条件的套接字：

			■ 有数据可以发出。
			■ 如果已完成了对一个非锁定连接调用的处理，连接就会成功。
			最后，exceptfds集合包括符合下述任何一个条件的套接字：
			■ 假如已完成了对一个非锁定连接调用的处理，连接尝试就会失败。
			■ 有带外（out-of-band，OOB）数据可供读取。

			最后一个参数timeout:

			对应的是一个指针，它指向一个timeval结构，用于决定select最多等待 I / O操作完成多久的时间。

			如 timeout是一个空指针，那么select调用会无限期地“锁定”或停顿下去，直到至少有一个描述符符合指定的条件后结束。

			对timeval结构的定义如下：

			struct timeval {
			long tv_sec;
			long tv_usec;
			} ;

			若将超时值设置为（0,0），表明select会立即返回，允许应用程序对 select操作进行“轮询”。出于对性能方面的考虑，应避免这样的设置。

			select成功完成后，会在 fd_set结构中，返回刚好有未完成的I/O操作的所有套接字句柄的总量。

			若超过timeval设定的时间，便会返回0。
		*/

		/*
		select模型的工作步骤:
		(1)定义一个集合fd_set并用fd_zero宏初始化为空

		(2)用FD_SET宏，把套接字句柄加入到fd_set集合

		(3)调用select函数，检查每个套接字的可读可写性，select完成后，会返回所有在fd_set集合中有数据到达的socket的socket句柄总数，并对每个集合进行更新，即没有数据到达的socket在原集合中会被置成空。
		(4)根据select的返回值以及FD_ISSET宏，对FD_SET集合进行检查
		(5)知道了每个集合中“待决”的I/O操作后，对相应I/O操作进行处理，返回步骤1，继续select

		select函数返回后，会修改FD_SET的结构，删除不存在待决IO操作的套接字，这也就是为什么我们之后要用FD_ISSET判断是否还在集合中的原因。
		*/

		// 创建fdset集合
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		// 清空fdset集合数据
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// 把套接字句柄加入到fdset集合
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (auto iter = g_clients.rbegin(); iter != g_clients.rend(); ++iter) {
			FD_SET(*iter, &fdRead);
		}

		// nfds 是一个整数值，指的是fd_set集合中所有的描述符（socket）的范围，而不是数量，所以用最大那个描述符+1即可
		// 在Windows下这个参数在select内部已经做了自动计算，实际上没有用到程序传递的这个值
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
		if (ret < 0) {
			printf("select 错误，程序退出！\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead)) {
			// 有可读 （说明有客户端链接进来）
			FD_CLR(_sock, &fdRead);

			//4 等待接受客户端连接 accept
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
			_cSock = accept(_sock, (sockaddr*)(&clientAddr), &nAddrLen);
			if (INVALID_SOCKET == _cSock) {
				printf("连接客户端失败\n");
			}
			else {
				printf("客户端链接成功, ip = %s， socket = %d\n", inet_ntoa(clientAddr.sin_addr), _cSock);
			}

			g_clients.push_back(_cSock);
		}

		//if (dh->dataLength <= len) {
		//}

		// 6 处理请求 向客户端发送一条数据send
		for (auto iter = g_clients.begin(); iter != g_clients.end();) {
			if (processor(*iter) == false) {
				closesocket(*iter);
				iter = g_clients.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}

	//6 关闭socket  closesocket
	for (auto iter = g_clients.begin(); iter != g_clients.end(); ++iter) {
		closesocket(*iter);
	}

	closesocket(_sock);

	WSACleanup();

	printf("服务端程序退出\n");
	getchar();

	return 0;
}