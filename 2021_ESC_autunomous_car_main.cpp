#include "function.h" 


int main(int, char**)
{
	//통신 시작
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
	

	//라인 검출 영상처리 시작
	//(데스크탑)
	//video_main("D:\\OneDrive - 공주대학교\\자율주행_프로젝트(영상자료)\\track_line_test.mp4", "D:\\OneDrive - 공주대학교\\자율주행_프로젝트(영상자료)\\track_line_test.avi");
	
	//(laptop)
	//video_main("C:\\Users\\82109\\OneDrive - 공주대학교\\자율주행_프로젝트(영상자료)\\track_line_test.mp4", "C:\\Users\\82109\\OneDrive - 공주대학교\\자율주행_프로젝트(영상자료)\\track_line_test.avi");

	VideoCapture video(0);			//____노트북 내장 카메라 사용_____

	video.set(CAP_PROP_FRAME_WIDTH, 640);
	video.set(CAP_PROP_FRAME_HEIGHT, 360);


	Mat original_video;


	if (!video.isOpened())
	{
		cout << "동영상 파일을 열수 없습니다. \n" << endl;

		char a;
		cin >> a;

		return -1;
	}

	video.read(original_video);

	if (original_video.empty())
	{
		cout << "비디오를 불러 올 수 없습니다" << endl;
		return -1;
	}

	//
	while (true)
	{
		video.read(original_video);
		if (original_video.empty()) break;

		imshow("원본영상", original_video);

		//우선정지 함수만 실행
		stop_check(original_video);							//우선정지 검출 함수

		//신호등 함수만 실행
		//traffic_lights_check(original_video);					//신호등 검출 함수


		if (waitKey(1) == 27) break; //ESC키 누르면 종료  
	}


	end = clock();
	printf("타이머 : %.2f초\n\n", ((float)(end - start) / CLOCKS_PER_SEC));
	
	//통신 종료
	tcp_server_onoff(false);

	return 0;
}