#include "function.h" 

using  std::cin;
using  std::cerr;


int main(int, char**)
{
	//��� ����
	tcp_server_onoff(true);

	clock_t start, end;
	start = clock();

	//����ó�� ����
	video_main("http://192.168.0.34:8090/?action=stream", "D:\\OneDrive - ���ִ��б�\\�������� ������Ʈ\\live.avi");

	end = clock();
	printf("Ÿ�̸� : %.2f��\n\n", ((float)(end - start) / CLOCKS_PER_SEC));
	
	//��� ����
	tcp_server_onoff(false);

	return 0;
}