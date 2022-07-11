#include "function.h"

#define PI 3.1415926535
#define SC 0.3					//영상 표시 스케일
#define LA 15					//차선 정보 평균 개수

using namespace cv;
using  std::vector;
using  std::cin;
using  std::cerr;

//Hough Transform 파라미터
float rho = 1;					// Hough 그리드의 거리 분해능(픽셀 단위)
float theta = 1 * CV_PI / 180;	// Hough 그리드의 라디안 단위의 각도 분해능
float hough_threshold = 5;		// 최소 투표 수(Hough 그리드 셀의 교차점)
float minLineLength = 5;		// 라인을 구성하는 최소 픽셀 수
float maxLineGap = 3;			// 연결 가능한 선 세그먼트 사이의 최대 픽셀 간격


//Region - of - interest vertices, 관심 영역 범위 계산시 사용 
//이미지 하단에 하단 가장자리가 있는 사다리꼴 모양을 원합니다.
float trap_bottom_width = 0.5;		// 사다리꼴 하단 가장자리의 너비, 이미지 폭의 백분율로 표시됨
float trap_top_width = 0.15;		// 사다리꼴의 위쪽 가장자리에 대해 편집
float trap_height = 0.37;			// 이미지 높이의 백분율로 표시되는 사다리꼴 높이
float offset_h = 0.13;				// offset에 지정한 픽셀만큼 아래 빈공간을 둔다
float offset_mid = 0.06;			// 지정한 수치만큼 라인 근접시 경고 거리를 정한다
float offset_end = 2.7;				// 지정한 수치만큼 라인 적정 거리를 정한다
float offset_wide = 0.1;			// 지정한 수치만큼 라인 감지불가 시 넓이의 백분율만큼 roi를 넓힌다
int line_check = 5;					// 지정한 프레임만큼 라인 감지 불가시 감지범위 확장


//차선 색깔 범위 
Scalar lower_white = Scalar(0, 0, 160); //흰색 차선 (HSV)
Scalar upper_white = Scalar(179, 255, 255);
Scalar lower_yellow = Scalar(10, 100, 100); //노란색 차선 (HSV)
Scalar upper_yellow = Scalar(40, 255, 255);

//영상, 크기 임시 저장소
UMat img_combine_w, img_masked_w;
//int width_w, height_w;

//차선 정보 및 dynamic roi 저장소
int line_avg_r, line_avg_l;		//차선 정보 평균 개수
int right_drp_avg[LA][2], left_drp_avg[LA][2];//dynamic roi point avg
int right_avg[2], left_avg[2];//dynamic roi point avg
boolean right_w, left_w;		//차선 정보 유무
Point right_drp[4], left_drp[4];//dynamic roi point
float check_roi = 0.03;			//dynamic roi 폭 비율(영상 대비)

//ROI
UMat region_of_interest(UMat img_edges, Point* points) {
	/*
	이미지 마스크를 적용합니다.

	이미지의 영역만 'vertice'에서 형성된 폴리곤으로 정의됩니다.
	이미지의 나머지 부분이 검은색으로 설정됩니다.
	*/

	UMat img_mask = UMat::zeros(img_edges.rows, img_edges.cols, CV_8UC1);


	Scalar ignore_mask_color = Scalar(255, 255, 255);
	const Point* ppt[1] = { points };
	int npt[] = { 4 };


	//채우기 색상으로 "filog"에 의해 정의된 폴리곤 내부의 픽셀 채우기
	fillPoly(img_mask, ppt, npt, 1, Scalar(255, 255, 255), LINE_8);


	//마스크 픽셀이 0이 아닌 경우에만 이미지 반환
	UMat img_masked;
	bitwise_and(img_edges, img_mask, img_masked);

	img_mask.copyTo(img_masked_w);
	return img_masked;
}

