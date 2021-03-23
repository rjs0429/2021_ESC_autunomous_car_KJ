#include <stdio.h>
#include <iostream>  
#include<winsock.h>

#pragma comment(lib, "ws2_32")      //������ ������ ������ϵ��� ������ �������� ��ũ

#define PORT 4578
#define PACKET_SIZE 1024
#define SERVER_IP "192.168.0.2"   //���� IP�� ���� "211.32.63.121"(�����)

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

    connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr));      // Ŭ���̾�Ʈ�� �ڵ�, ���� ������� ����ü�� ������ ������ ip�� �����ش�. Ŭ���̾�Ʈ������ bind�Լ� ��� connect�Լ��� ����Ѵ�.



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
            printf("�������!\n");
            closesocket(hSocket);
            break;
        }

    }
    printf("TCP ����� �����մϴ�(Ŭ���̾�Ʈ)\n");

    closesocket(hSocket);

    WSACleanup();

    return 0;
}