#ifndef FEATUREHELPER_H
#define FEATUREHELPER_H

#include "stdafx.h"
#include "PhotoInfo.h"
#include "PhotoProcess.h"
#include <vector>
#include <algorithm> //std::replace
#include <functional> //std::vector


//input configurations structure
struct InputConfig{
    //input photo directory
	string tszImageDir;
	//output photo directory
	string tszOutImageDir;
	//if we use gps information to do event clustering
	bool use_gps;
    //output file of clustering indexed by photos
	const char* tszPhotoSegFile;
    //output file of clustering indexed by events
	const char* tszEventSegFile;
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



	InputConfig() :use_gps(false) , global_density(1), K(10), threshold(0.75),
		timeK(1), gpsK(1){
		this->tszPhotoSegFile = "photo_event.xml";
		this->tszEventSegFile = "event_photos.xml";
		this->tszOutImageDir = "";
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

vector<string> split(const string& s, char delim);
int GetDayInfoInCollection(vector<Photo_Feature_Set>& photos);
bool SaveEvent2PhotosAsXml(const vector<vector<int>> eventIdx, const vector<Photo_Feature_Set> &photos, const char* outFilePath);
bool SavePhoto2EventAsXml(const vector<Photo_Feature_Set> &photos, const char* outFilePath);
int LoadPhotoFromXml(string& xmlFile, vector<Photo_Feature_Set>& photos);
int LoadEventFromXml(string& xmlFile, vector<Photo_Feature_Set>& photos, vector<SimpleEventInfo>& simEventInfos);
int LoadPhotoFromTxtFolder(string& folderPath, vector<Photo_Feature_Set>& photos);
bool SavePhoto2EventAsText(const vector<Photo_Feature_Set> &photos, const char* outFilePath);
bool SaveEvent2PhotosAsText(const vector<vector<int>> eventIdx, const vector<Photo_Feature_Set> &photos, const char* outFilePath);
bool AssignPhotos2Event(vector<Photo_Feature_Set>& photos, int days, InputConfig& inputCfg);
void SaveImagesToFolder(const vector<Photo_Feature_Set>& photos, vector<vector<int>>& images, const string& saveFolder);

float GetSimOfTwoVec(vector<float>& feat1, vector<float>& feat2);
float GetSimOfTwoVec2(vector<float>& feat1, vector<float>& feat2);
float GetSimOfTwoVecNorm(vector<float>& feat1, vector<float>& feat2, vector<float>& avg, vector<float>& stdv);

bool isVecEmpty(vector<float>& vec);

#endif
