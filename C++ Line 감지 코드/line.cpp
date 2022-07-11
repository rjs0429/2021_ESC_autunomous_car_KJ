#include "function.h"

#define PI 3.1415926535
#define SC 0.3					//���� ǥ�� ������
#define LA 15					//���� ���� ��� ����

using namespace cv;
using  std::vector;
using  std::cin;
using  std::cerr;

//Hough Transform �Ķ����
float rho = 1;					// Hough �׸����� �Ÿ� ���ش�(�ȼ� ����)
float theta = 1 * CV_PI / 180;	// Hough �׸����� ���� ������ ���� ���ش�
float hough_threshold = 5;		// �ּ� ��ǥ ��(Hough �׸��� ���� ������)
float minLineLength = 5;		// ������ �����ϴ� �ּ� �ȼ� ��
float maxLineGap = 3;			// ���� ������ �� ���׸�Ʈ ������ �ִ� �ȼ� ����


//Region - of - interest vertices, ���� ���� ���� ���� ��� 
//�̹��� �ϴܿ� �ϴ� �����ڸ��� �ִ� ��ٸ��� ����� ���մϴ�.
float trap_bottom_width = 0.5;		// ��ٸ��� �ϴ� �����ڸ��� �ʺ�, �̹��� ���� ������� ǥ�õ�
float trap_top_width = 0.15;		// ��ٸ����� ���� �����ڸ��� ���� ����
float trap_height = 0.37;			// �̹��� ������ ������� ǥ�õǴ� ��ٸ��� ����
float offset_h = 0.13;				// offset�� ������ �ȼ���ŭ �Ʒ� ������� �д�
float offset_mid = 0.06;			// ������ ��ġ��ŭ ���� ������ ��� �Ÿ��� ���Ѵ�
float offset_end = 2.7;				// ������ ��ġ��ŭ ���� ���� �Ÿ��� ���Ѵ�
float offset_wide = 0.1;			// ������ ��ġ��ŭ ���� �����Ұ� �� ������ �������ŭ roi�� ������
int line_check = 5;					// ������ �����Ӹ�ŭ ���� ���� �Ұ��� �������� Ȯ��


//���� ���� ���� 
Scalar lower_white = Scalar(0, 0, 160); //��� ���� (HSV)
Scalar upper_white = Scalar(179, 255, 255);
Scalar lower_yellow = Scalar(10, 100, 100); //����� ���� (HSV)
Scalar upper_yellow = Scalar(40, 255, 255);

//����, ũ�� �ӽ� �����
UMat img_combine_w, img_masked_w;
//int width_w, height_w;

//���� ���� �� dynamic roi �����
int line_avg_r, line_avg_l;		//���� ���� ��� ����
int right_drp_avg[LA][2], left_drp_avg[LA][2];//dynamic roi point avg
int right_avg[2], left_avg[2];//dynamic roi point avg
boolean right_w, left_w;		//���� ���� ����
Point right_drp[4], left_drp[4];//dynamic roi point
float check_roi = 0.03;			//dynamic roi �� ����(���� ���)

//ROI
UMat region_of_interest(UMat img_edges, Point* points) {
	/*
	�̹��� ����ũ�� �����մϴ�.

	�̹����� ������ 'vertice'���� ������ ���������� ���ǵ˴ϴ�.
	�̹����� ������ �κ��� ���������� �����˴ϴ�.
	*/

	UMat img_mask = UMat::zeros(img_edges.rows, img_edges.cols, CV_8UC1);


	Scalar ignore_mask_color = Scalar(255, 255, 255);
	const Point* ppt[1] = { points };
	int npt[] = { 4 };


	//ä��� �������� "filog"�� ���� ���ǵ� ������ ������ �ȼ� ä���
	fillPoly(img_mask, ppt, npt, 1, Scalar(255, 255, 255), LINE_8);


	//����ũ �ȼ��� 0�� �ƴ� ��쿡�� �̹��� ��ȯ
	UMat img_masked;
	bitwise_and(img_edges, img_mask, img_masked);

	img_mask.copyTo(img_masked_w);
	return img_masked;
}

