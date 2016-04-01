// cluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PhotoProcess.h"
#include "Cluster.h"

#include "3rdparty/pugixml.hpp"
#include "FeatureHelper.h"

#include <fstream>
#include "PreSegment.h"
#include "PostSegCheck.h"

using namespace std;

//TODO: test the code of time and distance computation
//TODO: make all parameters adaptive

void ParseInputFlags(int argc, char** argv, InputConfig& inputConfig);
void PrintEventInfo(vector<vector<int>>& eventIdx);

int main(int argc, char** argv)
{
	//default folder is the images folder in the same direcoty of the executable
	if (argc < 2){
		cout << "Usage: PhotoClustering.exe\n"
			<< "imgDir -ff string [input photo feature file in xml format](optional) \n";
		return -1;
	}

	//parse parameters
	InputConfig inConfig;
	inConfig.tszImageDir = argv[1];
	std::replace(inConfig.tszImageDir.begin(), inConfig.tszImageDir.end(), '\\', '/');
	if (inConfig.tszImageDir.back() != '/'){
		inConfig.tszImageDir.append("/");
	}
	ParseInputFlags(argc, argv, inConfig);
//	string infoFile = inConfig.photoFeatFile + ".info.txt";
//	ofstream ins(infoFile);

	//extract features
	vector<Photo_Feature_Set> photos;
	vector<SimpleEventInfo> simEventInfos;
	vector<vector<int>> eventIdx;
	//final event index and photos
	vector<vector<int>> fEventIdx;
	vector<Photo_Feature_Set> fPhotos;
	if (inConfig.photoFeatFile.size() > 0){
		LoadPhotoFromXml(inConfig.photoFeatFile, photos);
	}
	else if (inConfig.eventFeatXml.size() > 0){
		LoadEventFromXml(inConfig.eventFeatXml, photos, simEventInfos);
	}
	else{
		CPhotoProcess photoProcess;
		photoProcess.ProcessPhotos(inConfig.tszImageDir.c_str());
		photoProcess.GetPhotoFeats(photos);
		//photos do not need event segmentation
		photoProcess.GetOldEventIdx(fEventIdx);
		photoProcess.GetOldPhotoFeats(fPhotos);
	}

	//if no new photos need to do clustering
	if (photos.size() == 0){
		return 0;
	}
	//preprocess the collection to control the time span and photo number
	PreSegment preseg(photos);
	preseg.GetBoundaries();
	vector<vector<int>> preSegEventIdx = preseg.coEventIdx;
	printf("With %ld events after Pre Segment.\n", preSegEventIdx.size());
//	PrintEventInfo(preSegEventIdx);
	for (size_t i = 0; i < preSegEventIdx.size(); i++) {
		vector<Photo_Feature_Set> tmpPhotos;
		vector<vector<int>> tmpEventIdx;
		for (size_t p = 0; p < preSegEventIdx[i].size(); p++){
			tmpPhotos.push_back(photos[preSegEventIdx[i][p]]);
		}
		CCluster cluster(tmpPhotos);
		cluster.Clustering(inConfig.use_gps);
		cluster.GetEventIndex(tmpEventIdx);
		//merge new clustered photos into final result
		cluster.MergeTwoSegSet(fPhotos, fEventIdx, tmpPhotos, tmpEventIdx);
	}
	string clusterEventFile = inConfig.photoFeatFile + ".cluster.event.xml";
	SaveEvent2PhotosAsXml(fEventIdx, fPhotos, clusterEventFile.c_str());
//	ins << "Presegment time: " << ((double)t2 - (double)t1) / CLOCKS_PER_SEC << "\n";
	//merge is a global behavior
	CCluster gCluster(fPhotos);
	gCluster.MergeEvents2Scale(fEventIdx, inConfig.threshold, inConfig.timeK, inConfig.gpsK);
	gCluster.GetEventIndex(fEventIdx);
	string mergeEventFile = inConfig.photoFeatFile + ".merge.event.xml";
	SaveEvent2PhotosAsXml(fEventIdx, fPhotos, mergeEventFile.c_str());
	printf("With %ld events after merged\n", fEventIdx.size());
//	ins << "Merge time: " << ((double)t1 - (double)t2) / CLOCKS_PER_SEC << "\n";
//	PrintEventInfo(fEventIdx);

	//post check
	PostSegCheck postChecker(fPhotos, fEventIdx);
	postChecker.FinalCheck();
	photos = postChecker.GetPhotos();
	eventIdx = postChecker.GetEventIdx();
	printf("With %ld events after final check\n", eventIdx.size());
//	ins << "Check time: " << ((double)t2 - (double)t1) / CLOCKS_PER_SEC << "\n";
//	ins.close();
//	PrintEventInfo(eventIdx);

	//save event information into xml file
	string finalEventFile = inConfig.photoFeatFile + ".final.event.xml";
	SaveEvent2PhotosAsXml(fEventIdx, fPhotos, finalEventFile.c_str());
//	int pathLen = wcslen(inConfig.tszEventSegFile);
//	if (wcscmp(inConfig.tszEventSegFile + pathLen - 3, L"xml") == 0){
//		SaveEvent2PhotosAsXml(eventIdx, photos, inConfig.tszEventSegFile);
//	}
//	else{
//		SaveEvent2PhotosAsText(eventIdx, photos, inConfig.tszEventSegFile);
//	}

	//Compute Performance
//	EvaluateSegment evPerf(photos, eventIdx);
//	evPerf.GetPerformance(infoFile);
//	Performance perf = evPerf.meanPerf;
//	ins.open(infoFile, ios::app);
//	ins << "Precision: " << perf.precision << "\nRecall: " << perf.recall << 
//		"\nF-score: " << perf.FScore << "\nAlbumItemCountSurplus: " << perf.AlbumCountSurplus;
//	ins.close();
//	cout << "Precision: " << perf.precision << "\tRecall: " << perf.recall << 
//		"\tF-score: " << perf.FScore << "\tAlbumItemCountSurplus: " << perf.AlbumCountSurplus;
	return 0;
}

