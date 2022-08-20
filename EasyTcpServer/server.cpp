#define WIN32_LEAN_AND_MEAN
#define  _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

#include <vector>

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
#pragma comment(lib, "ws2_32.lib")
������д����Windows������û����ģ����������Ҫ��ƽ̨����ô����д���ͻ������⣬�ڷ�Windowsϵͳ���ǲ�֧������д����
2��
����һ��������Ŀ������->������->����->����������->��������Ӿ�̬���ӿ�

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
	short dataLength;		// ���ݳ���
	short cmd;	// ����
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
	// ����һ�����ջ�����
	char recvbuff[4096] = {};

	// 5 ���տͻ��˵�������
	int len = recv(_cSock, recvbuff, sizeof(DataHeader), 0);
	if (len <= 0) {
		printf("�ͻ����˳���\n");
		return false;
	}
	// ������Ϣͷ
	DataHeader* dh = (DataHeader*)(recvbuff);

	switch (dh->cmd)
	{
	case CMD_LOGIN:
	{
		int log_len = recv(_cSock, recvbuff + sizeof(DataHeader), dh->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)(recvbuff);
		if (log_len > 0) {
			printf("�յ�ָ��: %d�� ���ݳ���: %d\n", login->cmd, login->dataLength);
			printf("��¼�û���%s, ���룺%s\n", login->userName, login->passWord);
		}

		// ��Ӧ����
		LoginRet loginret;
		loginret.res = 200;
		strcpy_s(loginret.msg, "��¼�ɹ�\n");
		send(_cSock, (const char*)(&loginret), sizeof(loginret), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		int log_len = recv(_cSock, recvbuff + sizeof(DataHeader), dh->dataLength - sizeof(DataHeader), 0);
		Logout* logout = (Logout*)(recvbuff);
		if (log_len > 0) {
			printf("�յ�ָ��: %d�� ���ݳ���: %d\n", logout->cmd, logout->dataLength);
			printf("�û� ��%s�������˳���¼\n", logout->userName);
		}

		// ��Ӧ����
		LogoutRet logoutret;
		logoutret.res = 200;
		strcpy_s(logoutret.msg, "�˳���¼�ɹ�\n");
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

	while (true)
	{
		// select
		// ������ socket
		/*select(
			_In_ int nfds,			��һ�����������������+1�������Windows��û��̫�����壬��Unix/Linuxϵͳ������ã�
			_Inout_opt_ fd_set FAR * readfds,	�ɶ����� �ɶ��Լ��(�����ݿɶ��룬���ӹرգ����裬��ֹ)��Ϊ���򲻼��ɶ���
			_Inout_opt_ fd_set FAR * writefds,	��д���� ��д�Լ��(�����ݿɷ���)��Ϊ���򲻼���д��
			_Inout_opt_ fd_set FAR * exceptfds,		�쳣���� �������ݼ��(��������)��Ϊ���򲻼��
			_In_opt_ const struct timeval FAR * timeout
			);

			select����������⣺����

			���� fd_set������һ�����ڼ��ɶ��ԣ�readfds����һ�����ڼ���д�ԣ�writefds������һ�������������ݣ� excepfds����

			�Ӹ�����˵��fdset�������ʹ�����һϵ���ض��׽��ֵļ��ϡ����У�

			readfds���ϰ������������κ�һ���������׽��֣�

			�� �����ݿ��Զ��롣
			�� �����Ѿ��رա��������ֹ��
			�� �����ѵ�����listen������һ���������ڽ�������ôaccept�������û�ɹ���

			writefds���ϰ������������κ�һ���������׽��֣�

			�� �����ݿ��Է�����
			�� ���������˶�һ�����������ӵ��õĴ������Ӿͻ�ɹ���
			���exceptfds���ϰ������������κ�һ���������׽��֣�
			�� ����������˶�һ�����������ӵ��õĴ������ӳ��Ծͻ�ʧ�ܡ�
			�� �д��⣨out-of-band��OOB�����ݿɹ���ȡ��

			���һ������timeout:

			��Ӧ����һ��ָ�룬��ָ��һ��timeval�ṹ�����ھ���select���ȴ� I / O������ɶ�õ�ʱ�䡣

			�� timeout��һ����ָ�룬��ôselect���û������ڵء���������ͣ����ȥ��ֱ��������һ������������ָ���������������

			��timeval�ṹ�Ķ������£�

			struct timeval {
			long tv_sec;
			long tv_usec;
			} ;

			������ʱֵ����Ϊ��0,0��������select���������أ�����Ӧ�ó���� select�������С���ѯ�������ڶ����ܷ���Ŀ��ǣ�Ӧ�������������á�

			select�ɹ���ɺ󣬻��� fd_set�ṹ�У����ظպ���δ��ɵ�I/O�����������׽��־����������

			������timeval�趨��ʱ�䣬��᷵��0��
		*/

		/*
		selectģ�͵Ĺ�������:
		(1)����һ������fd_set����fd_zero���ʼ��Ϊ��

		(2)��FD_SET�꣬���׽��־�����뵽fd_set����

		(3)����select���������ÿ���׽��ֵĿɶ���д�ԣ�select��ɺ󣬻᷵��������fd_set�����������ݵ����socket��socket�������������ÿ�����Ͻ��и��£���û�����ݵ����socket��ԭ�����лᱻ�óɿա�
		(4)����select�ķ���ֵ�Լ�FD_ISSET�꣬��FD_SET���Ͻ��м��
		(5)֪����ÿ�������С���������I/O�����󣬶���ӦI/O�������д������ز���1������select

		select�������غ󣬻��޸�FD_SET�Ľṹ��ɾ�������ڴ���IO�������׽��֣���Ҳ����Ϊʲô����֮��Ҫ��FD_ISSET�ж��Ƿ��ڼ����е�ԭ��
		*/

		// ����fdset����
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		// ���fdset��������
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// ���׽��־�����뵽fdset����
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (auto iter = g_clients.rbegin(); iter != g_clients.rend(); ++iter) {
			FD_SET(*iter, &fdRead);
		}

		// nfds ��һ������ֵ��ָ����fd_set���������е���������socket���ķ�Χ������������������������Ǹ�������+1����
		// ��Windows�����������select�ڲ��Ѿ������Զ����㣬ʵ����û���õ����򴫵ݵ����ֵ
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
		if (ret < 0) {
			printf("select ���󣬳����˳���\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead)) {
			// �пɶ� ��˵���пͻ������ӽ�����
			FD_CLR(_sock, &fdRead);

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

			g_clients.push_back(_cSock);
		}

		//if (dh->dataLength <= len) {
		//}

		// 6 �������� ��ͻ��˷���һ������send
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

	//6 �ر�socket  closesocket
	for (auto iter = g_clients.begin(); iter != g_clients.end(); ++iter) {
		closesocket(*iter);
	}

	closesocket(_sock);

	WSACleanup();

	printf("����˳����˳�\n");
	getchar();

	return 0;
}