#include "function.h" 

using  std::cin;
using  std::cerr;


int main(int, char**)
{
	clock_t start, end;
	start = clock();
	string filename[] = { "D:\\OneDrive - ���ִ��б�\\�������� ������Ʈ\\live.avi", "D:\\OneDrive - ���ִ��б�\\�������� ������Ʈ\\live2.avi" };
	//����ó�� ����
	video_main("D:\\OneDrive - ���ִ��б�\\�������� ������Ʈ\\��ȭ��.mp4", filename);

	end = clock();
	printf("Ÿ�̸� : %.2f��\n\n", ((float)(end - start) / CLOCKS_PER_SEC));

	return 0;
}