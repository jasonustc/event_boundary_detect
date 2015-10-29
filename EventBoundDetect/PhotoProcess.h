#ifndef PHOTOPROCESS_H
#define PHOTOPROCESS_H

#include <map>
#include "PhotoInfo.h"
#include "PVImageInfo.h"
#include "FolderFileHelper.h"

struct timeComperer
{
	inline bool operator()(const Photo_Feature_Set& photo1, const Photo_Feature_Set& photo2){
		return (photo1.dTimeStamp < photo2.dTimeStamp);
	}
};

struct PhotoExifInfo{ 
	double longitude;
	double latitude;
	double altitude;
	std::string maker;
	std::string camModel;
	//F number
	float Fnumber;
	//exposure time
	float ExpoTime;
	//ISO speed rating
	float ISOsr;

	//brightness value
	float BrightValue;

	float Brightness(){
//		cout << "Fn: " << Fnumber << " ExpoTime: " << ExpoTime << " ISOrate: " << ISOsr << "\n";
		if (Fnumber == 0 || ExpoTime == 0 || ISOsr == 0){
			return NULL;
		}
		float apertValue = log2f(Fnumber * Fnumber);
		float timeValue = -log2f(ExpoTime);
		double speedValue = log2f(ISOsr / 3.125);
		return apertValue + timeValue - speedValue;
	}

	PhotoExifInfo() : longitude(0.), latitude(0.), altitude(0.),
		Fnumber(0.), ExpoTime(0.), ISOsr(0.), maker(""), camModel(""){}
};

class CPhotoProcess
{
public:
	CPhotoProcess();
	CPhotoProcess(TCHAR* SourceDir,TCHAR* OutputDir,string &segmentPhotoDir);
	~CPhotoProcess();
	int ProcessPhotos(const TCHAR* imgDir, const TCHAR* outFilePath);
    void GetPhotoFeats(vector<Photo_Feature_Set> &photoFeats);

protected:
	void SortPhotos();
	HRESULT LoadPhotos(const TCHAR* imgDir, const TCHAR* outFilePath);
	HRESULT GetTimeStampFromSinglePhoto(const string& imgPath, SYSTEMTIME& sysTime, 
		PhotoExifInfo& photoExifInfo);
	double GetSecondTime(IN SYSTEMTIME SysTime);
	bool LoadPhotoFeat(wstring& photoPath, Photo_Feature_Set& PhotoFeat);

private:
	//photos need to be clustered
	vector<Photo_Feature_Set> m_vecPhotos;

	int m_iPhotoNum; 
	int m_iEventNum;

};


#endif