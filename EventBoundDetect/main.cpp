// cluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "io.h"
#include "PhotoProcess.h"
#include "Cluster.h"

#include "3rdparty/pugixml.hpp"
#include "FeatureHelper.h"

#include <direct.h>
#include <fstream>
#include "PreSegment.h"
#include "PostSegCheck.h"

using namespace std;

//TODO: test the code of time and distance computation
//TODO: make all parameters adaptive

void ParseInputFlags(int argc, TCHAR** argv, InputConfig& inputConfig);
void PrintEventInfo(vector<vector<int>>& eventIdx);
int _tmain(int argc, TCHAR** argv)
{
	//default folder is the images folder in the same direcoty of the executable
	if (argc < 2){
		cout << "Usage: PhotoClustering.exe\n"
			<< "-ff string [input photo feature file in xml format] \n";
		return -1;
	}

	//parse parameters
	clock_t t1, t2;
	InputConfig inConfig;
	ParseInputFlags(argc, argv, inConfig);
	string infoFile = inConfig.photoFeatFile + ".info.txt";
	ofstream ins(infoFile);

	//extract features
	vector<Photo_Feature_Set> photos;
	vector<SimpleEventInfo> simEventInfos;
	vector<vector<int>> eventIdx;
	t1 = clock();
	if (inConfig.photoFeatFile.size() > 0){
		LoadPhotoFromXml(inConfig.photoFeatFile, photos);
	}
	else{
		cout << "Usage: PhotoClustering.exe " << "-ff string [input photo feature file in xml format] \n";
		return 0;
	}
	//if no new photos need to do clustering
	if (photos.size() == 0){
		return 0;
	}
	t1 = clock();
	//preprocess the collection to control the time span and photo number
	PreSegment preseg(photos);
	preseg.GetBoundaries();
	vector<vector<int>> preSegEventIdx = preseg.coEventIdx;
	printf("With %d events after Pre Segment.\n", preSegEventIdx.size());
//	PrintEventInfo(preSegEventIdx);
	vector<vector<int>> fEventIdx;
	vector<Photo_Feature_Set> fPhotos;
	for (size_t i = 0; i < preSegEventIdx.size(); i++){
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
	t2 = clock();
	string clusterEventFile = inConfig.photoFeatFile + ".cluster.event.xml";
	wstring wClusterEventFile(clusterEventFile.begin(), clusterEventFile.end());
	SaveEvent2PhotosAsXml(fEventIdx, fPhotos, wClusterEventFile.c_str());
	ins << "Presegment time: " << ((double)t2 - (double)t1) / CLOCKS_PER_SEC << "\n";
	//merge is a global behavior
	CCluster gCluster(fPhotos);
	gCluster.MergeEvents2Scale(fEventIdx, inConfig.threshold, inConfig.timeK, inConfig.gpsK);
	gCluster.GetEventIndex(fEventIdx);
	string mergeEventFile = inConfig.photoFeatFile + ".merge.event.xml";
	wstring wMergeEventFile(mergeEventFile.begin(), mergeEventFile.end());
	SaveEvent2PhotosAsXml(fEventIdx, fPhotos, wMergeEventFile.c_str());
	printf("With %d events after merged\n", fEventIdx.size());
	t1 = clock();
	ins << "Merge time: " << ((double)t1 - (double)t2) / CLOCKS_PER_SEC << "\n";
//	PrintEventInfo(fEventIdx);

	//post check
	PostSegCheck postChecker(fPhotos, fEventIdx);
	postChecker.FinalCheck();
	photos = postChecker.GetPhotos();
	eventIdx = postChecker.GetEventIdx();
	printf("With %d events after final check\n", eventIdx.size());
	t2 = clock();
	ins << "Check time: " << ((double)t2 - (double)t1) / CLOCKS_PER_SEC << "\n";
	ins.close();
//	PrintEventInfo(eventIdx);

//	//save event information into xml file
	string finalEventFile = inConfig.photoFeatFile + ".final.event.xml";
	wstring wFinalEventFile(finalEventFile.begin(), finalEventFile.end());
	SaveEvent2PhotosAsXml(fEventIdx, fPhotos, wFinalEventFile.c_str());
//	int pathLen = wcslen(inConfig.tszEventSegFile);
//	if (wcscmp(inConfig.tszEventSegFile + pathLen - 3, L"xml") == 0){
//		SaveEvent2PhotosAsXml(eventIdx, photos, inConfig.tszEventSegFile);
//	}
//	else{
//		SaveEvent2PhotosAsText(eventIdx, photos, inConfig.tszEventSegFile);
//	}

	//Compute Performance
	EvaluateSegment evPerf(photos, eventIdx);
	evPerf.GetPerformance(infoFile);
	Performance perf = evPerf.meanPerf;
	ins.open(infoFile, ios::app);
	ins << "Precision: " << perf.precision << "\nRecall: " << perf.recall << 
		"\nF-score: " << perf.FScore << "\nAlbumItemCountSurplus: " << perf.AlbumCountSurplus;
	ins.close();
	cout << "Precision: " << perf.precision << "\tRecall: " << perf.recall << 
		"\tF-score: " << perf.FScore << "\tAlbumItemCountSurplus: " << perf.AlbumCountSurplus;
	return 0;
}

int TcharToString(TCHAR* m_tchar,string &m_string)
{
	char pFilePathName[MAX_PATH];
	int nLen = wcslen(m_tchar) + 1;
	WideCharToMultiByte(CP_ACP, 0,m_tchar, nLen, pFilePathName, 2 * nLen, NULL, NULL);
	m_string= pFilePathName;
	return 0;
}
void ParseInputFlags(int argc, TCHAR** argv, InputConfig& inputConfig){
	for (int i = 0; i < argc; i++){
		wstring arg = argv[i];
		string flag(arg.begin(), arg.end());
		if (flag == "-ug"){
			inputConfig.use_gps = _ttoi(argv[i + 1]);
		}
		else if (flag == "-ps"){
			inputConfig.tszPhotoSegFile = argv[i + 1];
		}
		else if (flag == "-es"){
			inputConfig.tszEventSegFile = argv[i + 1];
		}
		else if (flag == "-K"){
			inputConfig.K = _ttoi(argv[i + 1]);
		}
		else if (flag == "-th"){
			inputConfig.threshold = _ttof(argv[i + 1]);
		}
		else if (flag == "-of"){
			inputConfig.tszOutImageDir = argv[i + 1];
		}
		else if (flag == "-ff"){
			TcharToString(argv[i + 1], inputConfig.photoFeatFile);
		}
		else if (flag == "-tK"){
			inputConfig.timeK = _ttof(argv[i + 1]);
		}
		else if (flag == "-gK"){
			inputConfig.gpsK = _ttof(argv[i + 1]);
		}
		else if (flag == "-et"){
			TcharToString(argv[i + 1], inputConfig.eventFileTxt);
		}
		else if (flag == "-ef"){
			TcharToString(argv[i + 1], inputConfig.eventFeatXml);
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



