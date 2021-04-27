#include "function.h"

void stop_check(Mat copy) {				//������ ����ŷ -> ������ ���� -> �󺧸� -> �Ǵ� ���� ���� -> (�׷��̽����� & ����þ� ��) -> HoughCircles
										//���� ����Ǿ ���� �Ÿ��ȿ� ���� ���� �Ǵ�

	Mat stop_video;

	copy.copyTo(stop_video);

	Mat stop_video_hsv;

	cvtColor(stop_video, stop_video_hsv, COLOR_BGR2HSV);

	Mat red_mask, red_image;

	//�б����� ���� ������
	//Scalar lower_red = Scalar(160, 50, 50);				// 10cm ���� 200 130
	//Scalar upper_red = Scalar(179, 255, 255);

	//������ ���� ������
	Scalar lower_red = Scalar(0, 100, 100);					// 10cm ���� 270 170		3cm ���� 480 320
	Scalar upper_red = Scalar(15, 255, 255);

	//Scalar lower_red = Scalar(38, 50, 50);					// �ʷ� �׽�Ʈ
	//Scalar upper_red = Scalar(80, 255, 255);



	inRange(stop_video_hsv, lower_red, upper_red, red_mask);


	//Erode - Dilate = Opening ���� : �ַ� ���� ��������� �����ϴµ� ����Ѵ�.
	//morphological opening ���� ������ ���� 
	erode(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
	dilate(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));

	//Dilate - Erode = Closing ���� : ���� �� ��ü�� �������� �� �ΰ� �̻��� ���� �κ����� ���� ��� ū ��ü�� ��ĥ �� ����Ѵ�. 
	//morphological closing ������ ���� �޿�� 
	dilate(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
	erode(red_mask, red_mask, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));




	Mat img_labels, stats, centroids;
	int numOfLables = connectedComponentsWithStats(red_mask, img_labels,
		stats, centroids, 8, CV_32S);


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


	int stop_left = stats.at<int>(idx, CC_STAT_LEFT);
	int stop_top = stats.at<int>(idx, CC_STAT_TOP);
	int stop_width = stats.at<int>(idx, CC_STAT_WIDTH);
	int stop_height = stats.at<int>(idx, CC_STAT_HEIGHT);

	rectangle(stop_video, Point(stop_left, stop_top), Point(stop_left + stop_width, stop_top + stop_height),
		Scalar(255, 0, 0), 1);

	cout << "p1 - " << stop_left << "," << stop_top << " p2 - " << stop_left + stop_width << "," << stop_top + stop_height << endl;

	imshow("��������_�󺧸�", stop_video);

	cout << "stop_width, stop_height = " << stop_width << " " << stop_height << endl;				//width 100  height 70�̸� ���� �ν��ϱ� ����
																									//width 200  height 130������ 10cm�̳� ����(�Ƹ���?)

	int fontface = FONT_HERSHEY_SIMPLEX;	//putText�� ���� ������
	double scale = 2.5;
	int thickness = 2;
	int baseline = 0;

	if (numOfLables > 1)													//���� ǥ������ ��쿡�� ���� �����ϰڴ� (ù��° ���� - �� �ν�)
	{
		Rect red_stop(stop_left, stop_top, stop_width, stop_height);		//���� �簢�� ���� ����

		Mat find_stop_circle = stop_video(red_stop);						//���� �簢�� ���������� �� ����	(�� ���� ���� ���� �ȼ� ũ�Ⱑ �Ѿ�� �����ϴ� ��ĵ� ���)
																			//��� HSV���� �� ã�ƺ���..



		if ((stop_width >= 270) && (stop_width <= 480) && (stop_height >= 170) && (stop_height <= 320))
		{
			putText(red_mask, "STOP", Point(40, 80), fontface, scale, CV_RGB(255, 255, 255), thickness, 8);
			tcp_server("m", 0);
		}

		imshow("red_mask", red_mask);
	}

}


