#include "function.h" 

using  std::cout;
using  std::endl;
using  std::cin;
using  std::cerr;


int main(int, char**)
{
	//��� ����
	tcp_server_onoff(true);

	clock_t start, end;
	start = clock();

	//����ó�� ����
	video_main("D:\\OneDrive - ���ִ��б�\\��������_������Ʈ\\curb_test.mp4", "D:\\OneDrive - ���ִ��б�\\��������_������Ʈ\\curb_test.avi");

	end = clock();
	printf("Ÿ�̸� : %.2f��\n\n", ((float)(end - start) / CLOCKS_PER_SEC));
	
	//��� ����
	tcp_server_onoff(false);

	return 0;
}