#include "function.h"


void get_video(Mat copy) {						//���� ������ �޾ƿ� HSV��ȯ �� ���� �� ����ŷ�� ���� �� ���� ����ŷ �Լ� ȣ��

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

	int fontface = FONT_HERSHEY_SIMPLEX;	//putText�� ���� ������
	double scale = 0.5;
	int thickness = 1;
	int baseline = 0;


	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(each_video, img_labels,
		stats, centroids, 8, CV_32S);


	//cout << masking_name << " numOfLables = " << numOfLables << endl;


	//�����ڽ� �׸���
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


	
	int shape_check = 0;					//���� ���� �˰��� ����

	bool is_circle;							//���� ���� �˰��� + HoughCircles���� ���������� ���� ũ�Ⱑ Ŭ ���� Houghcircles�� �����ϰڴ�(������ ����)



	//contour�� ã�´�.
	vector<vector<Point>> contours;
	findContours(each_video, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	//contour�� �ٻ�ȭ�Ѵ�.
	vector<Point2f> approx;

	//cout << "contours size : " << contours.size() << endl; 

	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true) * 0.02, true);

		if (fabs(contourArea(Mat(approx))) > 500 && fabs(contourArea(Mat(approx))) < 5000)			// ���� �����ϱ� ���� ���ǹ� + ������ ����
		{

			shape_check = approx.size();
			is_circle = true;

			cout << masking_name << " shape_check : " << shape_check << endl;

		}
		else if (fabs(contourArea(Mat(approx))) > 200 && fabs(contourArea(Mat(approx))) < 500)		//ȭ��ǥ�� �����ϱ� ���� ���ǹ�
		{
			shape_check = approx.size();
			is_circle = false;
		}

	}


	if (numOfLables >= 2)					//���� ���� �Ǹ�
	{

		vector<Vec3f> circles;
		HoughCircles(each_video, circles, HOUGH_GRADIENT, 1, 100, 200, 10, 10, 100);			//�Ÿ��� ���� ���� ������ �� ��


		cout << "circles_���� : " << circles.size() << endl;

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
		else if (masking_name.compare("yellow_masking") == 0)					//yellow���� ������ ���� 1�� �ִµ� ������� �� �� ������ 1������ 2���� �Դٰ���
		{																		//�����ν� ���ǿ� �ش������, ������ ���� 2�� �̻��� �� ��� �ؾ��ϴ°�?

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
			else if ((shape_check == 7) && (circles.size() < 1))				//�Ÿ��� �� ��쿡�� ȭ��ǥ�� �ν��ϴ� ���	
			{
				judgement = "turn left!";
			}
		}

		putText(each_video, judgement, Point(50, 60), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);

	}


	resize(each_video, each_video, Size(SC * each_video.cols, SC * each_video.rows));
	imshow(masking_name, each_video);

}