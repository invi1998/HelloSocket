#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

// #pragma comment(lib, "ws2_32.lib")

/*
��Windows����У�ͬʱ������������ͷ�ļ��������ʱ��ᱨ�ܶ�ĺ궨���ظ�����Ĵ���
#include <Windows.h>
#include <WinSock2.h>
��Ϊ�����ڵ�Windows.h�У���д�˵�һ����socket��̵�API��#include <Windows.h>��ʱ����ЩAPI���Ѿ������õ��ˣ���ô������������Ҫʹ�����µ�#include <WinSock2.h> 2.x��Socket API��
�����ͻᵼ���ض����������֣�
�����ʽ�����֣�
1��
#include <WinSock2.h>
#include <Windows.h>
��socketͷ�ļ�����Windowsͷ�ļ�֮ǰ����������д���ڹ������Ƚϴ��ʱ�����Ǿ�û�а취��֤socketͷ�ļ�һ����Windowsͷ�ļ�֮ǰ���á�
2:
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
�ڶ����������������ļ���ͷ����һ���궨�� WIN32_LEAN_AND_MEAN ��΢��ٷ��ṩ�ķ�����
����궨������þ����ó�������������һЩ���׳�ͻ������ʵ�ֵĿ⣬�������������������
- WIN32_LEAN_AND_MEAN�����ų�һЩ�����õ�API���ڼ�Сͷ�ļ���С������Cryptography, DDE, RPC, Shell, and Windows Sockets֮���ͷ�ļ���
- Windows.hͷ�ļ��а�����Winsock.h��Winsock2.h��ͻ������ᱨ������ͨ������WIN32_LEAN_AND_MEAN�����
- ������ʹ��NOXXX�궨���ų�ָ����API������NOCOMM�ų��˴���ͨ��API��

*/

/*
WSAStartup(ver, &dat); ���������������ͨ�����룬���������ӵ�ʱ��ͻᱨ������Ϊ�������ʵ�����ǵ�����һ��Windows�Ķ�̬�⣬���Ի���Ҫ������ļ�

���뾲̬���ӿ������ַ�����
1��
ֱ���ڴ����н������
#pragma comment(lib, "ws2_32.lib"
������д����Windows������û����ģ����������Ҫ��ƽ̨����ô����д���ͻ������⣬�ڷ�Windowsϵͳ���ǲ�֧������д����
2��
����һ��������Ŀ������->������->����->����������->��������Ӿ�̬���ӿ�

*/

using namespace std;

struct DataPackage {
	int age;
	char name[32];
};

int main(int agrs, const char* argv[]) {
	// ����Windows socket 2.x����

	// winows����socket�ĺ��� WSAStartup ��Ҫ����һ���汾��
	// �����汾��
	WORD ver = MAKEWORD(2, 2);

	WSADATA dat;
	// ���socket�����ĺ�������
	WSAStartup(ver, &dat);

	// 1 ����һ��socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		printf("socket����ʧ��\n");
	}
	else {
		printf("socket�����ɹ�\n");
	}

	// 2 �󶨽��ܿͻ������ӵĶ˿� bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);	// host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr("127.0.0.0")
	if (bind(_sock, (sockaddr*)(&_sin), sizeof(sockaddr_in)) == SOCKET_ERROR) {
		printf("IP��ַ�Ͷ˿ڰ�ʧ��\n");
	}
	else {
		printf("IP��ַ�Ͷ˿ڰ󶨳ɹ�\n");
	}

	//3 ��������˿� listen
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("�˿ڼ���ʧ��\n");
	}
	else {
		printf("�˿ڼ��� ing...\n");
	}

	//4 �ȴ����ܿͻ������� accept
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	_cSock = accept(_sock, (sockaddr*)(&clientAddr), &nAddrLen);
	if (INVALID_SOCKET == _cSock) {
		printf("���ӿͻ���ʧ��\n");
	}
	else {
		printf("�ͻ������ӳɹ�, ip = %s�� socket = %d\n", inet_ntoa(clientAddr.sin_addr), _cSock);

	}


	while (true)
	{

		// �������ջ�����
		char _recvBuf[128] = {};
		// 5 ���տͻ��˵�������
		int len = recv(_cSock, _recvBuf, 128, 0);
		if (len <= 0) {
			printf("�ͻ����˳���\n");
			break;
		}
		else {
			printf("�յ�ָ��: %s\n", _recvBuf);
		}

		// 6 �������� ��ͻ��˷���һ������send
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

	//6 �ر�socket  closesocket
	closesocket(_sock);

	WSACleanup();

	printf("����˳����˳�\n");
	getchar();

	return 0;
}