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

//��� ������
void tcp_server(float msg);
//��� ���� / ����
void tcp_server_onoff(double check);

//����ó�� ����
int video_main(string videoname, string filename);