#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>
#include <gsl/gsl_fit.h>
#include <iostream>  
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <Windows.h>
#include <winsock.h> 
#include <limits.h> 

using namespace std;

//통신 보내기
void tcp_server(float msg);
//통신 시작 / 종료
void tcp_server_onoff(double check);

//영상처리 메인
int video_main(string videoname, string filename);