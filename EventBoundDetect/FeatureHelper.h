#ifndef FEATUREHELPER_H
#define FEATUREHELPER_H

#include "stdafx.h"
#include "PhotoInfo.h"
#include <vector>
#include <algorithm> //std::replace
#include <functional> //std::vector
#include "FolderFileHelper.h"


//input configurations structure
struct InputConfig{
    //input photo directory
	wstring tszImageDir;
	//output photo directory
	wstring tszOutImageDir;
	//if we use gps information to do event clustering
	bool use_gps;
    //output file of clustering indexed by photos
	TCHAR* tszPhotoSegFile;
    //output file of clustering indexed by events
	TCHAR* tszEventSegFile;
	//global photo density
	float global_density;
	//K
	int K;
	//threshold of local similarity of two subevents
	float threshold;

	//control the scale of time
	float timeK;
	//control the scale of location
	float gpsK;

	//photo feature file
	string photoFeatFile;

	//event feature file(txt)
	string eventFileTxt;
	
	//event feature file(xml)
	string eventFeatXml;



	InputConfig() :K(10), use_gps(false) , global_density(1), threshold(0.75), 
		timeK(1), gpsK(1){
		this->tszPhotoSegFile = _T("photo_event.xml");
		this->tszEventSegFile = _T("event_photos.xml");
		this->tszOutImageDir = _T("");
		this->photoFeatFile = "";
		this->eventFileTxt = "";
	}
};

//simple event information
struct SimpleEventInfo{
	double startTime;
	double endTime;
	vector<int> photoIdx;
};

void RemoveSubString(string& str, const string& subStr);
int CountNumEvents(vector<Photo_Feature_Set>& userPhotos);
int CountSubString(const string& str, const string& subStr);
int SplitPhotoToDifferentUsers(vector<Photo_Feature_Set>& photos,
	vector<vector<Photo_Feature_Set>>& splitPhotos, vector<string>& userNames);
vector<string> split(const string& s, char delim);
int GetDayInfoInCollection(vector<Photo_Feature_Set>& photos);
bool SaveEvent2PhotosAsXml(const vector<vector<int>> eventIdx, const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath);
bool SavePhoto2EventAsXml(const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath);
int LoadPhotoFromXml(string& xmlFile, vector<Photo_Feature_Set>& photos);
int LoadEventFromXml(string& xmlFile, vector<Photo_Feature_Set>& photos, vector<SimpleEventInfo>& simEventInfos);
int LoadPhotoFromTxtFolder(string& folderPath, vector<Photo_Feature_Set>& photos);
bool SavePhoto2EventAsText(const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath);
bool SaveEvent2PhotosAsText(const vector<vector<int>> eventIdx, const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath);
bool AssignPhotos2Event(vector<Photo_Feature_Set>& photos, int days, InputConfig& inputCfg);
void SaveImagesToFolder(const vector<Photo_Feature_Set>& photos, vector<vector<int>>& images, const string& saveFolder);

float GetSimOfTwoVec(vector<float>& feat1, vector<float>& feat2);
float GetSimOfTwoVec2(vector<float>& feat1, vector<float>& feat2);
float GetSimOfTwoVecNorm(vector<float>& feat1, vector<float>& feat2, vector<float>& avg, vector<float>& stdv);

bool isVecEmpty(vector<float>& vec);

#endif
