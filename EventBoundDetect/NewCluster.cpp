#include "stdafx.h"
#include "Cluster.h"
#include <math.h>
#include <algorithm>
#include <sstream>


/*static*/PhotoDensity CCluster::GetPhotoDensity(vector<Photo_Feature_Set>& photos){
	float spanTime = 0;
	PhotoDensity pd;
	//time in photos are recorded in minutes
	int spanDays = photos.size() == 0 ? 0 : 1;
	vector<float> gapTimeVec;
	for (int i = 1; i < photos.size(); i++){
		Photo_Feature_Set photo1 = photos[i - 1];
		Photo_Feature_Set photo2 = photos[i];
		float timeGap = photo2.dTimeStamp - photo1.dTimeStamp; 
		gapTimeVec.push_back(timeGap);
		if (photo2.SysTime.wDay != photo1.SysTime.wDay || 
			photo2.SysTime.wMonth != photo1.SysTime.wMonth ||
			photo2.SysTime.wYear != photo1.SysTime.wYear){
			spanDays++;
			//if two photos are not in the same day, we do not consider the time
			//as photo density
			continue;
		}
		spanTime += timeGap;
	}
	if (photos.size() == 0){
		pd.avgGapTime = -1;
		pd.spanDays = -1;
		pd.burstNum = -1;
	}
	else{
		pd.avgGapTime = spanTime / photos.size();
		pd.spanDays = spanDays;
		int burstNum = 1;
		for (size_t i = 1; i < photos.size(); i++){
			float thisTimeGap = photos[i + 1].dTimeStamp - photos[i].dTimeStamp;
			if (thisTimeGap > pd.avgGapTime){
				burstNum++;
			}
		}
		pd.burstNum = burstNum;
	}
	return pd;
}

void CCluster::BuildEventInfos(vector<vector<int>>& eventIdxs){
	if (eventIdxs.size() == 0){
		return;
	}
	eventInfos.clear();
	for (size_t i = 0; i < eventIdxs.size(); i++){
		EventInfo event;
		event.SetPhotoIndex(eventIdxs[i]);
		event.ComputeEventFeat(this->m_vecPhoto);
		event.eventId = i;
		eventInfos.push_back(event);
	}
}

bool CCluster::IsInSameTrueEvent(EventInfo& e1, EventInfo& e2){
	if (e1.GetPhotoIndex().size() == 0 || e2.GetPhotoIndex().size() == 0){
		return true;
	}
	int idx_first1 = e1.GetPhotoIndex()[0];
	int idx_first2 = e2.GetPhotoIndex()[0];
	int idx_last1 = e1.GetPhotoIndex()[e1.GetPhotoIndex().size() - 1];
	int idx_last2 = e2.GetPhotoIndex()[e2.GetPhotoIndex().size() - 1];
	//if first image or last image is not in the same folder, the two
	//sub events are not in the same groundtruth event
	if (!IsInSameFolder(m_vecPhoto[idx_first1].tszFileName, m_vecPhoto[idx_first2].tszFileName) ||
		!IsInSameFolder(m_vecPhoto[idx_last1].tszFileName, m_vecPhoto[idx_last2].tszFileName)){
		return false;
	}
	return true;
}

void CCluster::GetTrueEventId(){
	if (eventInfos.size() == 0){
		BuildEventInfos(indexOfEventPhotos);
	}
	if (eventInfos.size() == 0){
		return;
	}
	eventInfos[0].trueId = 0;
	for (size_t i = 1; i < eventInfos.size(); i++){
		if (IsInSameTrueEvent(eventInfos[i], eventInfos[i - 1])){
			eventInfos[i].trueId = eventInfos[i - 1].trueId;
		}
		else{
			eventInfos[i].trueId = eventInfos[i - 1].trueId + 1;
		}
	}
}

void CCluster::GetPredEventId(vector<vector<int>>& eventIdxs, float threshold, float timeK, float gpsK){
	if (eventIdxs.size() == 0){
		return;
	}
	if (eventInfos.size() == 0){
		BuildEventInfos(eventIdxs);
	}
	if (globalEventInfo.IsEmpty()){
		ComputeGlobalEventInfo();
	}
	int numNewEvents = 1;
	eventInfos[0].predId = 0;
	//allow to set threshold from txt file.
	for (size_t i = 1; i < eventInfos.size(); i++){
		float sim = EventInfo::CalcSimOf2Events(eventInfos[i], eventInfos[i-1], timeK, gpsK, 0.1, this->globalEventInfo);
//		float sim = EventInfo::CalcSimOf2Events(eventInfos[i], eventInfos[i-1], timeK, gpsK, 0.2);
		if (sim > threshold){
			eventInfos[i].predId = eventInfos[i - 1].predId;
		}
		else{
			eventInfos[i].predId = eventInfos[i - 1].predId + 1;
			numNewEvents++;
		}
	}
}

