// this is the lib file created by xu shen in 10/31/2015
// contact: shenxu@mail.ustc.edu.cn

#ifndef XU_LIB_H_
#define XU_LIB_H_
#endif

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <io.h>
#include <Windows.h>
using std::string;

#define XU_LOG_ERROR(info__) \
		char errMsg__[MAX_PATH]; \
		sprintf_s(errMsg__, MAX_PATH, "%s,LINE %d: %s", __FILE__, __LINE__, info__); \
		std::ofstream err__("ERROR.txt", std::ios::app); \
		err__ << errMsg__ << "\n"; \
		err__.close()

#define YEAR_OFFSET					1990

double GetSecondTime(IN SYSTEMTIME SysTime);
