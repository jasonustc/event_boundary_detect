#include "Cluster.h"

void EvaluateSegment::GetGroundTrueEventIdx(){
	//already have groundtruth event index
	if (trueEventIndex.size() > 0){
		return;
	}
	vector<string> folders;
	for (size_t i = 0; i < photos.size(); i++){
		string folder, file;
		GetDirectoryName(photos[i].tszFileName, folder, file);
		//new groundtruth event
		int e = std::find(folders.begin(), folders.end(), folder) - folders.begin();
		if ( e == folders.size()){
			vector<int> eventIdx;
			eventIdx.push_back(i);
			trueEventIndex.push_back(eventIdx);
			folders.push_back(folder);
		}
		else{
			trueEventIndex[e].push_back(i);
		}
	}
}

void GetEventFolderName(const string& path, string& folder){
	int loc1 = path.find("\\");
	int loc2 = path.find("\\", loc1 + 1);
	if (loc2 == path.size() || loc2 - loc1 < 2){
		folder = "";
		return;
	}
	folder = path.substr(loc1 + 1, loc2 - loc1 - 1);
}

void EvaluateSegment::GetGroundTrueEventIdx2(){
	//already have groundtruth event index
	if (trueEventIndex.size() > 0){
		return;
	}
	vector<string> folders;
	for (size_t i = 0; i < photos.size(); i++){
		string folder, file;
		GetEventFolderName(photos[i].tszFileName, folder);
		if (folder.size() < 1){
			continue;
		}
		//new groundtruth event
		int e = std::find(folders.begin(), folders.end(), folder) - folders.begin();
		if ( e == folders.size()){
			vector<int> eventIdx;
			eventIdx.push_back(i);
			trueEventIndex.push_back(eventIdx);
			folders.push_back(folder);
			cout << folder << "\n";
		}
		else{
			trueEventIndex[e].push_back(i);
		}
	}
}
//return the median value in vector
double GetMedianOfVec(vector<double> data){
	if (data.size() == 0){
		return 0;
	}
	else{
		std::nth_element(data.begin(), data.begin() + data.size() / 2, data.end());
		return data[data.size() / 2];
	}
}

double GetAvgofVec(vector<double> data){
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

double EvaluateSegment::ConfValueOf2Event(vector<int>& event1, vector<int>& event2){
	float mean1, median1, mean2, median2;
	vector<double> time1, time2;
	for (size_t i = 0; i < event1.size(); i++){
		time1.push_back(photos[event1[i]].dTimeStamp);
	}
	for (size_t j = 0; j < event2.size(); j++){
		time2.push_back(photos[event2[j]].dTimeStamp);
	}
	mean1 = GetAvgofVec(time1);
//	median1 = GetMedianOfVec(time1);
	mean2 = GetAvgofVec(time2);
//	median2 = GetMedianOfVec(time2);
	return std::abs(mean1 - mean2);
}

void EvaluateSegment::BuildConfMatrix(){
	confMatrix.clear();
	//for every generated album, find the closest annotated album
	for (size_t i = 0; i < predEventIndex.size(); i++){
		vector<float> confs;
		for (size_t j = 0; j < trueEventIndex.size(); j++){
			float confValue = ConfValueOf2Event(predEventIndex[i], trueEventIndex[j]);
			confs.push_back(confValue);
		}
		confMatrix.push_back(confs);
	}
}

void EvaluateSegment::BuildEventPairs(){
	if (confMatrix.size() != predEventIndex.size()){
		printf("confmatrix and predEvnet index size not match\n");
		return;
	}
	eventPairIdx.clear();
	//predicted to true
	for (size_t i = 0; i < confMatrix.size(); i++){
		//pred -> true
		int loc = std::min_element(confMatrix[i].begin(), confMatrix[i].end()) - confMatrix[i].begin();
		eventPairIdx.push_back(loc);
	}
}

Performance EvaluateSegment::GetAlbumPerf(vector<int>& predIdx, vector<int>& trueIdx){
	int TP(0), FP(0), FN(0);
	if (predIdx.size() == 0 || trueIdx.size() == 0){
		printf("empty predIdx or trueIdx\n");
		Performance perf;
		perf.precision = 0;
		perf.recall = 0;
		perf.FScore = 0;
		return perf;
	}
	for (size_t i = 0; i < predIdx.size(); i++){
		for (size_t j = 0; j < trueIdx.size(); j++){
			if (predIdx[i] == trueIdx[j]){
				TP++;
			}
		}
	}
	FP = predIdx.size() - TP;
	FN = trueIdx.size() - TP;
	Performance perf;
	perf.precision = float(TP) / float(TP + FP);
	perf.recall = float(TP) / float(TP + FN);
	perf.ComputeFscore();
	cout << predIdx.size() << "\t" << trueIdx.size() << "\t"
		<< perf.precision << "\t" << perf.recall << "\n";
	return perf;
}

void EvaluateSegment::ComputePerformance(){
	if (eventPairIdx.size() == 0){
		return;
	}
	float avgPrecision(0), avgRecall(0), avgFscore(0), avgAlbumCountSurplus(0);
	for (size_t i = 0; i < eventPairIdx.size(); i++){
		Performance perf = GetAlbumPerf(predEventIndex[i], trueEventIndex[eventPairIdx[i]] );
		pairPerf.push_back(perf);
		avgPrecision += perf.precision;
		avgRecall += perf.recall;
		avgFscore += perf.FScore;
		avgAlbumCountSurplus += perf.AlbumCountSurplus;
	}
	meanPerf.precision = avgPrecision / eventPairIdx.size();
	meanPerf.recall = avgRecall / eventPairIdx.size();
	meanPerf.FScore = avgFscore / eventPairIdx.size();
	float true_size = trueEventIndex.size();
	float pred_size = predEventIndex.size();
	meanPerf.AlbumCountSurplus = (pred_size - true_size) / true_size;
}

void EvaluateSegment::GetPerformance(){
//	GetGroundTrueEventIdx();
	GetGroundTrueEventIdx2();
	BuildConfMatrix();
	BuildEventPairs();
	ComputePerformance();
}