Performance CCluster::GetPerformance(vector<vector<int>>& eventIdxs, float threshold, float timeK, float gpsK){
	Performance perf;
	float t2f(0), f2f(0), t2t(0), f2t(0);
	GetTrueEventId();
	GetPredEventId(eventIdxs, threshold, timeK, gpsK);
	for (size_t i = 0; i < eventInfos.size() - 1; i++){
		bool isPredB = EventInfo::IsPredBoundary(eventInfos[i], eventInfos[i + 1]);
		bool isTrueB = EventInfo::IsTrueBoundary(eventInfos[i], eventInfos[i + 1]);
		if (isTrueB && isPredB){
			t2t++;
		}
		else if (isTrueB && !isPredB){
			t2f++;
		}
		else if (!isTrueB && isPredB){
			f2t++;
		}
	}
	if (t2t + f2t == 0){
		perf.precision = 0;
	}
	else{
		perf.precision = t2t / (t2t + f2t);
	}
	if (t2t + t2f == 0){
		perf.recall = 0;
	}
	else{
		perf.recall = t2t / (t2t + t2f);
	} 
	if (perf.precision + perf.recall == 0){
		perf.FScore = 0;
	}
	else{
		perf.FScore = 2 * perf.precision * perf.recall / (perf.precision + perf.recall);
	}
	return perf;
}

//overloading: vector_a + vector_b by each elements
template<typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b){
	assert(a.size() == b.size());
	std::vector<T> result;
	result.reserve(a.size());
	std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result), std::plus<T>());
	return result;
}

//overloading: minus by each element
template<typename T>
std::vector<T> operator-(const std::vector<T>& a, const std::vector<T>& b){
	assert(a.size() == b.size());
	std::vector<T> result;
	result.reserve(a.size());
	std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result), std::minus<T>());
	return result;
}

//overloading: multiply by each element
template<typename T>
std::vector<T> operator*(const std::vector<T>& a, const std::vector<T>& b){
	assert(a.size() == b.size());
	std::vector<T> result;
	result.reserve(a.size());
	std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(result), std::multiplies<T>());
	return result;
}

//overloading: divide by each element
template<typename T>
std::vector<T> operator/(const std::vector<T>& a, T num){
	assert(num != 0);
	std::vector<T> result;
	result.resize(a.size());
	for (size_t i = 0; i < a.size(); i++){
		result[i] = a[i] / num;
	}
	return result;
}

template<typename T>
void Vec2Sqrt(vector<T>& vec){
	for (size_t i = 0; i < vec.size(); i++){
		vec[i] = sqrt(vec[i]);
	}
}


