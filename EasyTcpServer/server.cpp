#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

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
#pragma comment(lib, "ws2_32.lib"
像这种写法在Windows里面是没问题的，但是如果需要跨平台，那么这种写法就会有问题，在非Windows系统下是不支持这种写法的
2：
另外一种是在项目的属性->链接器->输入->附加依赖项->在这里添加静态链接库

*/

using namespace std;

struct DataPackage {
	int age;
	char name[32];
};

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


	while (true)
	{

		// 创建接收缓冲区
		char _recvBuf[128] = {};
		// 5 接收客户端的请求报文
		int len = recv(_cSock, _recvBuf, 128, 0);
		if (len <= 0) {
			printf("客户端退出！\n");
			break;
		}
		else {
			printf("收到指令: %s\n", _recvBuf);
		}

		// 6 处理请求 向客户端发送一条数据send
		if (0 == strcmp(_recvBuf, "getName")) {
			const char msgbuff[] = "Hello, i'm Server!";
			send(_cSock, msgbuff, strlen(msgbuff) + 1, 0);
		}
		else if (0 == strcmp(_recvBuf, "getAge")) {
			const char msgbuff[] = "i'm eighteen!";
			send(_cSock, msgbuff, strlen(msgbuff) + 1, 0);
		}
		else if (0 == strcmp(_recvBuf, "getInfo")) {
			DataPackage dp = { 90, "invi" };
			send(_cSock, (const char*)(&dp), sizeof(struct DataPackage), 0);
		}
		else {
			const char msgbuff[] = "???";
			send(_cSock, msgbuff, strlen(msgbuff) + 1, 0);
		}
	}

	//6 关闭socket  closesocket
	closesocket(_sock);

	WSACleanup();

	printf("服务端程序退出\n");
	getchar();

	return 0;
}