void filter_colors(UMat _img_bgr, UMat& img_filtered)
{
	// ������� ��� �ȼ��� �����ϵ��� �̹��� ���͸�
	UMat img_bgr;
	_img_bgr.copyTo(img_bgr);
	UMat img_hsv, img_hsv2, img_combine;
	UMat white_mask, white_image;
	UMat yellow_mask, yellow_image;


	//��� �ȼ� ���͸�
	cvtColor(img_bgr, img_hsv2, COLOR_BGR2HSV);

	inRange(img_hsv2, lower_white, upper_white, white_mask);
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

//�� ���� �̿��Ͽ� �Լ��� �����, y�� ���Խ� x�� ����
int point_calculation_x(Point po1, Point po2, int focus) {
	float a, b;
	float x1 = po1.x;
	float y1 = po1.y;
	float x2 = po2.x;
	float y2 = po2.y;

	a = (y1 - y2) / (x1 - x2);
	b = y1 - (a * x1);

	return int((focus - b) / a);
}

//�� ���� �̿��Ͽ� �Լ��� �����, x�� ���Խ� y�� ����
int point_calculation_y(Point po1, Point po2, int focus) {
	float a, b;
	float x1 = po1.x;
	float y1 = po1.y;
	float x2 = po2.x;
	float y2 = po2.y;

	a = (y1 - y2) / (x1 - x2);
	b = y1 - (a * x1);

	return int(a * focus + b);
}

//������ ������ ����� �ʰ� ����
float pont_control(float* point_y, float m, float b, float width) {
	float result = (*point_y - b) / m;
	if (result < 0)
		*point_y = b;
	else if (result > width)
		*point_y = m * width + b;

	result = (*point_y - b) / m;
	return result;
}

//���� ��Ż ���
int steering(float min_point, float max_point, float value) {
	float percent = (value - min_point) / (max_point - min_point);
	float steering_value = 255 * percent;
	if (steering_value >= 255)
		steering_value = 255;
	else if (steering_value <= 0)
		steering_value = 0;

	return int(steering_value);
}

//dynamic_roi ����Ʈ ��� / �ֱ� ����� ����Ʈ ��� ���ϱ�
int dynamic_roi_point(Point po1, Point po2, int width, int line_avg, int drp_avg[LA][2], Point roi_point[4], int lr) {
	int avg[2] = { 0, 0 };
	int sum[2] = { 0, 0 };

	if (line_avg == -1) {
		for (int i = 0; i < LA; i++) {
			drp_avg[i][0] = 0;
			drp_avg[i][1] = 0;
		}
		avg[0] = po1.x;
		avg[1] = po2.x;
	}
	else {
		if (line_avg == LA)
			line_avg = 0;

		drp_avg[line_avg][0] = po1.x;
		drp_avg[line_avg][1] = po2.x;
		for (int i = 0; i < line_avg + 1; i++) {
			sum[0] += drp_avg[i][0];
			sum[1] += drp_avg[i][1];
		}
		for (int i = 0; i < 2; i++) {
			avg[i] = sum[i] / (line_avg + 1);
		}
	}

	Point2f mid_point((avg[0] + avg[1]) / 2, (po1.y + po2.y) / 2), pts[4];
	float length = sqrt(pow(avg[0] - avg[1], 2) + pow(po1.y - po2.y, 2));
	float angle = atan2(po1.y - po2.y, avg[0] - avg[1]) * 180 / PI;
	Size2f size(length, width * check_roi);
	RotatedRect rot_rect(mid_point, size, angle);
	rot_rect.points(pts);

	for (int i = 0; i < 4; i++)
		roi_point[i] = pts[i];

	if (lr == 1) {
		for (int i = 0; i < 2; i++)
			right_avg[i] = avg[i];
	}
	else {
		for (int i = 0; i < 2; i++)
			left_avg[i] = avg[i];
	}



	line_avg++;
	return line_avg;
}

//����Ʈ�� dynamic_roi�ȿ� �ִ��� ����
boolean point_is_inside(Point po1, Point po2, Point roi_point[4]) {
	int crosses = 0;
	int crosses2 = 0;
	boolean check = false;

	for (int i = 0; i < 4; i++) {
		int j = (i + 1) % 4;
		if ((roi_point[i].y > po1.y) != (roi_point[j].y > po1.y)) {
			float atX = (roi_point[j].x - roi_point[i].x) * (po1.y - roi_point[i].y) / (roi_point[j].y - roi_point[i].y) + roi_point[i].x;
			if (po1.x < atX)
				crosses++;
		}
	}
	for (int i = 0; i < 4; i++) {
		int j = (i + 1) % 4;
		if ((roi_point[i].y > po2.y) != (roi_point[j].y > po2.y)) {
			float atX = (roi_point[j].x - roi_point[i].x) * (po2.y - roi_point[i].y) / (roi_point[j].y - roi_point[i].y) + roi_point[i].x;
			if (po2.x < atX)
				crosses2++;
		}
	}

	if ((crosses % 2 == 0) || (crosses2 % 2 == 0))
		check = false;
	else
		check = true;

	return check;
}

//���� ȸ�� �м��� �����Ͽ� ������ �� ���� ���� ���ο� ���� ������ ���� ã���ϴ�.
boolean linear_regression(float* m, float* b, vector<Vec4i> lines) {
	boolean draw_line = true;
	double lines_x[1000];
	double lines_y[1000];
	int index = 0;

	for (int i = 0; i < lines.size(); i++) {

		Vec4i line = lines[i];
		int x1 = line[0];
		int y1 = line[1];
		int x2 = line[2];
		int y2 = line[3];

		lines_x[index] = x1;
		lines_y[index] = y1;
		index++;
		lines_x[index] = x2;
		lines_y[index] = y2;
		index++;
	}


	if (index > 0) {

		double c0, c1, cov00, cov01, cov11, sumsq;
		gsl_fit_linear(lines_x, 1, lines_y, 1, index,
			&c0, &c1, &cov00, &cov01, &cov11, &sumsq);

		*m = c1;
		*b = c0;
	}
	else {
		*m = *b = 1;

		draw_line = false;
	}

	return draw_line;
}

void draw_line(UMat& img_line, vector<Vec4i> lines)
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

		Point po1(x1, y1), po2(x2, y2);

		float cx = width * 0.5; //x �̹��� �߾��� ��ǥ

		if (right_w && point_is_inside(po1, po2, right_drp))
			right_lines.push_back(line);
		else if (left_w && point_is_inside(po1, po2, left_drp))
			left_lines.push_back(line);
		else if (!left_w && slope < 0 && x1 < cx && x2 < cx)
			left_lines.push_back(line);
		else if (!right_w && slope > 0 && x1 > cx && x2 > cx)
			right_lines.push_back(line);
	}


	//���� ȸ�� �м��� �����Ͽ� ������ �� ���� ���� ���ο� ���� ������ ���� ã���ϴ�.
	//���� ����
	float right_m, right_b;
	draw_right = linear_regression(&right_m, &right_b, right_lines);

	// ���� ����
	float left_m, left_b;
	draw_left = linear_regression(&left_m, &left_b, left_lines);


	//���� �׸��� �� ���Ǵ� ������ �� ���� ���� ���� 2�� ã��
	//y = m*x + b --> x = (y - b) / m
	float y1 = height - (height * offset_h);
	float y2 = height * (1 - trap_height);
	float right_y1 = y1;
	float right_y2 = y2;
	float left_y1 = y1;
	float left_y2 = y2;

	float right_x1 = pont_control(&right_y1, right_m, right_b, width);
	float right_x2 = pont_control(&right_y2, right_m, right_b, width);

	float left_x1 = pont_control(&left_y1, left_m, left_b, width);
	float left_x2 = pont_control(&left_y2, left_m, left_b, width);

	//���� ������ float���� int�� ��ȯ
	right_y1 = int(right_y1);
	right_y2 = int(right_y2);
	left_y1 = int(left_y1);
	left_y2 = int(left_y2);
	right_x1 = int(right_x1);
	right_x2 = int(right_x2);
	left_x1 = int(left_x1);
	left_x2 = int(left_x2);

	int color;
	float cx = width * 0.5; //x �̹��� �߾��� ��ǥ
	//�� �� ���� �׸���
	if (draw_right && (right_x1 > cx - (width * offset_mid) && right_x2 > cx - (width * offset_mid))) {
		color = steering(width - width / offset_end, (width / 2) + (width * offset_mid), (right_x1 + right_x2) / 2);
		line_avg_r = dynamic_roi_point(Point(right_x1, right_y1), Point(right_x2, right_y2), width, line_avg_r, right_drp_avg, right_drp, 1);
		line(img_line, Point(right_avg[0], right_y1), Point(right_avg[1], right_y2), Scalar(255 - color * 3, 255 - color * 2, color), 30);
		right_w = true;
	}
	else if (draw_right && (right_x1 < cx || right_x2 < cx)) {
		line_avg_r = -1;
		right_w = false;
	}
	else {
		line_avg_r = -1;
		right_w = false;
	}
	if (draw_left && (left_x1 < cx + (width * offset_mid) && left_x2 < cx + (width * offset_mid))) {
		color = steering(width / offset_end, (width / 2) - (width * offset_mid), (left_x1 + left_x2) / 2);
		line_avg_l = dynamic_roi_point(Point(left_x1, left_y1), Point(left_x2, left_y2), width, line_avg_l, left_drp_avg, left_drp, 0);
		line(img_line, Point(left_avg[0], left_y1), Point(left_avg[1], left_y2), Scalar(255 - color * 3, 255 - color * 2, color), 30);
		left_w = true;
	}
	else if (draw_left && (left_x1 > cx + (width * offset_mid) || left_x2 > cx + (width * offset_mid))) {
		line_avg_l = -1;
		left_w = false;
	}
	else {
		line_avg_l = -1;
		left_w = false;
	}

}