void CCluster::ComputeGlobalEventInfo(){
	globalEventInfo.clear();
	//get average values
	EventInfo currEvent, prevEvent;
	Mat timeFeatMat, distFeatMat, speedFeatMat;
	if (eventInfos.size() == 0){
		return;
	}
	//only consider sub event in 1 month
	//window size: 1 month
	//TODO: tune this parameter
	float windowTime = 1 * 10 * 24 * 60;
	int numInfos = 0, numTimes = 0, numDists = 0, numBvs = 0,
		numGpsGaps = 0, numSpeeds = 0;
	for (size_t i = 0; i < eventInfos.size(); i++){
		//deal with null 
		currEvent = eventInfos[i];
		if (!isVecEmpty(currEvent.timeFeat)){
			globalEventInfo.timeFeatAvg = globalEventInfo.timeFeatAvg + currEvent.timeFeat;
			numTimes++;
		}
		if (!isVecEmpty(currEvent.distFeat)){
			globalEventInfo.distFeatAvg = globalEventInfo.distFeatAvg + currEvent.distFeat;
			numDists++;
		}
		if (!isVecEmpty(currEvent.speedFeat)){
			globalEventInfo.speedFeatAvg = globalEventInfo.speedFeatAvg + currEvent.speedFeat;
			numSpeeds++;
		}
		if (!isVecEmpty(currEvent.bvFeat)){
			globalEventInfo.bvFeatAvg = globalEventInfo.bvFeatAvg + currEvent.bvFeat;
			numBvs++;
		}
		if (i == 0){
			continue;
		}
//		cout << eventInfos[i].avgTime - eventInfos[i - 1].avgTime << "\t";
		if (eventInfos[i].avgTime - eventInfos[i - 1].avgTime > windowTime){
			continue;
		}
		numInfos++;
		prevEvent = eventInfos[i - 1];
		float timeGap = currEvent.avgTime - prevEvent.avgTime;
		globalEventInfo.timeGapAvg += timeGap;
		if (!prevEvent.avgGps.IsEmpty() && !currEvent.avgGps.IsEmpty()){
			float distGap = GPSInfo::Distof2GPS(prevEvent.avgGps, currEvent.avgGps);
			globalEventInfo.distGapAvg += distGap;
			numGpsGaps++;
		}
	}
	//avoid zero denomitor
	numTimes = numTimes > 0 ? numTimes : 1;
	numDists = numDists > 0 ? numDists : 1;
	numSpeeds = numSpeeds > 0 ? numSpeeds : 1;
	numBvs = numBvs > 0 ? numBvs : 1;
	numGpsGaps = numGpsGaps > 0 ? numGpsGaps : 1;
	numInfos = numInfos > 0 ? numInfos : 1;
	globalEventInfo.timeFeatAvg = globalEventInfo.timeFeatAvg / float(numTimes);
	globalEventInfo.distFeatAvg = globalEventInfo.distFeatAvg / float(numDists);
	globalEventInfo.speedFeatAvg = globalEventInfo.speedFeatAvg / float(numSpeeds);
	globalEventInfo.bvFeatAvg = globalEventInfo.bvFeatAvg / float(numBvs);
	globalEventInfo.timeGapAvg /= numInfos;
	globalEventInfo.distGapAvg /= numGpsGaps;
	//get variance values
	for (size_t i = 0; i < eventInfos.size(); i++){
		currEvent = eventInfos[i];
		//deal with missing information
		if (!isVecEmpty(currEvent.bvFeat)){
			globalEventInfo.bvFeatVar = globalEventInfo.bvFeatVar +
				(currEvent.bvFeat - globalEventInfo.bvFeatAvg) * (currEvent.bvFeat - globalEventInfo.bvFeatAvg);
		}
		if (!isVecEmpty(currEvent.timeFeat)){
			globalEventInfo.timeFeatVar = globalEventInfo.timeFeatVar +
				(currEvent.timeFeat - globalEventInfo.timeFeatAvg) * (currEvent.timeFeat - globalEventInfo.timeFeatAvg);
		}
		if (!isVecEmpty(currEvent.distFeat)){
			globalEventInfo.distFeatVar = globalEventInfo.distFeatVar +
				(currEvent.distFeat - globalEventInfo.distFeatAvg) * (currEvent.distFeat - globalEventInfo.distFeatAvg);
			globalEventInfo.speedFeatVar = globalEventInfo.speedFeatVar +
				(currEvent.speedFeat - globalEventInfo.speedFeatAvg) * (currEvent.speedFeat - globalEventInfo.speedFeatAvg);
		}
	}
	globalEventInfo.timeFeatVar = globalEventInfo.timeFeatVar / float(numTimes);
	globalEventInfo.distFeatVar = globalEventInfo.distFeatVar / float(numDists);
	globalEventInfo.speedFeatVar = globalEventInfo.speedFeatVar / float(numSpeeds);
	globalEventInfo.bvFeatVar = globalEventInfo.bvFeatVar / float(numBvs);
	//standard variance
	Vec2Sqrt<float>(globalEventInfo.timeFeatVar);
	Vec2Sqrt<float>(globalEventInfo.distFeatVar);
	Vec2Sqrt<float>(globalEventInfo.speedFeatVar);
	Vec2Sqrt<float>(globalEventInfo.bvFeatVar);
}

void CCluster::MergeEvents2Scale(vector<vector<int>>& eventIdxs, float threshold, float timeK, float gpsK){
	//merge by similarity
	GetPredEventId(eventIdxs,threshold,timeK,gpsK);
	//update event ids
	for (size_t i = 0; i < eventInfos.size(); i++){
		vector<int> thisEventIdx = eventInfos[i].GetPhotoIndex();
		for (size_t j = 0; j < thisEventIdx.size(); j++){
			this->m_vecPhoto[thisEventIdx[j]].iEventLabel = eventInfos[i].predId;
		}
	}
	//build event index vector
	this->BuildIndex();
	//build event info vector
	this->BuildEventInfos(this->indexOfEventPhotos);
}

bool CCluster::ExportEventInfo2Xml(const string& featFile){
	if (eventInfos.size() == 0){
		BuildEventInfos(indexOfEventPhotos);
	}
	pugi::xml_document doc;
	pugi::xml_node root_node = doc.append_child("EventInfo");
	root_node.append_attribute("descr").set_value("Event Information");
	for (size_t i = 0; i < eventInfos.size(); i++){
		cout << i << "\t";
		pugi::xml_node eventNode = root_node.append_child("Event");
		eventInfos[i].ExportAsXmlNode(eventNode);
	}
	if (!doc.save_file(featFile.c_str())){
		cerr << "Open or Create file " << featFile << " failed!\n";
		return false;
	}
	return true;
}

