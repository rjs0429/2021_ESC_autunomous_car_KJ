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

#include<stdbool.h>	//bool형 추가

using namespace std;
using namespace cv;

//통신 보내기
void tcp_server(float msg);
//통신 시작 / 종료
void tcp_server_onoff(double check);

//영상처리 메인
int video_main(string videoname, string filename);

//우선정지 검출함수
bool is_priority_stop(Mat video_for_copy);