void filter_colors(UMat _img_bgr, UMat& img_filtered)
{
	// 노란색과 흰색 픽셀만 포함하도록 이미지 필터링
	UMat img_bgr;
	_img_bgr.copyTo(img_bgr);
	UMat img_hsv, img_hsv2, img_combine;
	UMat white_mask, white_image;
	UMat yellow_mask, yellow_image;


	//흰색 픽셀 필터링
	cvtColor(img_bgr, img_hsv2, COLOR_BGR2HSV);

	inRange(img_hsv2, lower_white, upper_white, white_mask);
	bitwise_and(img_bgr, img_bgr, white_image, white_mask);


	//노란색 픽셀 필터링(색조 30)
	cvtColor(img_bgr, img_hsv, COLOR_BGR2HSV);


	inRange(img_hsv, lower_yellow, upper_yellow, yellow_mask);
	bitwise_and(img_bgr, img_bgr, yellow_image, yellow_mask);


	//위의 두 이미지를 결합합니다.
	addWeighted(white_image, 1.0, yellow_image, 1.0, 0.0, img_combine);

	img_combine.copyTo(img_combine_w);
	img_combine.copyTo(img_filtered);
}

//두 점을 이용하여 함수를 만들고, y축 대입시 x축 산출
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

//두 점을 이용하여 함수를 만들고, x축 대입시 y축 산출
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

//끝점이 영상을 벗어나지 않게 조절
float pont_control(float* point_y, float m, float b, float width) {
	float result = (*point_y - b) / m;
	if (result < 0)
		*point_y = b;
	else if (result > width)
		*point_y = m * width + b;

	result = (*point_y - b) / m;
	return result;
}

//차선 이탈 경고
int steering(float min_point, float max_point, float value) {
	float percent = (value - min_point) / (max_point - min_point);
	float steering_value = 255 * percent;
	if (steering_value >= 255)
		steering_value = 255;
	else if (steering_value <= 0)
		steering_value = 0;

	return int(steering_value);
}

//dynamic_roi 포인트 계산 / 최근 검출된 포인트 평균 구하기
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

//포인트가 dynamic_roi안에 있는지 검출
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

//선형 회귀 분석을 실행하여 오른쪽 및 왼쪽 차선 라인에 가장 적합한 선을 찾습니다.
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
	참고: 감지한 선 세그먼트를 평균/외삽하여 레인의 전체 범위를 매핑하려는 경우
	(원시 선 - 예.mp4에서 P1_예.mp4에 표시된 결과로 이동)
	이 기능을 시작점으로 사용할 수 있습니다.

	선 세그먼트를 기울기(y2 - y1) / (x2 - x1)로 구분하는 것과 같은 방법을 생각해보고,
	어떤 세그먼트가 왼쪽 선 대 오른쪽 선의 일부인지 결정하십시오.
	그런 다음 각 선의 위치를 평균화하고 차선 상단과 하단으로 추정할 수 있습니다.

	이 함수는 색과 두께로 선을 그린다. 해당 이미지에 선이 그려집니다(이미지를 변형).
	선을 반투명하게 하려면 이 함수를 아래 weighted_img() 함수와 결합하는 것을 고려하십시오.
	*/

	// 오류가 발생할 경우 선을 그리지 마십시오.
	bool draw_right = true;
	bool draw_left = true;
	int width = img_line.cols;
	int height = img_line.rows;


	//모든 선의 경사 찾기
	//그러나 abs(경사) > slope_threshold(경사)가 있는 선에만 주의하십시오.
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
		//경사 계산
		if (x2 - x1 == 0) //코너 케이스, 0으로 나누기 피함
			slope = 999.0; //사실상 무한 경사
		else
			slope = (y2 - y1) / (float)(x2 - x1);

		//경사를 기준으로 선 필터링
		if (abs(slope) > slope_threshold) {
			slopes.push_back(slope);
			new_lines.push_back(line);
		}
	}

	// 오른쪽과 왼쪽 차선 라인을 나타내는 오른쪽_라인과 왼쪽_라인으로 구분
	// 오른쪽/왼쪽 차선 라인은 양의/음의 기울기를 가져야 하며 이미지의 오른쪽/왼쪽 절반에 있어야 합니다.
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

		float cx = width * 0.5; //x 이미지 중앙의 좌표

		if (right_w && point_is_inside(po1, po2, right_drp))
			right_lines.push_back(line);
		else if (left_w && point_is_inside(po1, po2, left_drp))
			left_lines.push_back(line);
		else if (!left_w && slope < 0 && x1 < cx && x2 < cx)
			left_lines.push_back(line);
		else if (!right_w && slope > 0 && x1 > cx && x2 > cx)
			right_lines.push_back(line);
	}


	//선형 회귀 분석을 실행하여 오른쪽 및 왼쪽 차선 라인에 가장 적합한 선을 찾습니다.
	//우측 차선
	float right_m, right_b;
	draw_right = linear_regression(&right_m, &right_b, right_lines);

	// 왼쪽 차선
	float left_m, left_b;
	draw_left = linear_regression(&left_m, &left_b, left_lines);


	//선을 그리는 데 사용되는 오른쪽 및 왼쪽 선의 끝점 2개 찾기
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

	//계산된 끝점을 float에서 int로 변환
	right_y1 = int(right_y1);
	right_y2 = int(right_y2);
	left_y1 = int(left_y1);
	left_y2 = int(left_y2);
	right_x1 = int(right_x1);
	right_x2 = int(right_x2);
	left_x1 = int(left_x1);
	left_x2 = int(left_x2);

	int color;
	float cx = width * 0.5; //x 이미지 중앙의 좌표
	//각 측 차선 그리기
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

