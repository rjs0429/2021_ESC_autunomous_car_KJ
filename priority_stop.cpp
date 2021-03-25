#include "function.h" 

//우선정지 판단 함수(멈춰!)
bool is_priority_stop(Mat video_for_copy) {
	int range_count = 0;

	Scalar red(0, 0, 255);

	//Scalar blue(255, 0, 0);
	//Scalar yellow(0, 255, 255);
	//Scalar magenta(255, 0, 255);

	Mat rgb_color = Mat(1, 1, CV_8UC3, red);
	Mat hsv_color;

	cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);


	int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];					//hue - 색조 (0~180의 값)
	int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];				//saturation - 채도 (0~255의 값) (255의 값 = 맑고 깨끗한 원색, 0의 값 = 백색이 많이 섞인 색)
	int value = (int)hsv_color.at<Vec3b>(0, 0)[2];					//value - 명도 (0~255의 값) (밝기의 정도)


	/*cout << "hue = " << hue << endl;
	cout << "saturation = " << saturation << endl;
	cout << "value = " << value << endl;*/


	int low_hue = hue - 10;
	int high_hue = hue + 10;


	//cout << "low_hue = " << low_hue << " high_hue = " << high_hue << endl;			//low_hue = -10		high_hue = 10

	int low_hue1 = 0, low_hue2 = 0;
	int high_hue1 = 0, high_hue2 = 0;

	if (low_hue < 10) {
		range_count = 2;

		high_hue1 = 180;												//low_hue1 = 170		high_hue1 = 180
		low_hue1 = low_hue + 180;										//low_hue2 = 0		high_hue1 = 10
		high_hue2 = high_hue;
		low_hue2 = 0;
	}
	else if (high_hue > 170) {
		range_count = 2;

		high_hue1 = low_hue;											//low_hue1 = 		high_hue1 = 
		low_hue1 = 180;													//low_hue2 = 		high_hue1 = 
		high_hue2 = high_hue - 180;
		low_hue2 = 0;
	}
	else {
		range_count = 1;

		low_hue1 = low_hue;
		high_hue1 = high_hue;
	}


	//cout << low_hue1 << "  " << high_hue1 << endl;
	//cout << low_hue2 << "  " << high_hue2 << endl;

	Mat img_frame, img_hsv;

	bool is_stop = false;			//boolean형 대신 int형으로 우선정지 판단

	int fontface = FONT_HERSHEY_SIMPLEX;	//putText를 위한 변수들
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;

	video_for_copy.copyTo(img_frame);

	if (img_frame.empty()) {
		cerr << "ERROR! blank frame grabbed\n";
		return 0;
	}

	//HSV로 변환
	cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);

	//지정한 HSV 범위를 이용하여 영상을 이진화
	Mat img_mask1, img_mask2;
	inRange(img_hsv, Scalar(low_hue1, 50, 50), Scalar(high_hue1, 255, 255), img_mask1);			//(170,50,50) ~ (180,255,255) 
	if (range_count == 2) {
		inRange(img_hsv, Scalar(low_hue2, 50, 50), Scalar(high_hue2, 255, 255), img_mask2);		//(0,50,50) ~ (10,255,255) 
		img_mask1 |= img_mask2;
	}


	//Erode 연산 : 필터 내부의 가장 낮은(어두운) 값으로 변환(and) - 침식연산
	//Dilate 연산 : 필터 내부의 가장 높은(밝은) 값으로 변환(or) - 팽창연산


	//Erode - Dilate = Opening 연산 : 주로 작은 노이즈들을 제거하는데 사용한다.
	//morphological opening 작은 점들을 제거 
	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//Dilate - Erode = Closing 연산 : 보통 한 객체를 추출했을 때 두개 이상의 작은 부분으로 나올 경우 큰 객체로 합칠 때 사용한다. 
	//morphological closing 영역의 구멍 메우기 
	dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));



	//라벨링 
	//connectedComponentsWithStats() 함수의 인자는 총 6가지를 입력.
	//입력 이미지, 라벨링 결과 이미지, 라벨링 된 이미지의 정보, 라벨링 된 이미지의 중심 좌표, 4 or 8 방향, 타입
	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
		stats, centroids, 8, CV_32S);


	cout << "numOfLables = " << numOfLables << endl;


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


	//cout << "stop_left, stop_top = " << stop_left << " " << stop_top << endl;
	//cout << "stop_width, stop_height = " << stop_width  << " " << stop_height << endl;

	if (stop_width >= 165 && stop_height >= 115)							//영상을 받아올 때, 빨간 영역이 있을 경우 대비 화면에 일정 비율에 빨간 영역이 있을때만 실행
	{

		rectangle(img_frame, Point(stop_left, stop_top), Point(stop_left + stop_width, stop_top + stop_height),
			Scalar(255, 0, 0), 1);

		imshow("이진화 영상", img_mask1);

		imshow("원본 영상", img_frame);

		Rect red_stop(stop_left, stop_top, stop_width, stop_height);		//빨간 사각형 범위 지정

		Mat find_stop_circle = img_frame(red_stop);							//빨간 사각형 범위에서만 원 검출

		//	imshow("빨간 표지판 추출", find_stop_circle);

		Mat red_stop_gray;
		cvtColor(find_stop_circle, red_stop_gray, COLOR_BGR2GRAY);

		GaussianBlur(red_stop_gray, red_stop_gray, Size(9, 9), 2, 2);

		//	imshow("그레이스케일+가우시안블러", red_stop_gray);

		vector<Vec3f> circles;
		HoughCircles(red_stop_gray, circles, HOUGH_GRADIENT, 1, red_stop_gray.rows / 8, 200, 50, 0, 0);


		cout << "circles_size : " << circles.size() << endl;

		if (circles.size() >= 1)
		{
			is_stop = true;	// 화면에 보이는 빨간 우선정지 영역에 원이 검출되면 우선정지 표지판으로 인식
		}

		else if (circles.size() < 1)
		{
			is_stop = false;
		}

		for (size_t i = 0; i < circles.size(); i++) {
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);

			circle(red_stop_gray, center, 3, Scalar(0, 255, 0), -1, 8, 0);

			circle(red_stop_gray, center, radius, Scalar(0, 0, 255), 3, 8, 0);
		}

		if (is_stop == true)
		{
			putText(red_stop_gray, "STOP", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);

		}

		imshow("최종 판별", red_stop_gray);



	}

	return is_stop;
}