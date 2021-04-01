#include "function.h" 

#define PI 3.1415926535

using namespace cv;

int point_calculation(Point po1, Point po2, int pint) {
	float a, b;
	float x1 = po1.x;
	float y1 = po1.y;
	float x2 = po2.x;
	float y2 = po2.y;

	a = (y1 - y2) / (x1 - x2);
	b = y1 - (a * x1);

	return a * pint + b;
}

void main() {
	float slope_s = 0;
	float x1 = 499;
	float y1 = 1080;
	float x2 = 890;
	float y2 = 658;

	cout << point_calculation(Point(x1, y1), Point(x2, y2), x2) << endl;
	slope_s = (y1 - y2) / (x1 - x2);
	float b = y1 - (slope_s * x1);
	cout << "y = " << slope_s << "x + " << b << endl;
}