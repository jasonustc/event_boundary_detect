// this is the lib file created by xu shen in 10/31/2015
// contact: shenxu@mail.ustc.edu.cn

#include "xu_lib.h"

//transform system time into absolute time
double GetSecondTime(IN SYSTEMTIME SysTime)
{
	double dSecond = 0;

	int iYearOffset = SysTime.wYear - YEAR_OFFSET;
	int iYear = SysTime.wYear;
	int iMonth = SysTime.wMonth;
	int iDay = SysTime.wDay;
	int iHour = SysTime.wHour;
	int iMin = SysTime.wMinute;
	int iSec = SysTime.wSecond;

	bool leapYear = ((iYear % 4 == 0 && iYear % 100 != 0) || iYear % 400 == 0 );
	int iDayPerYear = leapYear ? 366 : 365;

	int iMonDays = 0;
	for (int i = 1; i < iMonth; i++)
	{
		int iDayPerMon = 31;
		switch (i)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				iDayPerMon = 31;
			break;

		case 2:
			if (leapYear == true)
			{
				iDayPerMon = 29;
			}
			else
			{
				iDayPerMon = 28;
			}
			break;

		default:
			iDayPerMon = 30;
		}
		iMonDays += iDayPerMon;
	}

	//convert to minutes
	dSecond = double(iYearOffset * iDayPerYear * 24 * 60 + iMonDays * 24 * 60 + iDay * 24 * 60 + iHour * 60 + iMin) + iSec / 60.0;
	return dSecond;
}
