#include "function.h"


void get_video(Mat copy) {						//주행 영상을 받아와 HSV변환 후 색깔 별 마스킹을 위해 각 색깔별 마스킹 함수 호출

	Mat traffic_lights_video;

	copy.copyTo(traffic_lights_video);

	Mat video_hsv;

	cvtColor(traffic_lights_video, video_hsv, COLOR_BGR2HSV);


	stop_red(video_hsv);

	stop_yellow(video_hsv);

	go_left_green(video_hsv);

}


void stop_red(Mat red_light) {

	Mat check_red;

	Mat red_mask;

	Scalar lower_red = Scalar(160, 50, 50);
	Scalar upper_red = Scalar(179, 255, 255);

	red_light.copyTo(check_red);

	inRange(check_red, lower_red, upper_red, red_mask);

	common_processing(red_mask, "red_masking");
}

void stop_yellow(Mat yellow_light) {


	Mat check_yellow;

	Mat yellow_mask;

	Scalar lower_yellow = Scalar(22, 50, 50);
	Scalar upper_yellow = Scalar(38, 255, 255);

	yellow_light.copyTo(check_yellow);

	inRange(check_yellow, lower_yellow, upper_yellow, yellow_mask);

	common_processing(yellow_mask, "yellow_masking");

}

void go_left_green(Mat green_go_light) {

	Mat check_go_green;

	Mat green_mask;

	Scalar lower_green = Scalar(38, 50, 50);
	Scalar upper_green = Scalar(75, 255, 255);

	green_go_light.copyTo(check_go_green);

	inRange(check_go_green, lower_green, upper_green, green_mask);

	common_processing(green_mask, "green_masking");

}


void common_processing(Mat each_video, string masking_name) {

	string judgement;

	int fontface = FONT_HERSHEY_SIMPLEX;	//putText를 위한 변수들
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;


	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(each_video, img_labels,
		stats, centroids, 8, CV_32S);


	//cout << masking_name << " numOfLables = " << numOfLables << endl;


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

	int left = stats.at<int>(idx, CC_STAT_LEFT);
	int top = stats.at<int>(idx, CC_STAT_TOP);
	int width = stats.at<int>(idx, CC_STAT_WIDTH);
	int height = stats.at<int>(idx, CC_STAT_HEIGHT);


	
	int shape_check = 0;					//도형 검출 알고리즘 변수

	bool is_circle;							//도형 검출 알고리즘 + HoughCircles에서 도형검출의 일정 크기가 클 때만 Houghcircles를 참고하겠다(노이즈 제거)



	//contour를 찾는다.
	vector<vector<Point>> contours;
	findContours(each_video, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	//contour를 근사화한다.
	vector<Point2f> approx;

	//cout << "contours size : " << contours.size() << endl; 

	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true) * 0.02, true);

		if (fabs(contourArea(Mat(approx))) > 500 && fabs(contourArea(Mat(approx))) < 5000)			// 원을 검출하기 위한 조건문 + 노이즈 제거
		{

			shape_check = approx.size();
			is_circle = true;

			cout << masking_name << " shape_check : " << shape_check << endl;

		}
		else if (fabs(contourArea(Mat(approx))) > 200 && fabs(contourArea(Mat(approx))) < 500)		//화살표를 검출하기 위한 조건문
		{
			shape_check = approx.size();
			is_circle = false;
		}

	}


	if (numOfLables >= 2)					//색이 검출 되면
	{

		vector<Vec3f> circles;
		HoughCircles(each_video, circles, HOUGH_GRADIENT, 1, 100, 200, 10, 10, 100);			//거리에 따라 값이 수정될 수 도


		cout << "circles_개수 : " << circles.size() << endl;

		if (circles.size() >= 1)
		{
			for (size_t i = 0; i < circles.size(); i++) {
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);

				circle(each_video, center, 3, Scalar(0, 255, 0), -1, 8, 0);

				circle(each_video, center, radius, Scalar(0, 0, 255), 3, 8, 0);
			}
		}

		if (masking_name.compare("red_masking") == 0)
		{
			if ((shape_check == 8) && (circles.size() == 1) && (is_circle == true))
			{
				judgement = "stop!";
			}
		}
		else if (masking_name.compare("yellow_masking") == 0)					//yellow에서 노이즈 원이 1개 있는데 노란불일 때 원 검출이 1개에서 2개로 왔다갔다
		{																		//함으로써 조건에 해당되지만, 노이즈 원이 2개 이상일 땐 어떻게 해야하는가?

			if ((shape_check == 8) && (circles.size() == 1) && (is_circle == true))
			{
				judgement = "slow and stop!";
			}

		}
		else if (masking_name.compare("green_masking") == 0)
		{
			if ((shape_check == 8) && (circles.size() == 1) && (is_circle == true))
			{
				judgement = "turn right!";
			}
			else if ((shape_check == 7) && (circles.size() < 1))				//거리가 멀 경우에도 화살표를 인식하는 방법	
			{
				judgement = "turn left!";
			}
		}

		putText(each_video, judgement, Point(50, 60), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);

	}


	resize(each_video, each_video, Size(SC * each_video.cols, SC * each_video.rows));
	imshow(masking_name, each_video);

}