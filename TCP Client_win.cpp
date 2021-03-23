#include <stdio.h>
#include <iostream>  
#include<winsock.h>

#pragma comment(lib, "ws2_32")      //위에서 선언한 헤더파일들을 가져다 쓰기위한 링크

#define PORT 4578
#define PACKET_SIZE 1024
#define SERVER_IP "192.168.0.2"   //서버 IP를 지정 "211.32.63.121"(기숙사)

using  std::cout;
using  std::endl;

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET hSocket;
    hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN tAddr = {};
    tAddr.sin_family = AF_INET;
    tAddr.sin_port = htons(PORT);
    tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr));      // 클라이언트측 코드, 소켓 구성요소 구조체에 접속할 서버의 ip를 적어준다. 클라이언트에서는 bind함수 대신 connect함수를 사용한다.



    char cMsg[] = "Client Send";
    send(hSocket, cMsg, strlen(cMsg), 0);


    while (true)
    {
        char cBuffer[PACKET_SIZE] = {};
        float temps;

        recv(hSocket, cBuffer, PACKET_SIZE, 0);

        memcpy(&temps, cBuffer, sizeof(float));

        cout << "RECV Msg : "<< temps <<endl;

        if (temps == -1)
        {
            printf("종료시점!\n");
            closesocket(hSocket);
            break;
        }

    }
    printf("TCP 통신을 종료합니다(클라이언트)\n");

    closesocket(hSocket);

    WSACleanup();

    return 0;
}