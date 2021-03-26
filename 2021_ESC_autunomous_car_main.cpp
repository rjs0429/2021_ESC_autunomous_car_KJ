#include "function.h" 

using  std::cout;
using  std::endl;
using  std::cin;
using  std::cerr;


int main(int, char**)
{
	//통신 시작
	tcp_server_onoff(true);

	clock_t start, end;
	start = clock();

	//영상처리 시작
	video_main("D:\\OneDrive - 공주대학교\\자율주행 프로젝트\\Untitled5.mp4", "D:\\OneDrive - 공주대학교\\자율주행 프로젝트\\live.avi");

	end = clock();
	printf("타이머 : %.2f초\n\n", ((float)(end - start) / CLOCKS_PER_SEC));
	
	//통신 종료
	tcp_server_onoff(false);

	return 0;
}