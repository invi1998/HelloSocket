#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

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

enum CMD {
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_LOGIN_RET,
	CMD_LOGOUT_RET
};

struct DataHeader {
	short dataLength;		// ���ݳ���
	short cmd;	// ����
};

struct Login {
	char userName[32];
	char passWord[32];
};

struct LoginRet {
	int res;
	char msg[32];
};

struct Logout {
	char userName[32];
};

struct LogoutRet {
	int res;
	char msg[32];
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
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock) {
		printf("socket����ʧ��\n");
	}
	else {
		printf("socket�����ɹ�\n");
	}

	// 2 ���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);	// host to net unsigned short
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)(&_sin), sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("����������ʧ��\n");
	}
	else {
		printf("���������ӳɹ�\n");
	}

	// ��������ͨ��
	char cmdBuf[128] = {};
	while (true) {
		printf("\n������ָ�\n");
		memset(cmdBuf, 0, sizeof(cmdBuf));
		scanf_s("%s", cmdBuf, 128);
		if (0 == strcmp(cmdBuf, "exit")) {
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login = {};
			char buf[32] = {};
			printf("\n�������û�����\n");
			scanf_s("%s", buf, 32);
			strcpy_s(login.userName, 32, buf);
			memset(buf, 0, sizeof(buf));
			printf("\n���������룺\n");
			scanf_s("%s", buf, 32);
			strcpy_s(login.passWord, 32, buf);
			
			// ��Ϣͷ
			DataHeader dh_login = { sizeof(login),  CMD_LOGIN };

			send(_sock, (const char*)(&dh_login), sizeof(dh_login), 0);

			send(_sock, (const char*)(&login), sizeof(login), 0);

			// ���շ�������������
			DataHeader dh_ret = {};
			LoginRet loginret = {};
			int dh_len = recv(_sock, (char*)(&dh_ret), sizeof(DataHeader), 0);
			if (dh_len > 0) {
				int ret_len = recv(_sock, (char*)(&loginret), dh_ret.dataLength, 0);
				if (ret_len > 0) {
					printf("��¼״̬�룺%d�� ��¼�����%s\n", loginret.res, loginret.msg);
				}
			}
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout = {};
			char buf[32] = {};
			printf("\n�������˳��û���\n");
			scanf_s("%s", buf, 32);
			strcpy_s(logout.userName, 32, buf);

			// ��Ϣͷ
			DataHeader dh_logout = { sizeof(logout),  CMD_LOGOUT };

			send(_sock, (const char*)(&dh_logout), sizeof(dh_logout), 0);

			send(_sock, (const char*)(&logout), sizeof(logout), 0);

			// ���շ�������������
			DataHeader dh_ret = {};
			LoginRet logoutret = {};
			int dh_len = recv(_sock, (char*)(&dh_ret), sizeof(DataHeader), 0);
			if (dh_len > 0) {
				int ret_len = recv(_sock, (char*)(&logoutret), dh_ret.dataLength, 0);
				if (ret_len > 0) {
					printf("�˳���¼״̬�룺%d�� �˳������%s\n", logoutret.res, logoutret.msg);
				}
			}
		}
		else {
			printf("���֧��\n");
		}
	}


	//4 �ر�socket  closesocket
	closesocket(_sock);

	WSACleanup();

	printf("�ͻ����˳�\n");

	getchar();

	return 0;
}