#include "function.h" 


int main(int, char**)
{
	//��� ����
	tcp_server_onoff(true);

	clock_t start, end;
	start = clock();

	int init = 0;

	/*
	do
	{
		++init;
		tcp_server("s", 90);
		tcp_server("c", 90);
		tcp_server("m", 240);

	} while (init < 1);
	*/
	

	//���� ���� ����ó�� ����
	//(����ũž)
	//video_main("D:\\OneDrive - ���ִ��б�\\��������_������Ʈ(�����ڷ�)\\track_line_test.mp4", "D:\\OneDrive - ���ִ��б�\\��������_������Ʈ(�����ڷ�)\\track_line_test.avi");
	
	//(laptop)
	//video_main("C:\\Users\\82109\\OneDrive - ���ִ��б�\\��������_������Ʈ(�����ڷ�)\\track_line_test.mp4", "C:\\Users\\82109\\OneDrive - ���ִ��б�\\��������_������Ʈ(�����ڷ�)\\track_line_test.avi");

	VideoCapture video(0);			//____��Ʈ�� ���� ī�޶� ���_____

	video.set(CAP_PROP_FRAME_WIDTH, 640);
	video.set(CAP_PROP_FRAME_HEIGHT, 360);


	Mat original_video;


	if (!video.isOpened())
	{
		cout << "������ ������ ���� �����ϴ�. \n" << endl;

		char a;
		cin >> a;

		return -1;
	}

	video.read(original_video);

	if (original_video.empty())
	{
		cout << "������ �ҷ� �� �� �����ϴ�" << endl;
		return -1;
	}

	//
	while (true)
	{
		video.read(original_video);
		if (original_video.empty()) break;

		imshow("��������", original_video);

		//�켱���� �Լ��� ����
		stop_check(original_video);							//�켱���� ���� �Լ�

		//��ȣ�� �Լ��� ����
		//traffic_lights_check(original_video);					//��ȣ�� ���� �Լ�


		if (waitKey(1) == 27) break; //ESCŰ ������ ����  
	}


	end = clock();
	printf("Ÿ�̸� : %.2f��\n\n", ((float)(end - start) / CLOCKS_PER_SEC));
	
	//��� ����
	tcp_server_onoff(false);

	return 0;
}