bool CCluster::ExportEventInfo2Txt(const string& txtFile){
	ofstream of(txtFile);
	if (eventInfos.size() == 0){
		BuildEventInfos(indexOfEventPhotos);
	}
	for (size_t i = 0; i < eventInfos.size(); i++){
		eventInfos[i].ExportToTxtFile(of);
	}
	of.close();
	return true;
}

void CCluster::PrintGroundTruth(){
	if (this->eventInfos.size() == 0){
		BuildEventInfos(indexOfEventPhotos);
	}
	int numInTrueEvent = 0;

	for (size_t i = 0; i < eventInfos.size(); i++){
		if (eventInfos[i].PrintGroudTruth(this->m_vecPhoto)){
			numInTrueEvent++;
		}
	}
	cout << "proportion: " << float(numInTrueEvent) / eventInfos.size() << "\n";
}

bool CCluster::LoadEventInfo(const string& featFile){
	eventInfos.clear();
	indexOfEventPhotos.clear();
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(featFile.c_str());
	if (!result){
		cerr << "Open or Load file " << featFile << " failed!\n";
		return false;
	}
	pugi::xml_node rootNode = doc.child("EventInfo");
	for (pugi::xml_node event = rootNode.child("Event"); event; event = event.next_sibling("Event")){
		EventInfo eventInfo;
		eventInfo.LoadDataFromXmlNode(event);
		eventInfos.push_back(eventInfo);
		indexOfEventPhotos.push_back(eventInfo.GetPhotoIndex());
	}
	return true;
}

void SwapEvent(int i, int j, vector<vector<int>>& eventIdx){
	vector<int> tmp = eventIdx[i];
	eventIdx[i] = eventIdx[j];
	eventIdx[j] = tmp;
}

void QuickSortEvent(vector<vector<int>>& eventIdx, vector<Photo_Feature_Set>& photos,
	int left, int right){
	if (left >= right){
		return;
	}
	const int nSize = (int)eventIdx.size();
	double lTime = photos[eventIdx[left][0]].dTimeStamp;
	int i = left;
	int j = right + 1;
	do{
		do i++;
		while ((i < nSize) && (photos[eventIdx[i][0]].dTimeStamp < lTime));
		do j--;
		while ((j< nSize) && (photos[eventIdx[j][0]].dTimeStamp > lTime));
		if (i < j){
			SwapEvent(i, j, eventIdx);
		}
	} while (i < j);
	SwapEvent(left, j, eventIdx);

	QuickSortEvent(eventIdx, photos, left, j - 1);
	QuickSortEvent(eventIdx, photos, j + 1, right);
}


//merge photos1 and photos2 into photos, ordered by time
//then put them into photos1
bool CCluster::MergeTwoSegSet(vector<Photo_Feature_Set>& photos1, vector<vector<int>>& eventIdx1,
	vector<Photo_Feature_Set>& photos2, vector<vector<int>>& eventIdx2){
	if (photos2.size() == 0 || eventIdx2.size() == 0){
		return false;
	}
	int size1 = photos1.size();
	for (size_t i = 0; i < eventIdx2.size(); i++){
		for (size_t p = 0; p < eventIdx2[i].size(); p++){
			photos1.push_back(photos2[eventIdx2[i][p]]);
			eventIdx2[i][p] += size1;
		}
		eventIdx1.push_back(eventIdx2[i]);
	}
	QuickSortEvent(eventIdx1, photos1, 0, eventIdx1.size() - 1);
	return true;
}

//TODO: consider altitude
float DistofTwoGPS(float lat1, float long1, float lat2, float long2, float alt1, float alt2){
	float dlong = (long2 - long1) * D2R;
	float dlat = (lat2 - lat1) * D2R;
	float a1 = sin(dlat / 2.0);
	float a2 = sin(dlong / 2.0);
	float a = a1 * a1 + cos(lat1 * D2R) * cos(lat2 * D2R) * a2 * a2;
	float c = 2 * atan2(sqrt(a), sqrt(1 - a));
	//in meters
	float d = 6367 * c * 1000;
	//approximate
	float finalD = sqrt(d * d + (alt2 - alt1) * (alt2 - alt1));
	return finalD;
}

//return the median value in vector
float GetMedianOfVec(vector<float> data){
	if (data.size() == 0){
		return 0;
	}
	else{
		std::nth_element(data.begin(), data.begin() + data.size() / 2, data.end());
		return data[data.size() / 2];
	}
}

