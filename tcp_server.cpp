#include "function.h" 

#define PORT 4578               //����� ��Ʈ�� �����ϰ� ���, 4�ڸ� ��Ʈ�� ������ ���ڸ� �Ҵ�
#define PACKET_SIZE 1024         //��Ŷ����� ����

#pragma comment(lib, "ws2_32")      //������ ������ ������ϵ��� ������ �������� ��ũ

//��� on/off
boolean Communication = true;

//��ź���
WSADATA wsaData;
SOCKET hListen;
SOCKADDR_IN tListenAddr = {};
SOCKADDR_IN tClntAddr = {};
int iClntSize;
SOCKET hClient;	

//��� ������
void tcp_server(const char menu[], float msg) {
	if (Communication != false) {

		char temp[16] = { 0 };				//���� 2 + numb + '\0' = 13 ~ 16 
		char numb[13] = { 0 };				//���� 2x4 + 1 ~3x4 + 1 = 9 ~ 13 �ε� 10���� �Ǿ��ϴ� ������..?

		sprintf_s(numb, sizeof(numb), "%.2f", msg);
		//cout << "numb : " << numb << endl;

		strcpy_s(temp, sizeof(temp), menu);


		strcat_s(temp, " ");
		strcat_s(temp, numb);

		//cout << "temp : " << temp << endl;
		//cout << "Server Msg : menu - " << menu << " / float - " << msg << endl;
		send(hClient, temp, strlen(temp), 0);

	}
}


//��� ���� / ����
void tcp_server_onoff(bool check) {
	if (Communication != false) {
		if (check != false) {
			WSAStartup(MAKEWORD(2, 2), &wsaData);
			hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			tListenAddr.sin_family = AF_INET;
			tListenAddr.sin_port = htons(PORT);
			tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
			listen(hListen, SOMAXCONN);
			iClntSize = sizeof(tClntAddr);
			hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize);

			cout << "���� ����!" << endl;
		}
		else {
			closesocket(hClient);
			closesocket(hListen);
		}
	}
}