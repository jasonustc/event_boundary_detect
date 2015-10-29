#ifndef CLUSTER_H
#define CLUSTER_H

#include "PhotoInfo.h"
#include <direct.h>
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include "io.h"
#include "3rdparty\pugixml.hpp"
#include "FeatureHelper.h"
#include "FolderFileHelper.h"

#define D2R (3.1415926 / 180)

using namespace cv;

struct timeComperer
{
	inline bool operator()(const Photo_Feature_Set& photo1, const Photo_Feature_Set& photo2){
		return (photo1.dTimeStamp < photo2.dTimeStamp);
	}
};

struct PhotoDensity
{
	int spanDays;
	float avgGapTime;
	int burstNum;
};

struct GPSInfo{

	float latitude;
	float longitude;
	float altitude;

	bool IsEmpty(){
		if (latitude == 0 && longitude == 0 && altitude == 0){
			return true;
		}
		else{
			return false;
		}
	}

	GPSInfo operator+=(const GPSInfo& gps){ latitude += gps.latitude; 
	longitude += gps.longitude; altitude += gps.altitude; return *this;}
	GPSInfo operator-=(const GPSInfo& gps){ latitude -= gps.latitude; 
	longitude -= gps.longitude; altitude -= gps.altitude; return *this;}

	static float Distof2GPS(GPSInfo g1, GPSInfo g2){
		if (g1.IsEmpty() || g2.IsEmpty()){
			return -1;
		}
		float dlong = (g2.longitude - g1.longitude) * D2R;
		float dlat = (g2.latitude - g1.latitude) * D2R;
		float dalt = (g2.altitude - g1.altitude);
		float a1 = sin(dlat / 2.0);
		float a2 = sin(dlong / 2.0);
		float a = a1 * a1 + cos(g1.latitude * D2R) * cos(g2.latitude * D2R) * a2 * a2;
		float c = 2 * atan2(sqrt(a), sqrt(1 - a));
		float d = 6367 * c * 1000;
		float ad = sqrt(d * d + dalt * dalt);
		return ad;
	}

	string ToString(){
		ostringstream oss;
		oss << latitude << ":" << longitude << ":" << altitude;
		return oss.str();
	}

	void FromString(string& str){
		istringstream iss;
		vector<string> subStrs = split(str, ':');
		if (subStrs.size() != 3){
			return;
		}
		iss.str(subStrs[0]);
		iss >> latitude;
		iss.clear();
		iss.str(subStrs[1]);
		iss >> longitude;
		iss.clear();
		iss.str(subStrs[2]);
		iss >> altitude;
	}
};
//GPSInfo operator+(GPSInfo g1, const GPSInfo& g2){ return g1 += g2; }
//GPSInfo operator-(GPSInfo g1, const GPSInfo& g2){ return g1 -= g2; }

struct GlobalEventInfo{
	vector<float> timeFeatVar;
	vector<float> timeFeatAvg;
	vector<float> distFeatVar;
	vector<float> distFeatAvg;
	vector<float> speedFeatVar;
	vector<float> speedFeatAvg;
	vector<float> bvFeatAvg;
	vector<float> bvFeatVar;
	float timeGapAvg;
//	float timeGapVar;
	float distGapAvg;
//	float distGapVar;

	void clear(){
		timeFeatVar.resize(4, 0);
		timeFeatAvg.resize(4, 0);
		distFeatVar.resize(4, 0);
		distFeatAvg.resize(4, 0);
		speedFeatAvg.resize(4, 0);
		speedFeatVar.resize(4, 0);
		bvFeatAvg.resize(4, 0);
		bvFeatVar.resize(4, 0);
		timeGapAvg = 0;
//		timeGapVar = 0;
		distGapAvg = 0;
//		distGapVar = 0;
	}

	bool IsEmpty(){
		if (timeFeatAvg.size() == 0 || distFeatVar.size() == 0 || speedFeatVar.size() == 0){
			return true;
		}
		else{
			return false;
		}
	}

};

class EventInfo
{
private:
	vector<int> photoIndex;

public:
	//sub event id
	int eventId = 0;
	//groundtruth id
	int trueId = 0;
	//after merge
	int predId = 0;
	vector<float> timeFeat;
	vector<float> distFeat;
	vector<float> speedFeat;
	vector<float> bvFeat;
	vector<float> taggingFeat;
	float avgTime;
	GPSInfo avgGps;
	int photoNum;
	int numGps;
	int numSpeeds;
	//event start time and end time
	double startTime;
	double endTime;

public:
	void ComputeEventFeat(vector<Photo_Feature_Set>& photos);
	void SetPhotoIndex(vector<int>& photoIndex);
	vector<int>& GetPhotoIndex(){ return this->photoIndex; }
	static float CalcSimOf2Events(EventInfo& event1, EventInfo& event2, float K, float gpsK, float threSim);
	static float CalcSimOf2Events(EventInfo& event1, EventInfo& event2, float K, float gpsK, 
		float threSim, GlobalEventInfo& gEventInfo);
	void ExportAsXmlNode(pugi::xml_node& eventNode);
	void ExportToTxtFile(ofstream& os);
	bool PrintGroudTruth(vector<Photo_Feature_Set>& photos);
	void LoadDataFromXmlNode(pugi::xml_node& eventNode);
	void ComputeTaggingFeat(vector<Photo_Feature_Set>& photos);
	static bool IsPredBoundary(EventInfo& currEvent, EventInfo& nextEvent);
	static bool IsTrueBoundary(EventInfo& currEvent, EventInfo& nextEvent);

private:
	bool IsInSameTrueEvent(vector<Photo_Feature_Set>& photos);
	bool LoadFeatInTxt(const string& path, vector<float>& vec);
};

