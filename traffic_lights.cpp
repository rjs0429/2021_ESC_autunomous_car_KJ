#include "function.h"

#define CIRCLES 4									//����Ǿ�� �ϴ� ���� 4��

bool is_traffic_lights = false;						//�ʱ� ��ȣ�� ������ ���� ��������
int count_circles = 0;

int circles_x[CIRCLES];

vector<Vec3f> circles;

void check_circle_lights(Mat filtered_video) {

	HoughCircles(filtered_video, circles, HOUGH_GRADIENT, 2, 30, 200, 50, 10, 50);

	//cout << "���� ��Ŭ �Լ� ����!" << endl;

	for (int i = 0; i < circles.size(); i++) {
		//{ cout << "circles : " << circles.size() << " center[" << i << "]: " << endl; }//�� �� ������� �߽��� �� ���� ���
	}

	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		circle(filtered_video, center, 3, Scalar(0, 0, 255), -1, 8, 0);			//circle center ȭ�鿡 �� �׸��� �Լ� 
		circle(filtered_video, center, radius, Scalar(0, 0, 255), 3, 8, 0);		//outline  ȭ�鿡 �� �׸��� �Լ� 


		int x = circles[i][0];		//x 
		int y = circles[i][1];		//y

		//cout << "x: " << x << "y: " << y << endl;

	}


	if (circles.size() == 4)
	{
		for (int i = 0; i < CIRCLES; i++) {
			circles_x[i] = circles[i][0];
		}

		sort(circles_x, circles_x + CIRCLES);				//�������� ���� (������ ����� ��ȸ�� ��ȸ��)

		for (int i = 0; i < CIRCLES; i++) {
			cout << "x : " << circles_x[i] << endl;
		}

		is_traffic_lights = true;						//x�� ��ǥ�� ���� ��ȣ���� �´��� ��Ȯ�� ���� �ڵ� �߰��� ���� ����
	}

}

void traffic_lights_check(Mat copy) {					//������ �޾ƿ� �� ���� ����ŷ�Լ����� hsv��ȯ�� ������ �Ѱ���


	Mat traffic_lights_video;

	copy.copyTo(traffic_lights_video);									//���� copy

	Mat video_hsv;

	cvtColor(traffic_lights_video, video_hsv, COLOR_BGR2HSV);			//hsv ��ȯ

	//Rect roi(Point(160, 120), Point(480, 240));		//Rect �Լ��� 1.Point�� + ���� ������ ���ų� 
														//2.Point Point ������ ���ų� �� ���� ����� �ִ�.

	int p1_x = (traffic_lights_video.cols / 4);			//640*360 �������� ��ȣ���� ��ġ�� ROI (160,120) - (480,240)�� ����
	int p1_y = (traffic_lights_video.rows / 3);

	int p2_x = p1_x + p1_x * 2;
	int p2_y = p1_y + p1_y;

	//cout << "traffic_lights_video.rows : " << traffic_lights_video.rows << endl;
	//cout << "traffic_lights_video.cols : " << traffic_lights_video.cols << endl;

	//cout << "p1_xy - " << p1_x << " " << p1_y << endl;
	//cout << "p2_xy - " << p2_x << " " << p2_y << endl;

	Rect roi(Point(p1_x, p1_y), Point(p2_x, p2_y));									//roi ���� ����

	Mat gray_roi = traffic_lights_video(roi);

	Mat hsv_roi = video_hsv(roi);

	Mat video_gray;

	cvtColor(gray_roi, video_gray, COLOR_BGR2GRAY);						//gray ��ȯ -> HoughCircles������ ����

	double sigmaColor = 10.0;
	double sigmaSpace = 10.0;

	Mat filtered_video;

	bilateralFilter(video_gray, filtered_video, -1, sigmaColor, sigmaSpace);	//����þȺ� ��� ���

	if (is_traffic_lights == false)
		check_circle_lights(filtered_video);


	if (is_traffic_lights == true)										//��ȣ������ Ȯ���� ����
	{
		go_left_green(hsv_roi);											//3���� ���� �� �Ǻ��� ��

		stop_yellow(hsv_roi);

		stop_red(hsv_roi);

		/*																//�ʷϻ��� �Ǻ��� ��

		Scalar lower_green = Scalar(38, 50, 50);		//65 ~ 75 50 50
		Scalar upper_green = Scalar(80, 255, 255);

		Mat green_mask;

		inRange(hsv_roi, lower_green, upper_green, green_mask);


		Mat img_labels, stats, centroids;
		int numOfLables = connectedComponentsWithStats(green_mask, img_labels,
			stats, centroids, 8, CV_32S);

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

		cout << "stop_width, stop_height = " << width << " " << height << endl;		//�� 70cm �Ÿ�����

																					//���� ���̴� 28 26�̻� 30 30 ����(��)  22 18  22 20(�����)
																					//ȭ��ǥ�� ���̴� 26 20�̻� 27 22����   20 18(�����)
		int fontface = FONT_HERSHEY_SIMPLEX;	//putText�� ���� ������
		double scale = 0.5;
		int thickness = 1;
		int baseline = 0;

		if ((width >= 22) && (width <= 24) && (height >= 18) && (height <= 20))
			putText(filtered_video, "Turn Right", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
		else if ((width >= 20) && (width <= 22) && (height >= 16) && (height <= 18))
			putText(filtered_video, "Turn Left", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
		else
			putText(filtered_video, "", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);

		imshow("green_masking", green_mask);

		*/
	}

	imshow("gray_masking", filtered_video);


	//cout << "is_traffic_lights - " << is_traffic_lights << endl;

}


