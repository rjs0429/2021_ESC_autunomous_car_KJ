#include "function.h" 

using  std::cin;
using  std::cerr;


int main(int, char**)
{
	clock_t start, end;
	start = clock();
	string filename[] = { "D:\\OneDrive - 공주대학교\\자율주행 프로젝트\\live.avi", "D:\\OneDrive - 공주대학교\\자율주행 프로젝트\\live2.avi" };
	//영상처리 시작
	video_main("D:\\OneDrive - 공주대학교\\자율주행 프로젝트\\고화질.mp4", filename);

	end = clock();
	printf("타이머 : %.2f초\n\n", ((float)(end - start) / CLOCKS_PER_SEC));

	return 0;
}