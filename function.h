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


#include<stdbool.h>	//bool�� �߰�

#define SC 1					//���� ǥ�� ������



//using namespace std;			//����Ҷ� ���� �߻�
using namespace cv;

using  std::cout;
using  std::endl;
using  std::cin;
using  std::cerr;

using std::string;
using std::vector;

using std::sort;

//��� ������
void tcp_server(const char menu[], float msg);

//��� ���� / ����
void tcp_server_onoff(bool check);

//����ó�� ����
int video_main(string videoname, string filename);

//�켱���� �����Լ�
void stop_check(Mat copy);

//��ȣ�� �Լ���
void traffic_lights_check(Mat copy);

void check_circle_lights(Mat filtered_video);

void stop_red(Mat red_light);
void stop_yellow(Mat red_light);
void go_left_green(Mat green_go_light);