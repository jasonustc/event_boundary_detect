#include "stdafx.h"
#include "PhotoProcess.h"
#include "exif.h"
#include "3rdparty\pugixml.hpp"
#include "FeatureHelper.h"
#include <algorithm>
#include "xu_lib.h"

CPhotoProcess::CPhotoProcess()
{
	m_iPhotoNum = 0;
}


CPhotoProcess::~CPhotoProcess()
{
}
//get the information of the processed photos
void CPhotoProcess::GetPhotoFeats(vector<Photo_Feature_Set> &photoFeats)
{
    photoFeats = this->m_vecPhotos;
}

void CPhotoProcess::SortPhotos(){
	//sort by time
	std::sort(m_vecPhotos.begin(), m_vecPhotos.end(), timeComperer());
}


/*********************************************************************************************\
GetTimeStamp

[IN]  szFileName - photo file name
[OUT] SysTime - system time: timestamp or photo creation time
\*********************************************************************************************/
HRESULT CPhotoProcess::GetTimeStampFromSinglePhoto(const string& imgPath, SYSTEMTIME& sysTime, PhotoExifInfo& photoExifInfo)
{
	HRESULT hr = S_OK;
	//time stamp
	FILE *fp;
	fopen_s(&fp, imgPath.c_str(), "rb");
	if (!fp){
		std::printf("Can't open file %s.\n", imgPath.c_str());
		string msg = "Can't open file " + imgPath;
		XU_LOG_ERROR(msg.c_str());
		return E_FAIL;
	}
	//sets the position indicator with the stream to a new position
	fseek(fp, 0, SEEK_END);
	//ftell: get current position in stream
	unsigned long fsize = ftell(fp);
	//set position of the stream to the beginning
	rewind(fp);
	unsigned char *buf = new unsigned char[fsize];
	//read 1*fsize data from fp into buf
	if (fread(buf, 1, fsize, fp) != fsize){
		std::printf("can't read file %s", imgPath.c_str());
		delete[] buf;
		return E_FAIL;
	}
	fclose(fp);
	//parse EXIF
	EXIFInfo photoExif;
	int errorCode = photoExif.parseFrom(buf, fsize);
	delete[] buf;
	if (errorCode){
		std::printf("Error parsing EXIF of %s, code %d, use created time instead.\n",
			imgPath.c_str(), errorCode);
		CPVImageInfo PhotoInfo;
		//from string to TCHAR*
		TCHAR tszFileName[MAX_PATH];
		LONG len = MultiByteToWideChar(CP_ACP, 0, imgPath.c_str(), -1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, imgPath.c_str(), -1, tszFileName, len + 1);//////////////////////////////

		hr = PhotoInfo.SetImageFile(tszFileName);

		if (SUCCEEDED(hr)){
			hr = PhotoInfo.GetDTOrig(&sysTime);  // always succeed!
		}
		else{
			cout << "set image file name failed. " << tszFileName << "\n";
			wstring wFile(tszFileName);
			string sFile(wFile.begin(), wFile.end());
			string err = "set image file name failed. " + sFile;
			XU_LOG_ERROR(err.c_str());
		}
		return S_OK;
		//return ERROR_PARSING_EXIF;
	}
	else{
		//always use CPVImageInfo to extract time feature
		CPVImageInfo PhotoInfo;
		//from string to TCHAR*
		TCHAR tszFileName[MAX_PATH];
		LONG len = MultiByteToWideChar(CP_ACP, 0, imgPath.c_str(), -1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, imgPath.c_str(), -1, tszFileName, len + 1);//////////////////////////////
		hr = PhotoInfo.SetImageFile(tszFileName);

		if (SUCCEEDED(hr)){
			hr = PhotoInfo.GetDTOrig(&sysTime);  // always succeed!
		}
		else{
			cout << "set image file name failed. " << tszFileName << "\n";
			wstring wFile(tszFileName);
			string sFile(wFile.begin(), wFile.end());
			string err = "set image file name failed. " + sFile;
			XU_LOG_ERROR(err.c_str());
		}
		//latitude
		photoExifInfo.latitude = photoExif.GeoLocation.Latitude;
		//longitude
		photoExifInfo.longitude = photoExif.GeoLocation.Longitude;
		//altitude
		photoExifInfo.altitude = photoExif.GeoLocation.Altitude;
		//brightness value
		photoExifInfo.Fnumber = photoExif.FNumber;
		photoExifInfo.ExpoTime = photoExif.ExposureTime;
		photoExifInfo.ISOsr = photoExif.ISOSpeedRatings;
		photoExifInfo.BrightValue = photoExifInfo.Brightness();
	}
	//camera info
	photoExifInfo.camModel = photoExif.Model;
	photoExifInfo.maker = photoExif.Make;
	//	delete tszFileName;
	return hr;
}

