#include "stdafx.h"
#include "PhotoProcess.h"
#include "Cluster.h"
#include "exif.h"

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
HRESULT CPhotoProcess::GetTimeStampFromSinglePhoto(const string& imgPath, SYSTEMTIME& sysTime,
	double& longtitude, double& latitude, double& altitude)
{
	HRESULT hr = S_OK;
	//time stamp
	FILE *fp;
	fopen_s(&fp, imgPath.c_str(), "rb");
	if (!fp){
		std::printf("Can't open file %s.\n", imgPath.c_str());
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
		return E_FAIL;
		delete[] buf;
	}
	fclose(fp);
	//parse EXIF
	EXIFInfo photoExif;
	TCHAR tszFileName[MAX_PATH];
	int errorCode = photoExif.parseFrom(buf, fsize);
	delete[] buf;
	if (errorCode){
		std::printf("Error parsing EXIF of %s, code %d, use created time instead.\n",
			imgPath.c_str(), errorCode);
		// here how to set a default value of photo's time?
		CPVImageInfo PhotoInfo;
		size_t numBytes;
		//from string to TCHAR*
		mbstowcs_s(&numBytes,tszFileName,MAX_PATH,imgPath.c_str(),imgPath.length());
		hr = PhotoInfo.SetImageFile(tszFileName);

		if (SUCCEEDED(hr))
		{
			hr = PhotoInfo.GetDTOrig(&sysTime);  // always succeed!
		}
		latitude = 0;
		longtitude = 0;
		altitude = 0;
		return S_OK;
		//return ERROR_PARSING_EXIF;
	}
	else{
		wchar_t timeBuf[128];
		mbstowcs(timeBuf, photoExif.DateTimeOriginal.c_str(), 128);
		CPVImageInfo::ExifDTToDateTime(timeBuf, &sysTime);
		//latitude
		latitude = photoExif.GeoLocation.Latitude;
		//longitude
		longtitude = photoExif.GeoLocation.Longitude;
		//altitude
		altitude = photoExif.GeoLocation.Altitude;
	}
	/*
	CPVImageInfo PhotoInfo;

	hr = PhotoInfo.SetImageFile(szFileName);

	if (SUCCEEDED(hr))
	{
	hr = PhotoInfo.GetDTOrig(&SysTime);  // always succeed!
	}
	*/
//	delete tszFileName;
	return hr;
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
	TCHAR tszFileName[MAX_PATH];
	int errorCode = photoExif.parseFrom(buf, fsize);
	delete[] buf;
	if (errorCode){
		std::printf("Error parsing EXIF of %s, code %d, use created time instead.\n",
			imgPath.c_str(), errorCode);
		// here how to set a default value of photo's time?
		CPVImageInfo PhotoInfo;
		size_t numBytes;
		//from string to TCHAR*
		mbstowcs_s(&numBytes, tszFileName, MAX_PATH, imgPath.c_str(), imgPath.length());
		hr = PhotoInfo.SetImageFile(tszFileName);

		if (SUCCEEDED(hr))
		{
			hr = PhotoInfo.GetDTOrig(&sysTime);  // always succeed!
		}
		return S_OK;
		//return ERROR_PARSING_EXIF;
	}
	else{
//		wchar_t timeBuf[128];
//		mbstowcs(timeBuf, photoExif.DateTimeOriginal.c_str(), 128);
//		CPVImageInfo::ExifDTToDateTime(timeBuf, &sysTime);
		//always use CPVImageInfo to extract time feature
		CPVImageInfo PhotoInfo;
		size_t numBytes;
		//from string to TCHAR*
		mbstowcs_s(&numBytes, tszFileName, MAX_PATH, imgPath.c_str(), imgPath.length());
		hr = PhotoInfo.SetImageFile(tszFileName);

		if (SUCCEEDED(hr))
		{
			hr = PhotoInfo.GetDTOrig(&sysTime);  // always succeed!
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
HRESULT CompGPSInfo(vector<Photo_Feature_Set> &photos);
HRESULT CPhotoProcess::LoadPhotos(const TCHAR* m_tszUsrDir)
{
	HRESULT hr = S_OK;
	int iFileNum = 0;
	vector<wstring> m_vecPicFile;
	hr = GetImageFilesInDir(m_tszUsrDir, m_vecPicFile);
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
		if (LoadPhotoFeat(m_vecPicFile[i], photoFeat)){
			photoFeat.photoIndex = index;
			this->m_vecPhotos.push_back(photoFeat);
			index++;
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
	TCHAR tszFileName[MAX_PATH];
	_stprintf_s(tszFileName, _T("%s"), photoPath.c_str());
	char pFilePathName[MAX_PATH];
	int nLen = wcslen(tszFileName) + 1;
	WideCharToMultiByte(CP_ACP, 0, tszFileName, nLen, pFilePathName, 2 * nLen, NULL, NULL);
	string imgPath = pFilePathName;
	PhotoExifInfo pExifInfo;
	HRESULT hResult = GetTimeStampFromSinglePhoto(imgPath, SysTime, pExifInfo);
	if (SUCCEEDED(hResult))
	{
		std::cout << imgPath << "\n";
		PhotoFeat.SysTime = SysTime;
		dTime = GetSecondTime(SysTime);
		PhotoFeat.dTimeStamp = dTime;
		sprintf(PhotoFeat.tszFileName, pFilePathName);
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

int CPhotoProcess::ProcessPhotos(const TCHAR* m_UsrDir)
{
	// load all photos and their time-stamp information into m_vecPhotoLabel
	int ErrCode = 0;
	//int ErrCode = LoadPhotos(m_UsrDir); // always succeed!
	ErrCode = GetReEventPhotos(m_UsrDir);
	//sort photos by time
	this->SortPhotos();
	return ErrCode;
}

bool PhotoInVector(wstring& photoPath, vector<Photo_Feature_Set>& photoVec){
	for (std::vector<Photo_Feature_Set>::iterator iter = photoVec.begin(); 
		iter != photoVec.end(); ++iter){
		string path(photoPath.begin(), photoPath.end());
		if (strcmp(path.c_str(), (*iter).tszFileName) == 0){
			return true;
		}
	}
	return false;
}

int PhotoTimeInOldEvent(Photo_Feature_Set& photo, vector<SimpleEventInfo>& simEvents){
	for (size_t i = 0; i < simEvents.size(); i++){
		if (photo.dTimeStamp >= simEvents[i].startTime && photo.dTimeStamp <= simEvents[i].endTime){
			return i;
		}
	}
	//photo time is not in the time span of the events
	return -1;
}

bool CPhotoProcess::GetReEventPhotos(const TCHAR* m_UsrDir){
	vector<wstring> xmlFiles;
	wstring ext = L"xml";
	GetFilesInDirWithExt(m_UsrDir, xmlFiles, ext);
	vector<wstring> picFiles;
	GetImageFilesInDir(m_UsrDir, picFiles);
	vector<Photo_Feature_Set> oldPhotos;
	vector<SimpleEventInfo> oldEventInfos;
	for (size_t i = 0; i < xmlFiles.size(); i++){
		string xmlFile(xmlFiles[i].begin(), xmlFiles[i].end());
		//currently we only consider 1 event segmentation file
		if (!LoadEventFromXml(xmlFile, oldPhotos, oldEventInfos)){
			break;
		}
	}
	//here if the number of added photos is less than 20, we do not re-cluster
	//TODO: adjust this threshold
	//TODO: deal with the situation that user delete some photos
	if (oldPhotos.size() > 0 &&  picFiles.size() < oldPhotos.size() + 5){
		return false;
	}
	vector<Photo_Feature_Set> newPhotos;
	vector<int> mergeEventIdx;
	for (size_t i = 0; i < picFiles.size(); i++){
		//not in the clustered photos before
		if (!PhotoInVector(picFiles[i], oldPhotos)){
			Photo_Feature_Set newPhoto;
			//load photo feature
			if (LoadPhotoFeat(picFiles[i], newPhoto)){
				newPhotos.push_back(newPhoto);
				//push back all related photos
				int e = PhotoTimeInOldEvent(newPhoto, oldEventInfos);
				if (e >= 0 && std::find(mergeEventIdx.begin(), mergeEventIdx.end(), e) == mergeEventIdx.end()){
					for (size_t p = 0; p < oldEventInfos[e].photoIdx.size(); p++){
						newPhotos.push_back(oldPhotos[oldEventInfos[e].photoIdx[p]]);
					}
					mergeEventIdx.push_back(e);
				}
			}
		}
	}
	//reserve the info of photos that do not need to cluster again
	int num = 0;
	for (size_t i = 0; i < oldEventInfos.size(); i++){
		if (std::find(mergeEventIdx.begin(), mergeEventIdx.end(), i) == mergeEventIdx.end()){
			vector<int> eIdx;
			for (size_t p = 0; p < oldEventInfos[i].photoIdx.size(); p++){
				this->vecOldPhotos.push_back(oldPhotos[oldEventInfos[i].photoIdx[p]]);
				eIdx.push_back(num);
				num++;
			}
			this->vecOldEventIdx.push_back(eIdx);
		}
	}

	//TODO: consider global information
	this->m_vecPhotos = newPhotos;
	return true;
}

bool FindNearestNeigh(vector<Photo_Feature_Set> &photos,const float timestamp
					  ,double &lat,double &lon, double &alt);
//to complete the gps information of photos by their nearest neighbor in time
HRESULT CompGPSInfo(vector<Photo_Feature_Set> &photos){
	if ( photos.size()==0){
		return E_FAIL;
	}
	for(unsigned int i=0;i<photos.size();i++){
		Photo_Feature_Set photo=photos[i];
		if (photo.latitude==0 && photo.latitude==0 && photo.atitude ==0){
			FindNearestNeigh(photos,photo.dTimeStamp,photo.latitude,photo.longitude,photo.atitude);
			photos[i].latitude=photo.latitude;
			photos[i].longitude=photo.longitude;
			photos[i].atitude=photo.atitude;
		}
	}
	return S_OK;
}

//find the longtitude,latitude and atitude of the nearest photo in time
bool FindNearestNeigh(vector<Photo_Feature_Set> &photos,const float timestamp
					  ,double &lat,double &lon,double &alt){
	float minTimeDiff=1e10;
	for(unsigned int i=0;i<photos.size();i++){
		Photo_Feature_Set photo=photos[i];
		if(abs(photo.dTimeStamp-timestamp)<minTimeDiff && (photo.latitude!=0 
			|| photo.longitude!=0 || photo.atitude !=0)){
			minTimeDiff=abs(photo.dTimeStamp-timestamp);
			lat=photo.latitude;
			lon=photo.longitude;
			alt=photo.atitude;
		}
	}
	return true;
}
