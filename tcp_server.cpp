#include "function.h" 

#define PORT 4578               //����� ��Ʈ�� �����ϰ� ���, 4�ڸ� ��Ʈ�� ������ ���ڸ� �Ҵ�
#define PACKET_SIZE 1024         //��Ŷ����� ����

#pragma comment(lib, "ws2_32")      //������ ������ ������ϵ��� ������ �������� ��ũ

//��� on/off
boolean Communication = false;

//��ź���
WSADATA wsaData;
SOCKET hListen;
SOCKADDR_IN tListenAddr = {};
SOCKADDR_IN tClntAddr = {};
int iClntSize;
SOCKET hClient;

//��� ������
void tcp_server(float msg) {
	if (Communication) {
		char temp[4];
		memcpy(temp, &msg, sizeof(float));
		send(hClient, temp, strlen(temp), 0);
		cout << "Server Msg : " << msg << endl;
	}
}

//��� ���� / ����
void tcp_server_onoff(double check) {
	if (Communication) {
		if (check) {
			WSAStartup(MAKEWORD(2, 2), &wsaData);
			hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			tListenAddr.sin_family = AF_INET;
			tListenAddr.sin_port = htons(PORT);
			tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
			listen(hListen, SOMAXCONN);
			iClntSize = sizeof(tClntAddr);
			hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize);
		}
		else {
			closesocket(hClient);
			closesocket(hListen);
		}
	}
}