float GetAvgofVec(vector<float> data){
	float avg = 0;
	for (size_t i = 0; i < data.size(); i++){
		avg += data[i];
	}
	if (data.size() == 0){
		return -1;
	}
	else{
		return avg / data.size();
	}
}

float GetVarofVec(vector<float> data, float avg){
	float sumVar = 0;
	for (size_t i = 0; i < data.size(); i++){
		float a = data[i] - avg;
		sumVar += a * a;
	}
	if (data.size() == 0){
		return -1;
	}
	else{
		return sqrt(sumVar / data.size());
	}
}


float FusionMultiviewDecision(vector<float>& multiVec, float threshold, vector<float>& weight){
	int validNum = 0;
	float sumScore(0), denomitor(0);
	if (weight.size() != multiVec.size()){
		weight.resize(multiVec.size(), 1.);
	}
	//set threshold from outside txt file
	ifstream inTh("fusThre.txt"); 
	if (inTh.is_open()){
		inTh >> threshold;
	}
	inTh.close();
	//only consider values larger than threshold
	for (size_t i = 0; i < multiVec.size(); i++){
		if (multiVec[i] > threshold && weight[i] != 0){
			validNum++;
			sumScore += weight[i] * multiVec[i];
			denomitor += weight[i];
		}
	}
	if (validNum == 0 || denomitor == 0){
		return 0;
	}
	return sumScore / denomitor;
}

void InputWeightsFromTxt(const string& txtFile, vector<float>& weight, int dim){
	ifstream in(txtFile);
	if (!in.is_open()){
		weight.resize(dim, 1.);
	}
	else{
		float data;
		for (int i = 0; in >> data && i < dim; i++){
			weight[i] = data;
		}
	}
	in.close();
}

float EventInfo::CalcSimOf2Events(EventInfo& event1, EventInfo& event2, float timeK, float gpsK, float threSim,
	GlobalEventInfo& gEventInfo){
	//the weight of 5 similarity views
	//TODO: learn more resonable weights
	vector<float> weight(6, 1.);
	InputWeightsFromTxt("weight.txt", weight, 6);
	//the number of valid similarities
	int numView = 0;
	float avgGpsSim(0), avgTimeSim(0), taggingSim(0);
	float spTimeSim(0), spGPSSim(0), spSpeedSim(0), spBvSim(0);
	//average time
	avgTimeSim = exp(-abs(event1.avgTime - event2.avgTime) /  (timeK * gEventInfo.timeGapAvg));
	//time gap distribution
	spTimeSim = GetSimOfTwoVecNorm(event1.timeFeat, event2.timeFeat, gEventInfo.timeFeatAvg, gEventInfo.timeFeatVar);
	//gps similarity
	float distGPS = GPSInfo::Distof2GPS(event1.avgGps, event2.avgGps);
	avgGpsSim = distGPS < 0 ? 0 : exp(- distGPS / (gpsK * gEventInfo.distGapAvg));
	//distance and speed similarity
	spGPSSim = GetSimOfTwoVecNorm(event1.distFeat, event2.distFeat, gEventInfo.distFeatAvg, gEventInfo.distFeatVar);
	spSpeedSim = GetSimOfTwoVecNorm(event1.speedFeat, event2.speedFeat, gEventInfo.speedFeatAvg, gEventInfo.speedFeatVar);
	spBvSim = GetSimOfTwoVecNorm(event1.bvFeat, event2.bvFeat, gEventInfo.bvFeatAvg, gEventInfo.bvFeatVar);
	vector<float> decScores;
	decScores.push_back(avgTimeSim);
	decScores.push_back(spTimeSim);
	decScores.push_back(avgGpsSim);
	decScores.push_back(spGPSSim);
	decScores.push_back(spSpeedSim);
	decScores.push_back(spBvSim);
	//we only consider the similarity that exceed given threshold
	float finalScore = FusionMultiviewDecision(decScores, threSim, weight);
//	cout << avgTimeSim << " " << spTimeSim << " " << avgGpsSim << " " << spGPSSim <<
//		" " << spSpeedSim << " "<< spBvSim << " " << finalScore << "\n";
	return finalScore;
}

