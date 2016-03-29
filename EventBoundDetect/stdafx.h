// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <cstdio>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdint>
using namespace std;

#define EMPTY_IMAGE_COLLECTION 1/*there are no meta data in the photo collection*/
#define  EMPTY_IMAGE_DATA 2
#define  CANNY_FAILED 3
#define TOO_SMALL_IMAGE_FOR_DFT 4
#define  BESTPHOTO_NUM_OUT_OF_RANGE 5
#define  FAIL_TO_LOAD_SVM_MODEL 6
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif
// TODO: reference additional headers your program requires here
//
#ifndef WORD
#define WORD unsigned short
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif


enum class  StatusCode {
  OK = 0,
  Error = 1,
  InvalidArgs = 2,

};

#ifndef SYSTEMTIME
#define SYSTEMTIME struct tm
#endif

#define __in
#define __out
