#include "function.h" 

#define PORT 4578               //예약된 포트를 제외하고 사용, 4자리 포트중 임의의 숫자를 할당
#define PACKET_SIZE 1024         //패킷사이즈를 정의

#pragma comment(lib, "ws2_32")      //위에서 선언한 헤더파일들을 가져다 쓰기위한 링크

//통신 on/off
boolean Communication = true;

//통신변수
WSADATA wsaData;
SOCKET hListen;
SOCKADDR_IN tListenAddr = {};
SOCKADDR_IN tClntAddr = {};
int iClntSize;
SOCKET hClient;	

//통신 보내기
void tcp_server(const char menu[], float msg) {
	if (Communication != false) {

		char temp[16] = { 0 };				//문자 2 + numb + '\0' = 13 ~ 16 
		char numb[13] = { 0 };				//숫자 2x4 + 1 ~3x4 + 1 = 9 ~ 13 인데 10부터 되야하는 이유는..?

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


//통신 시작 / 종료
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

			cout << "연결 성공!" << endl;
		}
		else {
			closesocket(hClient);
			closesocket(hListen);
		}
	}
}