float EventInfo::CalcSimOf2Events(EventInfo& event1, EventInfo& event2, float timeK, float gpsK, float threSim){
	//the weight of 5 similarity views
	//TODO: learn more resonable weights
	vector<float> weight(5, 1.);
	InputWeightsFromTxt("weight.txt", weight, 5);
	//the number of valid similarities
	float avgGpsSim(0), avgTimeSim(0), taggingSim(0);
	float spTimeSim(0), spGPSSim(0), spSpeedSim(0);
	//we only consider the similarity that exceed given threshold
	avgTimeSim = exp(-abs(event1.avgTime - event2.avgTime) /  (timeK * 1440));
	spTimeSim = GetSimOfTwoVec2(event1.timeFeat, event2.timeFeat);
	float distGPS = GPSInfo::Distof2GPS(event1.avgGps, event2.avgGps);
	avgGpsSim = distGPS < 0 ? 0 : exp(- distGPS / (gpsK * 1000));
	spGPSSim = GetSimOfTwoVec2(event1.distFeat, event2.distFeat);
	spSpeedSim = GetSimOfTwoVec2(event1.speedFeat, event2.speedFeat);
	vector<float> decScores;
	decScores.push_back(avgTimeSim);
	decScores.push_back(spTimeSim);
	decScores.push_back(avgGpsSim);
	decScores.push_back(spGPSSim);
	decScores.push_back(spSpeedSim);
	cout << avgTimeSim << " " << spTimeSim << " " << avgGpsSim << " " << spGPSSim << " " << spSpeedSim << "\t";
	float finalSim = FusionMultiviewDecision(decScores, threSim, weight);

	return finalSim;
}

template<typename T>
string Vector2String(vector<T> vec){
	if (vec.size() == 0){
		return "";
	}
	ostringstream oss;
	oss << vec[0];
	for (int i = 1; i < vec.size(); i++){
		oss << ":" << vec[i];
	}
	return oss.str();
}
string Vector2String(vector<int> vec);
string Vector2String(vector<float> vec);

template<typename T>
vector<T> String2Vector(string& str){
	vector<T> dataVec;
	if (str.size() == 0){
		return dataVec;
	}
	std::istringstream iss;
	vector<string> subStrs = split(str, ':');
	if (subStrs.size() == 0){
		return dataVec;
	}
	T data;
	for (auto it = subStrs.begin(); it != subStrs.end(); ++it){
		iss.clear();
		iss.str(*it);
		iss >> data;
		dataVec.push_back(data);
	}
	return dataVec;
}

void EventInfo::ExportAsXmlNode(pugi::xml_node& eventNode){
	eventNode.append_child("id").text().set(this->eventId);
	eventNode.append_child("trueId").text().set(this->trueId);
	eventNode.append_child("predId").text().set(this->predId);
	eventNode.append_child("photoIndex").text().set(Vector2String<int>(this->photoIndex).c_str());
	eventNode.append_child("avgTime").text().set(avgTime);
	eventNode.append_child("timeFeat").text().set(Vector2String<float>(this->timeFeat).c_str());
	eventNode.append_child("distFeat").text().set(Vector2String<float>(this->distFeat).c_str());
	eventNode.append_child("speedFeat").text().set(Vector2String<float>(this->speedFeat).c_str());
	eventNode.append_child("brightFeat").text().set(Vector2String<float>(this->bvFeat).c_str());
	eventNode.append_child("avgGPS").text().set(this->avgGps.ToString().c_str());
	eventNode.append_child("photoNum").text().set(this->photoNum);
	eventNode.append_child("numGps").text().set(this->numGps);
	eventNode.append_child("numSpeeds").text().set(this->numSpeeds);
}

void EventInfo::LoadDataFromXmlNode(pugi::xml_node& eventNode){
	eventId = atoi(eventNode.child_value("id"));
	trueId = atoi(eventNode.child_value("trueId"));
	predId = atoi(eventNode.child_value("predId"));
	string str = eventNode.child_value("photoIndex");
	photoIndex = String2Vector<int>(str);
	str = eventNode.child_value("timeFeat");
	timeFeat = String2Vector<float>(str);
	str = eventNode.child_value("distFeat");
	distFeat = String2Vector<float>(str);
	str = eventNode.child_value("speedFeat");
	speedFeat = String2Vector<float>(str);
	str = eventNode.child_value("avgGPS");
	avgGps.FromString(str);
	str = eventNode.child_value("brightFeat");
	bvFeat = String2Vector<float>(str);
	avgTime = atof(eventNode.child_value("avgTime"));
	photoNum = atoi(eventNode.child_value("photoNum"));
	numGps = atoi(eventNode.child_value("numGps"));
	numSpeeds = atoi(eventNode.child_value("numSpeeds"));
}

void EventInfo::SetPhotoIndex(vector<int>& photoIndex){
	photoNum = photoIndex.size();
	this->photoIndex = photoIndex;
}