struct Performance{
	float precision;
	float recall;
	float FScore;
	float AlbumCountSurplus;
	void ComputeFscore(){
		FScore = 2 * precision * recall / (precision + recall + FLT_EPSILON);
	}
};

class EvaluateSegment{
public:
	EvaluateSegment(vector<Photo_Feature_Set>& photos, vector<vector<int>>& predEventIndex){
		this->photos = photos;
		this->predEventIndex = predEventIndex;
	}

	EvaluateSegment(vector<Photo_Feature_Set>& photos, vector<vector<int>>& predEventIndex,
		vector<vector<int>>& trueEventIndex){
		this->photos = photos;
		this->predEventIndex = predEventIndex;
		this->trueEventIndex = trueEventIndex;
	}

private:
	vector<Performance> pairPerf;
	//pred to true confusion matrix
	vector<vector<float>> confMatrix;
	vector<vector<int>> predEventIndex;
	vector<vector<int>> trueEventIndex;
	vector<Photo_Feature_Set> photos;
	vector<int> eventPairIdx;
	void GetGroundTrueEventIdx();
	void GetGroundTrueEventIdx2();
	void BuildConfMatrix();
	void BuildEventPairs();
	void ComputePerformance();
	double ConfValueOf2Event(vector<int>& event1, vector<int>& event2);

public:
	Performance meanPerf;

public:
	static Performance GetAlbumPerf(vector<int>& predIdx, vector<int>& trueIdx);
	void GetPerformance();
};


class CCluster
{
private:
	void BuildEventInfos(vector<vector<int>>& eventIdxs);
	bool IsInSameTrueEvent(EventInfo& e1, EventInfo& e2);
	void GetTrueEventId();  // event id of user
	void GetPredEventId(vector<vector<int>>& eventIdxs, float threshold, float timeK, float gpsK); //event id after merge

public:
	CCluster(vector<Photo_Feature_Set> &m_vecPhotos);
	~CCluster();
	HRESULT Clustering(bool use_gps);
    void GetEventIndex(vector<vector<int>> &eventPhotoIndex);
    void GetPhotoFeatures(vector<Photo_Feature_Set> &photos);
	Performance GetPerformance(vector<vector<int>>& eventIdxs, float threshold, float timeK, float gpsK);

	bool MergeTwoSegSet(vector<Photo_Feature_Set>& photos1, vector<vector<int>>& eventIdx1,
		vector<Photo_Feature_Set>& photos2, vector<vector<int>>& eventIdx2);

	//static methods
	static PhotoDensity GetPhotoDensity(vector<Photo_Feature_Set>& photos);
	void MergeEvents2Scale(vector<vector<int>>& events, float threshold,
		float timeK, float gpsK);

	bool ExportEventInfo2Xml(const string& featFile);
	bool ExportEventInfo2Txt(const string& txtFile);
	bool LoadEventInfo(const string& featFile);
	//functions to get scale parameters
	void ComputeGlobalEventInfo();
	void PrintGroundTruth();

private:
	HRESULT Preprocess();
	HRESULT PreprocessNew();
    double GetSecondTime(IN SYSTEMTIME SysTime);
    HRESULT BuildIndex();
	HRESULT MergeEvent();
    void SortPhotos();
	HRESULT EMtraining(Mat & usedSamples, bool use_gps);
	HRESULT LoadFeatures(Mat &samples, bool use_gps);


private:
	vector<Photo_Feature_Set> m_vecPhoto;		// all of the current user's photos
    vector<vector<int>> indexOfEventPhotos;  //index the photos by event
	vector<EventInfo> eventInfos;             //event features
	GlobalEventInfo globalEventInfo;          //global event information

	/////////////////////////////////////////////////////////////////////////////////////
	// These parameters are all for EM iteration
	int m_iInitK;								      // the initial number of event
	int m_iK;									     // kth event
	int m_iN;									     // the number of photo, m_iN = m_vecPhoto.size()
	int iTotalEvent;                                 // K_max: max number of events

	double m_flCurrML;                          // Maximum Likelihood : log(p(X | Theta))
	float *m_pPrevPe;							// loop t:     P( ej ), ( k * 1 )
	float *m_pCurrPe;							// loop t+1:   P( ej ), ( k * 1 )
	float **m_ppPrevPxe;						// loop t:     P( xi | ej ), ( k * N )
	float **m_ppCurrPxe;						// loop t+1:   P( xi | ej ), ( k * N )
	float **m_ppCurrPex;						// loop t+1:   P( ej | xi ), ( k * N )
	
	// The best K and its corresponding p( E j | X i)
	//  MDL Priciple 
	int m_iBestK_MDL;							   // The best K value
	double m_flPrevCost_MDL;							   
	double m_flCurrCost_MDL;
	Mat m_matBestPex_MDL;                         //The best P(E_j | x_i)
	Mat m_matBestLikelihood;
	int* m_pBestLabel_MDL;                        //The best label
	double **m_ppBestPxe_MDL;                     //The best p(x_i | E_j)

//global density
private:
	float global_photo_density_;
};

#endif