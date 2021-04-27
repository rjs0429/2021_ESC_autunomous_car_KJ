#include "function.h"

void stop_check(Mat copy) {				//영상을 마스킹 -> 노이즈 제거 -> 라벨링 -> 판단 범위 지정 -> (그레이스케일 & 가우시안 블러) -> HoughCircles
										//원이 검출되어도 일정 거리안에 들어올 때만 판단

	Mat stop_video;

	copy.copyTo(stop_video);

	Mat stop_video_hsv;

	cvtColor(stop_video, stop_video_hsv, COLOR_BGR2HSV);

	Mat red_mask, red_image;

	//학교에서 뽑은 빨간색
	//Scalar lower_red = Scalar(160, 50, 50);				// 10cm 기준 200 130
	//Scalar upper_red = Scalar(179, 255, 255);

	//집에서 뽑은 빨간색
	Scalar lower_red = Scalar(0, 100, 100);					// 10cm 기준 270 170		3cm 기준 480 320
	Scalar upper_red = Scalar(15, 255, 255);

	//Scalar lower_red = Scalar(38, 50, 50);					// 초록 테스트
	//Scalar upper_red = Scalar(80, 255, 255);



	inRange(stop_video_hsv, lower_red, upper_red, red_mask);


	//Erode - Dilate = Opening 연산 : 주로 작은 노이즈들을 제거하는데 사용한다.
	//morphological opening 작은 점들을 제거 
	erode(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
	dilate(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));

	//Dilate - Erode = Closing 연산 : 보통 한 객체를 추출했을 때 두개 이상의 작은 부분으로 나올 경우 큰 객체로 합칠 때 사용한다. 
	//morphological closing 영역의 구멍 메우기 
	dilate(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
	erode(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));




	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(red_mask, img_labels,
		stats, centroids, 8, CV_32S);


	//영역박스 그리기
	int max = -1, idx = 0;
	for (int j = 1; j < numOfLables; j++) {

		int area = stats.at<int>(j, CC_STAT_AREA);

		if (max < area)
		{
			max = area;
			idx = j;
		}
	}


	int stop_left = stats.at<int>(idx, CC_STAT_LEFT);
	int stop_top = stats.at<int>(idx, CC_STAT_TOP);
	int stop_width = stats.at<int>(idx, CC_STAT_WIDTH);
	int stop_height = stats.at<int>(idx, CC_STAT_HEIGHT);

	rectangle(stop_video, Point(stop_left, stop_top), Point(stop_left + stop_width, stop_top + stop_height),
		Scalar(255, 0, 0), 1);

	cout << "p1 - " << stop_left << "," << stop_top << " p2 - " << stop_left + stop_width << "," << stop_top + stop_height << endl;

	imshow("원본영상_라벨링", stop_video);

	cout << "stop_width, stop_height = " << stop_width << " " << stop_height << endl;				//width 100  height 70이면 원을 인식하기 시작
																									//width 200  height 130정도면 10cm이내 진입(아마도?)

	int fontface = FONT_HERSHEY_SIMPLEX;	//putText를 위한 변수들
	double scale = 2.5;
	int thickness = 2;
	int baseline = 0;

	if (numOfLables > 1)													//빨간 표지판일 경우에만 원을 검출하겠다 (첫번째 기준 - 색 인식)
	{
		Rect red_stop(stop_left, stop_top, stop_width, stop_height);		//빨간 사각형 범위 지정

		Mat find_stop_circle = stop_video(red_stop);						//빨간 사각형 범위에서만 원 검출	(원 검출 없이 일정 픽셀 크기가 넘어가면 정지하는 방식도 대안)
																			//대신 HSV값을 잘 찾아봐야..



		if ((stop_width >= 270) && (stop_width <= 480) && (stop_height >= 170) && (stop_height <= 320))
		{
			putText(red_mask, "STOP", Point(40, 80), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);
			tcp_server("m", 0);
		}

		imshow("red_mask", red_mask);
	}

}