void ParseInputFlags(int argc, char** argv, InputConfig& inputConfig){
	for (int i = 0; i < argc; i++){
		string flag = argv[i];
		if (flag == "-ug"){
			inputConfig.use_gps = stoi(argv[i + 1]);
		}
		else if (flag == "-ps"){
			inputConfig.tszPhotoSegFile = argv[i + 1];
		}
		else if (flag == "-es"){
			inputConfig.tszEventSegFile = argv[i + 1];
		}
		else if (flag == "-K"){
			inputConfig.K = stoi(argv[i + 1]);
		}
		else if (flag == "-th"){
			inputConfig.threshold = stoi(argv[i + 1]);
		}
		else if (flag == "-of"){
			inputConfig.tszOutImageDir = argv[i + 1];
		}
		else if (flag == "-ff"){
			inputConfig.photoFeatFile = argv[i + 1];
		}
		else if (flag == "-tK"){
			inputConfig.timeK = stoi(argv[i + 1]);
		}
		else if (flag == "-gK"){
			inputConfig.gpsK = stoi(argv[i + 1]);
		}
		else if (flag == "-et"){
			inputConfig.eventFileTxt = argv[i + 1];
		}
		else if (flag == "-ef"){
			inputConfig.eventFeatXml = argv[i + 1];
		}
	}
	ifstream inc("inConfig.txt");
	if (inc.is_open()){
		inc >> inputConfig.threshold >> inputConfig.timeK >> inputConfig.gpsK;
	}
	inc.close();
}

void PrintEventInfo(vector<vector<int>>& eventIdx){
	for (size_t i = 0; i < eventIdx.size(); i++){
		cout << eventIdx[i].size() << "\t";
	}
	cout << "\n";
}
