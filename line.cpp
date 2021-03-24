#include "function.h" 

#define PI 3.1415926535
#define SC 1.5					//���� ǥ�� ������

using namespace cv;

//Hough Transform �Ķ����
float rho = 2; // Hough �׸����� �Ÿ� ���ش�(�ȼ� ����)
float theta = 1 * CV_PI / 180; // Hough �׸����� ���� ������ ���� ���ش�
float hough_threshold = 15;    // �ּ� ��ǥ ��(Hough �׸��� ���� ������)
float minLineLength = 30; // ������ �����ϴ� �ּ� �ȼ� ��
float maxLineGap = 3;   // ���� ������ �� ���׸�Ʈ ������ �ִ� �ȼ� ����


//Region - of - interest vertices, ���� ���� ���� ���� ��� 
//�̹��� �ϴܿ� �ϴ� �����ڸ��� �ִ� ��ٸ��� ����� ���մϴ�.
float trap_bottom_width = 1;  // ��ٸ��� �ϴ� �����ڸ��� �ʺ�, �̹��� ���� ������� ǥ�õ�
float trap_top_width = 0.9;     // ��ٸ����� ���� �����ڸ��� ���� ����
float trap_height = 0.4;         // �̹��� ������ ������� ǥ�õǴ� ��ٸ��� ����
float offset_h = 0;                // offset�� ������ �ȼ���ŭ �Ʒ� ������� �д�
float offset_w = 0;                // offset�� ������ �ȼ���ŭ �¿�� ��������.
float offset_s = 0;                // offset�� ������ �ȼ���ŭ ���⸦ �д�


//���� ���� ���� 
Scalar lower_white = Scalar(200, 200, 200); //��� ���� (RGB)
Scalar upper_white = Scalar(255, 255, 255);
Scalar lower_yellow = Scalar(10, 100, 100); //����� ���� (HSV)
Scalar upper_yellow = Scalar(40, 255, 255);

//����, ũ�� �ӽ� �����
Mat img_combine_w, img_masked_w[2];
int width_w, height_w;

//ROI
Mat region_of_interest(Mat img_edges, Point* points, int i)
{
	/*
	�̹��� ����ũ�� �����մϴ�.

	�̹����� ������ 'vertice'���� ������ ���������� ���ǵ˴ϴ�.
	�̹����� ������ �κ��� ���������� �����˴ϴ�.
	*/

	Mat img_mask = Mat::zeros(img_edges.rows, img_edges.cols, CV_8UC1);


	Scalar ignore_mask_color = Scalar(255, 255, 255);
	const Point* ppt[1] = { points };
	int npt[] = { 4 };


	//ä��� �������� "filog"�� ���� ���ǵ� ������ ������ �ȼ� ä���
	fillPoly(img_mask, ppt, npt, 1, Scalar(255, 255, 255), LINE_8);


	//����ũ �ȼ��� 0�� �ƴ� ��쿡�� �̹��� ��ȯ
	Mat img_masked;
	bitwise_and(img_edges, img_mask, img_masked);

	img_mask.copyTo(img_masked_w[i]);
	return img_masked;
}




void filter_colors(Mat _img_bgr, Mat& img_filtered)
{
	// ������� ��� �ȼ��� �����ϵ��� �̹��� ���͸�
	UMat img_bgr;
	_img_bgr.copyTo(img_bgr);
	UMat img_hsv, img_combine;
	UMat white_mask, white_image;
	UMat yellow_mask, yellow_image;


	//��� �ȼ� ���͸�
	inRange(img_bgr, lower_white, upper_white, white_mask);
	bitwise_and(img_bgr, img_bgr, white_image, white_mask);


	//����� �ȼ� ���͸�(���� 30)
	cvtColor(img_bgr, img_hsv, COLOR_BGR2HSV);


	inRange(img_hsv, lower_yellow, upper_yellow, yellow_mask);
	bitwise_and(img_bgr, img_bgr, yellow_image, yellow_mask);


	//���� �� �̹����� �����մϴ�.
	addWeighted(white_image, 1.0, yellow_image, 1.0, 0.0, img_combine);

	img_combine.copyTo(img_combine_w);
	img_combine.copyTo(img_filtered);
}