//����ó�� ����
int video_main(string videoname, string filename[]) {
	//Py_Initialize();
	PyObject* pName, * pModule, * pFunc, * pValue, * pResult, * pArgs, * pDict;

	int line_check_temp;
	UMat img_bgr, img_gray, img_edges, img_edge, img_hough, img_annotated;

	right_w = false;
	left_w = false;
	line_avg_r = -1;
	line_avg_l = -1;
	line_check_temp = 0;

	VideoCapture videoCapture(videoname);

	if (!videoCapture.isOpened())
	{
		cout << "������ ������ ���� �����ϴ�. \n" << endl;

		char a;
		cin >> a;

		return 1;
	}



	videoCapture.read(img_bgr);
	if (img_bgr.empty()) return -1;

	VideoWriter writer[2];
	int codec = VideoWriter::fourcc('X', 'V', 'I', 'D');  // ���ϴ� �ڵ� ����(��Ÿ�� �� ����� �� �־�� ��)
	double fps = 29.97;									// ������ ���� ��Ʈ�� �����ӷ�
	for (int i = 0; i < 2; i++) {
		writer[i].open(filename[i], codec, fps, img_bgr.size(), CV_8UC3);
		// ���� ���� Ȯ��
		if (!writer[i].isOpened()) {
			cerr << "Could not open the output video file for write\n";
			return -1;
		}
	}


	videoCapture.read(img_bgr);
	int width = img_bgr.size().width;
	int height = img_bgr.size().height;

	int count = 0;

	while (1)
	{
		//���� ������ �о�� 
		videoCapture.read(img_bgr);
		if (img_bgr.empty()) break;

		//�̸� ���ص� ���, ����� ���� ���� �ִ� �κи� �����ĺ��� ���� ������ 
		UMat img_filtered;
		filter_colors(img_bgr, img_filtered);

		//�׷��̽����� �������� ��ȯ�Ͽ� ���� ������ ����
		cvtColor(img_filtered, img_gray, COLOR_BGR2GRAY);
		GaussianBlur(img_gray, img_gray, Size(3, 3), 0, 0);
		Canny(img_gray, img_edges, 50, 150);

		int width = img_filtered.cols;
		int height = img_filtered.rows;

		//���̽����� �ڵ� ����
		/*UMat bgr_temp;
		img_bgr.copyTo(bgr_temp);
		Mat imgs = bgr_temp.getMat(ACCESS_READ);

		pName = PyUnicode_FromString("opencv-yolo");
		pModule = PyImport_Import(pName);
		pDict = PyModule_GetDict(pModule);
		import_array();
		npy_intp dimensions[3] = { imgs.rows, imgs.cols, imgs.channels() };
		pValue = PyArray_SimpleNewFromData(imgs.dims + 1, (npy_intp*)&dimensions, NPY_UINT8, imgs.data);
		pArgs = PyTuple_New(1);
		PyTuple_SetItem(pArgs, 0, pValue);
		pFunc = PyDict_GetItemString(pDict, (char*)"yolo_img");
		if (PyCallable_Check(pFunc)){
			PyObject_CallObject(pFunc, pArgs);
		}
		else{
			cout << "Function is not callable !" << endl;
		}*/

		//���� ������ ������ ������(������� �ٴڿ� �����ϴ� �������� ����)
		Point points[4], edges_points[4];
		if ((!right_w || !left_w) && line_check_temp == line_check) {
			points[0] = Point(((width * (1 - trap_bottom_width)) / 2) - (width * offset_wide), height - (height * offset_h));
			points[1] = Point(((width * (1 - trap_top_width)) / 2) - (width * offset_wide), height - height * trap_height);
			points[2] = Point((width - (width * (1 - trap_top_width)) / 2) + (width * offset_wide), height - height * trap_height);
			points[3] = Point((width - (width * (1 - trap_bottom_width)) / 2) + (width * offset_wide), height - (height * offset_h));
		}
		else if ((!right_w || !left_w) && line_check_temp != line_check) {
			points[0] = Point(((width * (1 - trap_bottom_width)) / 2), height - (height * offset_h));
			points[1] = Point(((width * (1 - trap_top_width)) / 2), height - height * trap_height);
			points[2] = Point((width - (width * (1 - trap_top_width)) / 2), height - height * trap_height);
			points[3] = Point((width - (width * (1 - trap_bottom_width)) / 2), height - (height * offset_h));
			line_check_temp++;
		}
		else {
			points[0] = Point(width, height - (height * offset_h));
			points[1] = Point(width, height - height * trap_height);
			points[2] = Point(0, height - height * trap_height);
			points[3] = Point(0, height - (height * offset_h));
			line_check_temp = 0;
		}
		for (int j = 0; j < 4; j++) {
			edges_points[j] = points[j];
		}
		img_edges = region_of_interest(img_edges, points);


		//ROI ���� ����ġ
		UMat edges_bgr;
		img_bgr.copyTo(edges_bgr);
		const Point* edges_temp[1] = { edges_points };
		int npt[2] = { 4, 4 };
		polylines(edges_bgr, edges_temp, npt, 1, 1, CV_RGB(0, 255, 0), 10);

		if (left_w) {
			const Point* edges_temp_l[1] = { left_drp };
			int npt_l[1] = { 4 };
			polylines(edges_bgr, edges_temp_l, npt_l, 1, 1, CV_RGB(0, 255, 0), 10);
		}
		if (right_w) {
			const Point* edges_temp_r[1] = { right_drp };
			int npt_r[1] = { 4 };
			polylines(edges_bgr, edges_temp_r, npt_r, 1, 1, CV_RGB(0, 255, 0), 10);
		}


		UMat uImage_edges;
		img_edges.copyTo(uImage_edges);

		//���� ������ ����(�� ������ ������ǥ�� ����ǥ�� �����)
		vector<Vec4i> lines;
		HoughLinesP(uImage_edges, lines, rho, theta, hough_threshold, minLineLength, maxLineGap);




		//������ �����������κ��� �¿� ������ ���� ���ɼ��ִ� �����鸸 ���� �̾Ƽ�
		//�¿� ���� �ϳ��� ������ ����� (Linear Least-Squares Fitting)
		UMat img_line = UMat::zeros(img_bgr.rows, img_bgr.cols, CV_8UC3);
		draw_line(img_line, lines);




		//���� ���� 6���� ������ ���� ������ 
		addWeighted(img_bgr, 1.0, img_line, 1.0, 0.0, img_annotated);


		//����� ������ ���Ϸ� ��� 
		writer[0] << img_annotated;

		count++;
		if (count == 10) imwrite("img_annota1ted.jpg", img_annotated);

		//����� ȭ�鿡 ������ 
		UMat img_result[3];
		resize(img_combine_w, img_combine_w, Size(width * SC, height * SC));
		resize(edges_bgr, edges_bgr, Size(width * SC, height * SC));
		hconcat(img_combine_w, edges_bgr, img_result[0]);
		resize(img_annotated, img_annotated, Size(width * SC, height * SC));
		resize(img_edges, img_edges, Size(width * SC, height * SC));
		cvtColor(img_edges, img_edges, COLOR_GRAY2BGR);
		hconcat(img_edges, img_annotated, img_result[1]);
		vconcat(img_result[0], img_result[1], img_result[2]);
		imshow("���� ����", img_result[2]);
		resize(img_result[2], img_result[2], Size(width, height));
		writer[1] << img_result[2];

		if (waitKey(1) == 27) break; //ESCŰ ������ ����  
	}
	//Py_Finalize();
}
