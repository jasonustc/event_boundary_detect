// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <afx.h>
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <algorithm>
using namespace std;

#define OK 0/*the algorithm works well*/
#define EMPTY_IMAGE_COLLECTION 1/*there are no meta data in the photo collection*/
#define  EMPTY_IMAGE_DATA 2
#define  CANNY_FAILED 3
#define TOO_SMALL_IMAGE_FOR_DFT 4
#define  BESTPHOTO_NUM_OUT_OF_RANGE 5
#define  FAIL_TO_LOAD_SVM_MODEL 6
// TODO: reference additional headers your program requires here
