#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>
#include <gsl/gsl_fit.h>
#include <iostream>  
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <time.h>
#include <Windows.h>
#include <limits.h> 
#include <cstring>
#include "opencv2/core/ocl.hpp"
#include "includePython.h"
#include "numpy\arrayobject.h"

using std::string;
using std::cout;
using std::endl;

//영상처리 메인
int video_main(string videoname, string filename[]);