void go_left_green(Mat green_go_light) {		//�ʷϻ� ����ŷ 

	Mat check_go_green;

	Mat green_mask;

	Scalar lower_green = Scalar(38, 50, 50);		//65 ~ 75 50 50
	Scalar upper_green = Scalar(80, 255, 255);

	green_go_light.copyTo(check_go_green);

	inRange(check_go_green, lower_green, upper_green, green_mask);

	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(green_mask, img_labels,
		stats, centroids, 8, CV_32S);

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

	//cout << "Green_x1 - " << left << "," << " Green x2 - " << left + width << endl;


	//cout << "green_width, green_height = " << width << " " << height << endl;		//�� 70cm �Ÿ����� 

																				//���� ���̴� 28 26�̻� 30 30 ����(��)  22 18  22 20(�����)
																				//ȭ��ǥ�� ���̴� 26 20�̻� 27 22����   20 18(�����)
	int fontface = FONT_HERSHEY_SIMPLEX;	//putText�� ���� ������
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;


	int check_center = (left + left + width) / 2;						//����ŷ �� ������ �������� ���� ��ǥ ���� 
																		//����� ���� x��ǥ�� �ִ�

														//�ణ�� ���̰� ������ (������+���� x��ǥ)/2 - �� x��ǥ�� 5������ ���� ���� �ִٸ� (������ ���� �� �����Ƿ� 0���� Ŀ����)
	if ((width >= 18) && (height >= 14) && (check_center - circles_x[2] >= 0) && (check_center - circles_x[2] <= 5)) {
		putText(green_mask, "Turn Left", Point(50, 60), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);
		tcp_server("t", 115);	//Turn Left 115(���� ��)
	}
	if ((width >= 18) && (height >= 14) && (check_center - circles_x[3] >= 0) && (check_center - circles_x[3] <= 5)) {
		putText(green_mask, "Turn Right", Point(50, 60), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);
		tcp_server("t", 65);	//Turn Right 65(���� ��)
	}
	else
		putText(green_mask, "", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);


	imshow("green_masking", green_mask);

}



void stop_red(Mat red_light) {					//������ ����ŷ 

	Mat stop;

	Mat red_mask;

	Scalar lower_red = Scalar(0, 100, 100);
	Scalar upper_red = Scalar(15, 255, 255);

	red_light.copyTo(stop);

	inRange(stop, lower_red, upper_red, red_mask);

	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(red_mask, img_labels,
		stats, centroids, 8, CV_32S);

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


	//cout << "Red_x1 - " << left << "," << " Red_x2 - " << left + width << endl;

	//cout << "red_width, red_height = " << width << " " << height << endl;		//�� 70cm �Ÿ����� 

																				//���� ���̴� 28 26�̻� 30 30 ����(��)  22 18  22 20(�����)
																				//ȭ��ǥ�� ���̴� 26 20�̻� 27 22����   20 18(�����)
	int fontface = FONT_HERSHEY_SIMPLEX;	//putText�� ���� ������
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;

	int check_center = (left + left + width) / 2;


	if ((width >= 18) && (height >= 14) && (check_center - circles_x[0] >= 0) && (check_center - circles_x[0] <= 5))
		putText(red_mask, "STOP", Point(50, 60), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);
	else
		putText(red_mask, " ", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);


	imshow("red_masking", red_mask);

}

void stop_yellow(Mat yellow_light) {			//����� ����ŷ 


	Mat slow_and_stop;

	Mat yellow_mask;

	Scalar lower_yellow = Scalar(27, 50, 50);		//65 ~ 75 50 50
	Scalar upper_yellow = Scalar(34, 255, 255);

	yellow_light.copyTo(slow_and_stop);

	inRange(slow_and_stop, lower_yellow, upper_yellow, yellow_mask);

	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(yellow_mask, img_labels,
		stats, centroids, 8, CV_32S);

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

	//cout << "Yello_x1 - " << left << "," << " Yellow_x2 - " << left + width << endl;


	//cout << "yellow_width, yellow_height = " << width << " " << height << endl;		//�� 70cm �Ÿ����� 

																				//���� ���̴� 28 26�̻� 30 30 ����(��)  22 18  22 20(�����)
																				//ȭ��ǥ�� ���̴� 26 20�̻� 27 22����   20 18(�����)
	int fontface = FONT_HERSHEY_SIMPLEX;	//putText�� ���� ������
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;

	int check_center = (left + left + width) / 2;


	if ((width >= 18) && (height >= 14) && (check_center - circles_x[1] >= 0) && (check_center - circles_x[1] <= 5))
		putText(yellow_mask, "Slow and STOP", Point(50, 60), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);
	else
		putText(yellow_mask, " ", Point(50, 60), fontface, scale, CV_RGB(0, 0, 0), thickness, 8);


	imshow("yellow_masking", yellow_mask);

}