//영상처리 메인
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
		cout << "동영상 파일을 열수 없습니다. \n" << endl;

		char a;
		cin >> a;

		return 1;
	}



	videoCapture.read(img_bgr);
	if (img_bgr.empty()) return -1;

	VideoWriter writer[2];
	int codec = VideoWriter::fourcc('X', 'V', 'I', 'D');  // 원하는 코덱 선택(런타임 시 사용할 수 있어야 함)
	double fps = 29.97;									// 생성된 비디오 스트림 프레임률
	for (int i = 0; i < 2; i++) {
		writer[i].open(filename[i], codec, fps, img_bgr.size(), CV_8UC3);
		// 성공 여부 확인
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
		//원본 영상을 읽어옴 
		videoCapture.read(img_bgr);
		if (img_bgr.empty()) break;

		//미리 정해둔 흰색, 노란색 범위 내에 있는 부분만 차선후보로 따로 저장함 
		UMat img_filtered;
		filter_colors(img_bgr, img_filtered);

		//그레이스케일 영상으로 변환하여 에지 성분을 추출
		cvtColor(img_filtered, img_gray, COLOR_BGR2GRAY);
		GaussianBlur(img_gray, img_gray, Size(3, 3), 0, 0);
		Canny(img_gray, img_edges, 50, 150);

		int width = img_filtered.cols;
		int height = img_filtered.rows;

		//파이썬으로 코드 전달
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

		//차선 검출할 영역을 제한함(진행방향 바닥에 존재하는 차선으로 한정)
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


		//ROI 영역 스케치
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

		//직선 성분을 추출(각 직선의 시작좌표와 끝좌표를 계산함)
		vector<Vec4i> lines;
		HoughLinesP(uImage_edges, lines, rho, theta, hough_threshold, minLineLength, maxLineGap);




		//추출한 직선성분으로부터 좌우 차선에 있을 가능성있는 직선들만 따로 뽑아서
		//좌우 각각 하나씩 직선을 계산함 (Linear Least-Squares Fitting)
		UMat img_line = UMat::zeros(img_bgr.rows, img_bgr.cols, CV_8UC3);
		draw_line(img_line, lines);




		//원본 영상에 6번의 직선을 같이 보여줌 
		addWeighted(img_bgr, 1.0, img_line, 1.0, 0.0, img_annotated);


		//결과를 동영상 파일로 기록 
		writer[0] << img_annotated;

		count++;
		if (count == 10) imwrite("img_annota1ted.jpg", img_annotated);

		//결과를 화면에 보여줌 
		UMat img_result[3];
		resize(img_combine_w, img_combine_w, Size(width * SC, height * SC));
		resize(edges_bgr, edges_bgr, Size(width * SC, height * SC));
		hconcat(img_combine_w, edges_bgr, img_result[0]);
		resize(img_annotated, img_annotated, Size(width * SC, height * SC));
		resize(img_edges, img_edges, Size(width * SC, height * SC));
		cvtColor(img_edges, img_edges, COLOR_GRAY2BGR);
		hconcat(img_edges, img_annotated, img_result[1]);
		vconcat(img_result[0], img_result[1], img_result[2]);
		imshow("차선 영상", img_result[2]);
		resize(img_result[2], img_result[2], Size(width, height));
		writer[1] << img_result[2];

		if (waitKey(1) == 27) break; //ESC키 누르면 종료  
	}
	//Py_Finalize();
}