void EventInfo::ComputeTaggingFeat(vector<Photo_Feature_Set>& photos){
	if (photos.size() < photoIndex.size()){
		return;
	}
	int start1 = 0, end1 = 0.1 * photoIndex.size();
	int start2 = 0.9 * photoIndex.size(), end2 = photoIndex.size();
	//maximum of 20 photos
	if (photoIndex.size() > 100){
		end1 = 10;
		start2 = photoIndex.size() - 10;
	}
	vector<float> tmpTagging;
	taggingFeat.resize(118, 0);
	int numFeat = 0;
	for (int i = start1; i < end1; i++){
		string imgPath = photos[photoIndex[i]].tszFileName;
		string cmd = "ExtractTagFeat.exe \"" + imgPath + "\"";
		string taggingPath = imgPath + ".tagging.txt";
		if (!FileExist(taggingPath)){
			system(cmd.c_str());
		}
		LoadFeatInTxt(taggingPath, tmpTagging);
		if (tmpTagging.size() == taggingFeat.size()){
			taggingFeat = tmpTagging + taggingFeat;
			numFeat++;
		}
		//TODO: do not add new file in released version
//		std::remove(taggingPath.c_str());
	}
	if (start2 > end1){
		for (int i = start2; i < end2; i++){
			string imgPath = photos[photoIndex[i]].tszFileName;
			string cmd = "ExtractTagFeat.exe \"" + imgPath + "\"";
			string taggingPath = imgPath + ".tagging.txt";
			if (!FileExist(taggingPath)){
				system(cmd.c_str());
			}
			LoadFeatInTxt(taggingPath, tmpTagging);
			if (tmpTagging.size() == taggingFeat.size()){
				taggingFeat = tmpTagging + taggingFeat;
				numFeat++;
			}
			//TODO: delete extra files in release version
//			std::remove(taggingPath.c_str());
		}
	}
	taggingFeat = taggingFeat / float(numFeat);
}

void EventInfo::ComputeEventFeat(vector<Photo_Feature_Set>& photos){
	if (photos.size() < photoIndex.size()){
		return;
	}
	float midTimeGap(1e6), maxTimeGap(0),varTimeGap(0), avgTimeGap(0);
	float midDist(1e6), maxDist(0), varDist(0), avgDist(0);
	float midSpeed(1e6), maxSpeed(0), varSpeed(0), avgSpeed(0);
	float avgTime(0), avgLat(0), avgAlt(0), avgLong(0);
	vector<float> timeDiffs, distDiffs, speeds, brightVals;
	vector<float> taggingInfo;
	int numGps = 0, numSpeeds = 0, numBv(0);
	avgTime += photos[photoIndex[0]].dTimeStamp;
	int Idx0 = photoIndex[0];
	if (photos[Idx0].latitude != 0 || photos[Idx0].longitude != 0 || photos[Idx0].atitude != 0){
		avgAlt += photos[Idx0].atitude;
		avgLong += photos[Idx0].longitude;
		avgLat += photos[Idx0].latitude;
		numGps++;
	}
	float bv0 = photos[0].brightValue;
	if (bv0 != NULL){
		brightVals.push_back(bv0);
		numBv++;
	}
	for (size_t i = 1; i < photoIndex.size(); i++){
//		cout << i << "\t";
		//time related
		int idx1 = photoIndex[i - 1];
		int idx2 = photoIndex[i];
		float time1 = photos[idx1].dTimeStamp;
		float time2 = photos[idx2].dTimeStamp;
		avgTime += time2;
		timeDiffs.push_back(time2 - time1);

		//camera(brightness) related
		float bv2 = photos[idx2].brightValue;
		if (bv2 != NULL){
			brightVals.push_back(bv2);
			numBv++;
		}

		//gps related
		float lat1 = photos[idx1].latitude;
		float alt1 = photos[idx1].atitude;
		float lon1 = photos[idx1].longitude;
		float lat2 = photos[idx2].latitude;
		float alt2 = photos[idx2].atitude;
		float lon2 = photos[idx2].longitude;
		if (lat2 == 0 && alt2 == 0 && lon2 == 0){
			continue;
		}
		avgLat += lat2;
		avgLong += lon2;
		avgAlt += alt2;
		numGps++;
		if (lat1 == 0 && alt1 == 0 && lon1 == 0){
			continue;
		}
		numSpeeds++;
		float dist = DistofTwoGPS(lat1, lon1, lat2, lon2, alt1, alt2);
		//it's possible to set the timestamp of a photo to be modified time
		//could be the same
		speeds.push_back(time2 - time1 == 0 ? 0: dist / (time2 - time1));
		distDiffs.push_back(dist);
	}

	//general features
	this->photoNum = photoIndex.size();
	this->numGps = numGps;
	this->numSpeeds = numSpeeds;
	this->avgTime = avgTime / this->photoNum;
	this->avgGps.latitude = numGps == 0 ? 0: avgLat / numGps;
	this->avgGps.longitude = numGps == 0 ? 0: avgLong / numGps;
	this->avgGps.altitude = numGps == 0 ? 0:  avgAlt / numGps;

	//time statistics
	if (photoIndex.size() < 2){
		return;
	}
	midTimeGap = GetMedianOfVec(timeDiffs);
	maxTimeGap = *std::max_element(timeDiffs.begin(), timeDiffs.end());
	avgTimeGap = GetAvgofVec(timeDiffs);
	varTimeGap = GetVarofVec(timeDiffs, avgTimeGap);
	timeFeat.push_back(midTimeGap);
	timeFeat.push_back(maxTimeGap);
	timeFeat.push_back(avgTimeGap);
	timeFeat.push_back(varTimeGap);

	//location statistics
	if (numSpeeds == 0){
		distFeat.clear();
	}
	else{
		midDist = GetMedianOfVec(distDiffs);
		maxDist = *std::max_element(distDiffs.begin(), distDiffs.end());
		avgDist = GetAvgofVec(distDiffs);
		varDist = GetVarofVec(distDiffs, avgDist);
		distFeat.push_back(midDist);
		distFeat.push_back(maxDist);
		distFeat.push_back(avgDist);
		distFeat.push_back(varDist);
	}

	//speeds statistics
	if (numSpeeds == 0){
		speedFeat.clear();
	}
	else{
		midSpeed = GetMedianOfVec(speeds);
		maxSpeed = *std::max_element(speeds.begin(), speeds.end());
		avgSpeed = GetAvgofVec(speeds);
		varSpeed = GetVarofVec(speeds, avgSpeed);
		speedFeat.push_back(midSpeed);
		speedFeat.push_back(maxSpeed);
		speedFeat.push_back(avgSpeed);
		speedFeat.push_back(varSpeed);
	}
	//camera statistics
	if (numBv == 0){
		bvFeat.clear();
	}
	else{
		float midBvVal = GetMedianOfVec(brightVals);
		float maxBvVal = *std::max_element(brightVals.begin(), brightVals.end());
		float avgBvVal = GetAvgofVec(brightVals);
		float varBvVal = GetVarofVec(brightVals, avgBvVal);
		bvFeat.push_back(midBvVal);
		bvFeat.push_back(maxBvVal);
		bvFeat.push_back(avgBvVal);
		bvFeat.push_back(varBvVal);
	}
//	ComputeTaggingFeat(photos);
}

