#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>

#define PORT 4578
#define PACKET_SIZE 1024
#define SERVER_IP "192.168.0.2"   //서버 IP를 지정 "211.32.63.121"(기숙사)

using std::cout;
using std::endl;

int main() {
	int sock;
	struct sockaddr_in serv_addr;
	char message[PACKET_SIZE];
	float temps;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serv_addr.sin_port = htons(PORT);

	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	while (true) {
		read(sock, message, sizeof(message) - 1);
		memcpy(&temps, message, sizeof(float));

		cout << "RECV : " << temps << endl;
	}

	close(sock);

	return 0;
}