//transform system time into absolute time
//in minutes
double CPhotoProcess::GetSecondTime(IN SYSTEMTIME SysTime)
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
	dSecond = double(iYearOffset * iDayPerYear * 24 * 60 + iMonDays * 24 * 60 + iDay * 24 * 60 + iHour * 60 + iMin) + iSec / 60.0;
	return dSecond;
}
/*********************************************************************************************\
LoadPhotos

[IN]  m_szUsrDir - the directory containing all the photos of a user
[OUT] m_vecPhotos - the unsorted vector containing the meta data of all photos
\*********************************************************************************************/
HRESULT CPhotoProcess::LoadPhotos(const TCHAR* m_tszUsrDir, const TCHAR* outFilePath)
{
	HRESULT hr = S_OK;
	int iFileNum = 0;
	vector<wstring> m_vecPicFile;
	try{
		hr = GetImageFilesInDir(m_tszUsrDir, m_vecPicFile);
	}
	catch (int& code){
		cout << "GetImageFilesInDir failsed, ERROR CODE: " << code << "\n";
		return E_FAIL;
	}
	iFileNum = m_vecPicFile.size();
	if (iFileNum == 0 || FAILED(hr))
	{
		_tprintf(_T("\nError: There is no photo file in the directory of %s!\n"), m_tszUsrDir);
		return E_FAIL;
	}
	int index = 0;
	for (int i = 0; i < iFileNum; i++)
	{
		Photo_Feature_Set photoFeat;
		if (LoadPhotoFeat(m_vecPicFile[index], photoFeat)){
			photoFeat.photoIndex = index;
			this->m_vecPhotos.push_back(photoFeat);
			index++;
		}
		else{
			string picFile(m_vecPicFile[index].begin(), m_vecPicFile[index].end());
			printf("Load feature of photo '%s' failed.", picFile.c_str());
		}
	}

	//replace missing gps info of photo by its nearest neighbor
	// in time line.
//	CompGPSInfo(this->m_vecPhotos);

	m_iPhotoNum = this->m_vecPhotos.size();

	return hr;
}

bool CPhotoProcess::LoadPhotoFeat(wstring& photoPath, Photo_Feature_Set& PhotoFeat){
	SYSTEMTIME SysTime;
	double dTime;
	char imgPath[MAX_PATH];
	LONG len = WideCharToMultiByte(CP_ACP, 0, photoPath.c_str(), -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, photoPath.c_str(), -1, imgPath, len + 1, NULL, NULL);///////////////////
	PhotoExifInfo pExifInfo;
	HRESULT hResult = GetTimeStampFromSinglePhoto(imgPath, SysTime, pExifInfo);
	if (SUCCEEDED(hResult))
	{
		std::cout << imgPath << "\n";
		PhotoFeat.SysTime = SysTime;
		dTime = GetSecondTime(SysTime);
		PhotoFeat.dTimeStamp = dTime;
		sprintf_s(PhotoFeat.tszFileName, MAX_PATH,"%s", imgPath);
		PhotoFeat.longitude = pExifInfo.longitude;
		PhotoFeat.latitude = pExifInfo.latitude;
		PhotoFeat.atitude = pExifInfo.altitude;
		PhotoFeat.camModel = pExifInfo.camModel;
		PhotoFeat.maker = pExifInfo.maker;
		PhotoFeat.brightValue = pExifInfo.BrightValue;
		return true;
	}
	return false;
}

int CPhotoProcess::ProcessPhotos(const TCHAR* m_UsrDir, const TCHAR* outFilePath)
{
	// load all photos and their time-stamp information into m_vecPhotoLabel
	int ErrCode = LoadPhotos(m_UsrDir, outFilePath); // always succeed!
	//sort photos by time
	this->SortPhotos();
	return ErrCode;
}
