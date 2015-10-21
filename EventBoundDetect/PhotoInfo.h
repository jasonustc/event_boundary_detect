#ifndef PHOTOINFO_H
#define PHOTOINFO_H

#include "stdafx.h"
#include <vector>
using namespace std;

#define PCA_COLOR_DIM		1			// the PCA dimension of color feature
#define PCA_TEXTURE_DIM		1		// the PCA dimension of texture feature
#define FEATURE_DIM         6 
#define TIME_FEATURE_DIM    1
#define GPS_FEATURE_DIM     2
#define QUALITY_FEATURE_DIM 1
#define GPS_DIM             2

#define PHOTO_INTERVAL 720	// 12 hours, 12 * 60 mins
#define BESTPHOTO_INTERVAL 240 // 4 hours, 4*60 mins
#define FEATURE_TAMURA_DIM			20
#define FEATURE_CLRHIST_DIM			64


#define YEAR_OFFSET					1990
#define TIME_INTERVAL				5		// 5 minutes = 300 sec
#define MAX_EVENTS_PERDAY			2		// maximum event number in a day
#define EVENT_STEP					1

#define ATTENTION_IMAGE_WIDTH		320
#define ATTENTION_IMAGE_HEIGHT		240

#define DEL_IPLIMG(x) if(NULL!=x) {cvReleaseImage(&x); x=NULL;}
#define DEL_ARRAY(x) if(NULL!=x) {delete [] x; x=NULL;}
#define DEL_POINTER(x) if(NULL!=x) {delete x; x=NULL;}
#define DEL_MATRIX(x) if(NULL!=x) {cvReleaseMat(&x); x=NULL;}
#define DEL_FILE(x) if(NULL!=x) {fclose(x); x=NULL;}



struct Photo_Feature_Set
{
	//1. Time Feature
	double dTimeStamp;
	SYSTEMTIME SysTime;
	// 2. Photo index
	int photoIndex = 0;
	//3. aesthetic quality
	float pflQuality = 0;
	//4.Event label
	int iEventLabel = 0;
	//5.image path
	char tszFileName[MAX_PATH];
	//6.label of best photo
	BOOL fRep = 0;
	//7. longitude
	double longitude = 0;
	//8. latitude
	double latitude = 0;
	//9. atitude
	double atitude = 0;
	//10. camera model
	std::string camModel;
	//11. maker
	std::string maker;
	//12. day_id
	int day_id = 0;
	//13. brightness value
	float brightValue = 0;
	//14. toc score for local segment
	float TOCscore = 0;

	void LoadFeatFromTxt(const string& featPath){
		ifstream inFeat(featPath);
		if (!inFeat.is_open()){
			printf("failed to load feature from file: %s", featPath.c_str());
			return;
		}
		string line;
		while (getline(inFeat, line)){
			vector<string> subStrs = parseLine(line, ':');
			if (subStrs[0] == "Path"){
				sprintf(tszFileName, "%s", subStrs[1].c_str());
			}
			else if (subStrs[0] == "Time"){
				dTimeStamp = stod(subStrs[1]);
			}
			else if (subStrs[0] == "SystemTime"){
				SysTime = String2SysTime(subStrs[1]);
			}
			else if (subStrs[0] == "Latitude"){
				latitude = stof(subStrs[1]);
			}
			else if (subStrs[0] == "longitude"){
				longitude = stof(subStrs[1]);
			}
			else if (subStrs[0] == "Altitude"){
				atitude = stof(subStrs[1]);
			}
			else if (subStrs[0] == "Camera")
			{
				camModel = subStrs[1];
			}
			else if (subStrs[0] == "Maker"){
				maker = subStrs[1];
			}
		}
		inFeat.close();
	}

private:

	vector<string> parseLine(string& line, char delim){
		int loc = line.find_first_of(delim);
		vector<string> subStrs;
		subStrs.push_back(line.substr(0, loc));
		subStrs.push_back(line.substr(loc + 1));
		return subStrs;
	}

	SYSTEMTIME String2SysTime(const string& str){
		SYSTEMTIME sysTime = { 0 };
		vector<string> subStrs = split(str, ':');
		if (subStrs.size() != 6){
			return sysTime;
		}
		std::istringstream iss;
		iss.str(subStrs[0]);
		iss >> sysTime.wYear;
		iss.clear();
		iss.str(subStrs[1]);
		iss >> sysTime.wMonth;
		iss.clear();
		iss.str(subStrs[2]);
		iss >> sysTime.wDay;
		iss.clear();
		iss.str(subStrs[3]);
		iss >> sysTime.wHour;
		iss.clear();
		iss.str(subStrs[4]);
		iss.clear();
		iss >> sysTime.wMinute;
		iss.clear();
		iss.str(subStrs[5]);
		iss >> sysTime.wSecond;
		return sysTime;
	}

	vector<string>& split(const string& s, char delim, vector<string>& elems){
		if (elems.size() > 0){
			elems.clear();
		}
		string tempS = s;
		std::replace(tempS.begin(), tempS.end(), ' ', delim);
		std::stringstream ss(tempS);
		string item;
		while (std::getline(ss, item, delim)){
			//does not include empty sub strings
			if (item.size() > 0){
				elems.push_back(item);
			}
		}
		return elems;
	}

	vector<string> split(const string& s, char delim){
		vector<string> elems;
		split(s, delim, elems);
		return elems;
	}
};


#endif