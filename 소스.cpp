#include "function.h" 

#define PI 3.1415926535

void main() {
	float slope_s = 0;
	int x1;
	int x2;
	int y1;
	int y2;

	slope_s = atan2(y2 - y1, x2 - x1) * 180 / PI + 90;
	cout << "왼쪽차선 기울기 : " << slope_s << endl;
}