void draw_line(Mat& img_line, vector<Vec4i> lines)
{
	if (lines.size() == 0) return;

	/*
	����: ������ �� ���׸�Ʈ�� ���/�ܻ��Ͽ� ������ ��ü ������ �����Ϸ��� ���
	(���� �� - ��.mp4���� P1_��.mp4�� ǥ�õ� ����� �̵�)
	�� ����� ���������� ����� �� �ֽ��ϴ�.

	�� ���׸�Ʈ�� ����(y2 - y1) / (x2 - x1)�� �����ϴ� �Ͱ� ���� ����� �����غ���,
	� ���׸�Ʈ�� ���� �� �� ������ ���� �Ϻ����� �����Ͻʽÿ�.
	�׷� ���� �� ���� ��ġ�� ���ȭ�ϰ� ���� ��ܰ� �ϴ����� ������ �� �ֽ��ϴ�.

	�� �Լ��� ���� �β��� ���� �׸���. �ش� �̹����� ���� �׷����ϴ�(�̹����� ����).
	���� �������ϰ� �Ϸ��� �� �Լ��� �Ʒ� weighted_img() �Լ��� �����ϴ� ���� ����Ͻʽÿ�.
	*/

	// ������ �߻��� ��� ���� �׸��� ���ʽÿ�.
	bool draw_right = true;
	bool draw_left = true;
	int width = img_line.cols;
	int height = img_line.rows;


	//��� ���� ��� ã��
	//�׷��� abs(���) > slope_threshold(���)�� �ִ� ������ �����Ͻʽÿ�.
	float slope_threshold = 0.5;
	vector<float> slopes;
	vector<Vec4i> new_lines;

	for (int i = 0; i < lines.size(); i++)
	{
		Vec4i line = lines[i];
		int x1 = line[0];
		int y1 = line[1];
		int x2 = line[2];
		int y2 = line[3];


		float slope;
		//��� ���
		if (x2 - x1 == 0) //�ڳ� ���̽�, 0���� ������ ����
			slope = 999.0; //��ǻ� ���� ���
		else
			slope = (y2 - y1) / (float)(x2 - x1);


		//��縦 �������� �� ���͸�
		if (abs(slope) > slope_threshold) {
			slopes.push_back(slope);
			new_lines.push_back(line);
		}
	}



	// �����ʰ� ���� ���� ������ ��Ÿ���� ������_���ΰ� ����_�������� ����
	// ������/���� ���� ������ ����/���� ���⸦ ������ �ϸ� �̹����� ������/���� ���ݿ� �־�� �մϴ�.
	vector<Vec4i> right_lines;
	vector<Vec4i> left_lines;

	for (int i = 0; i < new_lines.size(); i++)
	{

		Vec4i line = new_lines[i];
		float slope = slopes[i];

		int x1 = line[0];
		int y1 = line[1];
		int x2 = line[2];
		int y2 = line[3];


		float cx = width * 0.5; //x �̹��� �߾��� ��ǥ

		if (slope > 0 && x1 > cx && x2 > cx)
			right_lines.push_back(line);
		else if (slope < 0 && x1 < cx && x2 < cx)
			left_lines.push_back(line);
	}


	//���� ȸ�� �м��� �����Ͽ� ������ �� ���� ���� ���ο� ���� ������ ���� ã���ϴ�.
	//���� ����
	double right_lines_x[1000];
	double right_lines_y[1000];
	float right_m, right_b;


	int right_index = 0;
	for (int i = 0; i < right_lines.size(); i++) {

		Vec4i line = right_lines[i];
		int x1 = line[0];
		int y1 = line[1];
		int x2 = line[2];
		int y2 = line[3];

		right_lines_x[right_index] = x1;
		right_lines_y[right_index] = y1;
		right_index++;
		right_lines_x[right_index] = x2;
		right_lines_y[right_index] = y2;
		right_index++;
	}


	if (right_index > 0) {

		double c0, c1, cov00, cov01, cov11, sumsq;
		gsl_fit_linear(right_lines_x, 1, right_lines_y, 1, right_index,
			&c0, &c1, &cov00, &cov01, &cov11, &sumsq);

		//printf("# best fit: Y = %g + %g X\n", c0, c1);

		right_m = c1;
		right_b = c0;
	}
	else {
		right_m = right_b = 1;

		draw_right = false;
	}



	// ���� ����
	double left_lines_x[1000];
	double left_lines_y[1000];
	float left_m, left_b;

	int left_index = 0;
	for (int i = 0; i < left_lines.size(); i++) {

		Vec4i line = left_lines[i];
		int x1 = line[0];
		int y1 = line[1];
		int x2 = line[2];
		int y2 = line[3];

		left_lines_x[left_index] = x1;
		left_lines_y[left_index] = y1;
		left_index++;
		left_lines_x[left_index] = x2;
		left_lines_y[left_index] = y2;
		left_index++;
	}


	if (left_index > 0) {
		double c0, c1, cov00, cov01, cov11, sumsq;
		gsl_fit_linear(left_lines_x, 1, left_lines_y, 1, left_index,
			&c0, &c1, &cov00, &cov01, &cov11, &sumsq);

		//printf("# best fit: Y = %g + %g X\n", c0, c1);

		left_m = c1;
		left_b = c0;
	}
	else {
		left_m = left_b = 1;

		draw_left = false;
	}



	//���� �׸��� �� ���Ǵ� ������ �� ���� ���� ���� 2�� ã��
	//y = m*x + b--> x = (y - b) / m
	int y1 = height;
	int y2 = height * (1 - trap_height);

	float right_x1 = (y1 - right_b) / right_m;
	float right_x2 = (y2 - right_b) / right_m;

	float left_x1 = (y1 - left_b) / left_m;
	float left_x2 = (y2 - left_b) / left_m;

	//���� ������ float���� int�� ��ȯ
	y1 = int(y1);
	y2 = int(y2);
	right_x1 = int(right_x1);
	right_x2 = int(right_x2);
	left_x1 = int(left_x1);
	left_x2 = int(left_x2);


	//�̹����� ������ �� ���� �� �׸��� / �� ������ ���� ��� �� ���
	float slope_r = 0;
	float slope_l = 0;
	float slope_s = 0;
	if (draw_right) {
		line(img_line, Point(right_x1, y1), Point(right_x2, y2), Scalar(255, 200, 0), 10);
		slope_r = atan2(y2 - y1, right_x2 - right_x1) * 180 / PI + 90;
	}
	if (draw_left) {
		line(img_line, Point(left_x1, y1), Point(left_x2, y2), Scalar(255, 200, 200), 10);
		slope_l = atan2(y2 - y1, left_x2 - left_x1) * 180 / PI + 90;
	}
	if (draw_right || draw_left) {
		if (draw_right && draw_left) {
			line(img_line, Point(width_w / 2, height_w), Point((right_x2 + left_x2) / 2, y2), Scalar(100, 100, 100), 10);
			slope_s = atan2(height_w - y2, (right_x2 + left_x2) / 2 - width_w / 2) * 180 / PI;
		}
		else if (draw_right) {
			line(img_line, Point(width_w / 2, height_w), Point((right_x2 + 0) / 2, y2), Scalar(100, 100, 100), 10);
			slope_s = atan2(height_w - y2, (right_x2 + 0) / 2 - width_w / 2) * 180 / PI;
		}
		else if (draw_left) {
			line(img_line, Point(width_w / 2, height_w), Point((width_w + left_x2) / 2, y2), Scalar(100, 100, 100), 10);
			slope_s = atan2(height_w - y2, (width_w + left_x2) / 2 - width_w / 2) * 180 / PI;
		}
	}
	tcp_server(slope_s);
	cout << "�������� ���� : " << slope_l << "\t���������� ���� : " << slope_r << "\n\t\t�߾Ӽ� ���� : " << slope_s << endl;
}