bool EventInfo::LoadFeatInTxt(const string& path, vector<float>& vec){
	ifstream inf(path);
	vec.clear();
	if (!inf.is_open()){
		return false;
	}
	float feat;
	while (inf >> feat){
		vec.push_back(feat);
	}
	inf.close();
	return true;
}

template<typename T>
void PrintVec(vector<T> vec, int reqDim, ofstream& os){
	if (vec.size() == reqDim){
		for (size_t i = 0; i < reqDim; i++){
			os << vec[i] << "\t";
		}
	}
	else{
		for (size_t i = 0; i < reqDim; i++){
			os << 0 << "\t";
		}
	}
}

void EventInfo::ExportToTxtFile(ofstream& os){
	os << eventId << "\t" << trueId <<"\t" << predId << "\t";
	PrintVec<float>(timeFeat, 4, os);
	PrintVec<float>(distFeat, 4, os);
	PrintVec<float>(speedFeat, 4, os);
	PrintVec<float>(bvFeat, 4, os);
	os << avgTime << "\t";
	os << avgGps.altitude << "\t" << avgGps.latitude << "\t" << avgGps.longitude << "\t";
	os << photoNum << "\t";
	os << numGps << "\t";
	os << numSpeeds << "\n";
	//make sure print to disk
	os.flush();
}

bool EventInfo::IsInSameTrueEvent(vector<Photo_Feature_Set>& photos){
	for (size_t i = 0; i < photoIndex.size() - 1; i++){
		if (!IsInSameFolder(photos[photoIndex[i]].tszFileName, photos[photoIndex[i + 1]].tszFileName)){
			cout << photos[photoIndex[i]].tszFileName << "\t" << photos[photoIndex[i + 1]].tszFileName << "\n";
			return false;
		}
	}
	return true;
}

bool EventInfo::PrintGroudTruth(vector<Photo_Feature_Set>& photos){
	bool sf = IsInSameTrueEvent(photos);
	cout << eventId << " " << sf << "\n";
	return sf;
}

bool EventInfo::IsPredBoundary(EventInfo& currEvent, EventInfo& nextEvent){
	return currEvent.predId != nextEvent.predId;
}

bool EventInfo::IsTrueBoundary(EventInfo& currEvent, EventInfo& nextEvent){
	return currEvent.trueId != nextEvent.trueId;
}

