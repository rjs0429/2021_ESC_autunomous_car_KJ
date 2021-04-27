#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>
#include <gsl/gsl_fit.h>
#include <iostream>  
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <Windows.h>
#include<winsock.h> 

#include<algorithm>


#include<stdbool.h>	//bool형 추가

#define SC 1					//영상 표시 스케일



//using namespace std;			//통신할때 문제 발생
using namespace cv;

using  std::cout;
using  std::endl;
using  std::cin;
using  std::cerr;

using std::string;
using std::vector;

using std::sort;

//통신 보내기
void tcp_server(const char menu[], float msg);

//통신 시작 / 종료
void tcp_server_onoff(bool check);

//영상처리 메인
int video_main(string videoname, string filename);

//우선정지 검출함수
void stop_check(Mat copy);

//신호등 함수들
void traffic_lights_check(Mat copy);

void check_circle_lights(Mat filtered_video);

void stop_red(Mat red_light);
void stop_yellow(Mat red_light);
void go_left_green(Mat green_go_light);