//����ó�� ����
int video_main(string videoname, string filename) {
	char buf[256];
	Mat img_bgr, img_gray, img_edges, img_edge[2], img_hough, img_annotated;

	VideoCapture videoCapture(videoname);

	if (!videoCapture.isOpened())
	{
		cout << videoname << endl;
		cout << "������ ������ ���� �����ϴ�. \n" << endl;

		char a;
		cin >> a;

		return 1;
	}



	videoCapture.read(img_bgr);
	if (img_bgr.empty()) return -1;

	VideoWriter writer;
	int codec = VideoWriter::fourcc('X', 'V', 'I', 'D');  // ���ϴ� �ڵ� ����(��Ÿ�� �� ����� �� �־�� ��)
	double fps = 10.0;                          // ������ ���� ��Ʈ�� �����ӷ�
	writer.open(filename, codec, fps, img_bgr.size(), CV_8UC3);
	// ���� ���� Ȯ��
	if (!writer.isOpened()) {
		cerr << "Could not open the output video file for write\n";
		return -1;
	}


	videoCapture.read(img_bgr);
	int width = img_bgr.size().width;
	int height = img_bgr.size().height;
	width_w = width;
	height_w = height;

	int count = 0;

	while (1)
	{

		//���� ������ �о�� 
		videoCapture.read(img_bgr);
		if (img_bgr.empty()) break;


		//�̸� ���ص� ���, ����� ���� ���� �ִ� �κи� �����ĺ��� ���� ������ 
		Mat img_filtered;
		filter_colors(img_bgr, img_filtered);

		//3. �׷��̽����� �������� ��ȯ�Ͽ� ���� ������ ����
		cvtColor(img_filtered, img_gray, COLOR_BGR2GRAY);
		GaussianBlur(img_gray, img_gray, Size(3, 3), 0, 0);
		Canny(img_gray, img_edges, 50, 150);



		int width = img_filtered.cols;
		int height = img_filtered.rows;


		//���� ������ ������ ������(������� �ٴڿ� �����ϴ� �������� ����)
		Point points[2][4], edges_points[2][4];
		for (int i = 0; i < 2; i++) {
			if (i == 0) {
				points[i][0] = Point(((width * (1 - trap_bottom_width)) / 2) - (width * offset_w), height - (height * offset_h));
				points[i][1] = Point(((width * (1 - trap_top_width)) / 2) - (width * offset_w) + (width * offset_s), height - height * trap_height);
				points[i][2] = Point((width - (width * (1 - trap_top_width)) / 2) - (width * offset_w) + (width * offset_s), height - height * trap_height);
				points[i][3] = Point((width - (width * (1 - trap_bottom_width)) / 2) - (width * offset_w), height - (height * offset_h));
				for (int j = 0; j < 4; j++) {
					edges_points[i][j] = points[i][j];
				}
			}
			else {
				points[i][0] = Point(((width * (1 - trap_bottom_width)) / 2) + (width * offset_w), height - (height * offset_h));
				points[i][1] = Point(((width * (1 - trap_top_width)) / 2) + (width * offset_w) - (width * offset_s), height - height * trap_height);
				points[i][2] = Point((width - (width * (1 - trap_top_width)) / 2) + (width * offset_w) - (width * offset_s), height - height * trap_height);
				points[i][3] = Point((width - (width * (1 - trap_bottom_width)) / 2) + (width * offset_w), height - (height * offset_h));
				for (int j = 0; j < 4; j++) {
					edges_points[i][j] = points[i][j];
				}
			}
			img_edge[i] = region_of_interest(img_edges, points[i], i);
		}
		addWeighted(img_edge[0], 1.0, img_edge[1], 1.0, 0.0, img_edges);

		//ROI ���� ����ġ
		Mat edges_bgr;
		img_bgr.copyTo(edges_bgr);
		const Point* edges_temp[2] = { edges_points[0], edges_points[1] };
		int npt[2] = { 4, 4 };
		polylines(edges_bgr, edges_temp, npt, 2, 1, CV_RGB(0, 255, 0));


		UMat uImage_edges;
		img_edges.copyTo(uImage_edges);

		//���� ������ ����(�� ������ ������ǥ�� ����ǥ�� �����)
		vector<Vec4i> lines;
		HoughLinesP(uImage_edges, lines, rho, theta, hough_threshold, minLineLength, maxLineGap);




		//5������ ������ �����������κ��� �¿� ������ ���� ���ɼ��ִ� �����鸸 ���� �̾Ƽ�
		//�¿� ���� �ϳ��� ������ ����� (Linear Least-Squares Fitting)
		Mat img_line = Mat::zeros(img_bgr.rows, img_bgr.cols, CV_8UC3);
		draw_line(img_line, lines);




		//���� ���� 6���� ������ ���� ������ 
		addWeighted(img_bgr, 0.8, img_line, 1.0, 0.0, img_annotated);


		//����� ������ ���Ϸ� ��� 
		writer << img_annotated;

		count++;
		if (count == 10) imwrite("D:\\OneDrive - ���ִ��б�\\��������_������Ʈ\\curb_test.jpg", img_annotated);

		//����� ȭ�鿡 ������ 
		Mat img_result;
		Mat img_mask;
		addWeighted(img_masked_w[0], 1.0, img_masked_w[1], 1.0, 0.0, img_mask);
		resize(img_combine_w, img_combine_w, Size(width * SC, height * SC));
		resize(edges_bgr, edges_bgr, Size(width * SC, height * SC));
		resize(img_mask, img_mask, Size(width * SC, height * SC));
		hconcat(img_combine_w, edges_bgr, img_result);
		imshow("�������� / ����ŷ ����", img_result);
		//imshow("����ũ ����", img_mask);
		resize(img_annotated, img_annotated, Size(width * SC, height * SC));
		resize(img_edges, img_edges, Size(width * SC, height * SC));
		cvtColor(img_edges, img_edges, COLOR_GRAY2BGR);
		hconcat(img_edges, img_annotated, img_result);
		imshow("���� ����", img_result);

		//Ŭ���̾�Ʈ ��� ����
		/*char cBuffer[PACKET_SIZE] = {};
		recv(hClient, cBuffer, PACKET_SIZE, 0);
		printf("Recv Msg : %s\n\n", cBuffer);*/

		if (waitKey(1) == 27) break; //ESCŰ ������ ����  
	}
}
