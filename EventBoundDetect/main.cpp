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

using namespace std;


void ParseInputFlags(int argc, TCHAR** argv, InputConfig& inputConfig);
int _tmain(int argc, TCHAR** argv)
{
	//default folder is the images folder in the same direcoty of the executable
	if (argc < 2){
		cout << "Usage: PhotoClustering.exe imgDir\n"
			"-ug 1 or 0 [if use gps for event segmentation]\n" <<
			"-ps photoSegmentFile [file to save result by photo] \n" <<
			"-es eventSegmentFile [file to save result by event] \n" <<
			"-gd float [global photo density of this user] \n" <<
			"-K  int [the granuaty to merge two sub events] \n" <<
			"-th float [the threshold to determine if two sub event are in same event] \n" <<
			"-of outputFolderName [folder to save result photos] \n" <<
			"-tK scale of time([5,20]) [parameter to control the time scale of events] \n" <<
			"-et string [event txt file name] \n" <<
			"-ff string [input photo feature file in xml format] \n" <<
			"-ef string [input event feature file in xml format] \n" <<
			"-gK scale of location([50, 100]) [parameter to control the gps scale of events] \n";
		return -1;
	}

	//parse parameters
	string infoFile("info.txt");
	ofstream ins(infoFile);
	clock_t t1, t2;
	InputConfig inConfig;
	inConfig.tszImageDir = argv[1];
	std::replace(inConfig.tszImageDir.begin(), inConfig.tszImageDir.end(), '/', '\\');
	if (inConfig.tszImageDir.back() != '\\'){
		inConfig.tszImageDir.append(L"\\");
	} 
	ParseInputFlags(argc, argv, inConfig);

	//extract features
	vector<Photo_Feature_Set> photos;
	vector<SimpleEventInfo> simEventInfos;
	vector<Photo_Feature_Set> oldPhotos;
	vector<vector<int>> oldEventIdx;
	t1 = clock();
	if (inConfig.photoFeatFile.size() > 0){
		LoadPhotoFromXml(inConfig.photoFeatFile, photos);
//		LoadPhotoFromTxtFolder(inConfig.photoFeatFile, photos);
	}
	else if (inConfig.eventFeatXml.size() > 0){
		LoadEventFromXml(inConfig.eventFeatXml, photos, simEventInfos);
	}
	else{
		HRESULT hr = S_OK;
		CPhotoProcess photoProcess;
		hr = photoProcess.ProcessPhotos(inConfig.tszImageDir.c_str());
		photoProcess.GetPhotoFeats(photos);
		photoProcess.GetOldEventIdx(oldEventIdx);
		photoProcess.GetOldPhotoFeats(oldPhotos);
	}
	t2 = clock();
	ins << "Feature extraction time: " << (t2 - t1) / CLOCKS_PER_SEC << "sec.\n";
	//if no new photos need to do clustering
	if (photos.size() == 0){
		return 0;
	}
	t1 = clock();
//	PreSegment preseg(photos);
//	preseg.GetBoundaries();
//	for (size_t i = 0; i < preseg.coEventIdx.size(); i++){
//		vector<Photo_Feature_Set> tmpPhotos;
//		for (size_t p = 0; p < preseg.coEventIdx[i].size(); p++){
//			tmpPhotos.push_back(photos[preseg.coEventIdx[i][p]]);
//		}
//		CCluster cluster(tmpPhotos);
//		cluster.Clustering(inConfig.use_gps);
//		//if all images in subevent is in a groundtruth event
//		//	cluster.PrintGroundTruth();
//		vector<vector<int>> eventIdx;
//		cluster.GetEventIndex(eventIdx);
//		//merge new clustered photos into oldPhotos and oldEventIdx
//		cluster.MergeTwoSegSet(oldPhotos, oldEventIdx, tmpPhotos, eventIdx);
//	}
//	//merge is a global behavior
//	CCluster gCluster(oldPhotos);
//	gCluster.MergeEvents2Scale(oldEventIdx, inConfig.threshold, inConfig.timeK, inConfig.gpsK);
//	gCluster.GetEventIndex(oldEventIdx);
//	cout << "With " << oldEventIdx.size() << " events in final segmentation.\n";

//	t2 = clock();
//	ins << "Segmentation time: " << double(t2 - t1) / CLOCKS_PER_SEC << "sec.\n";
//	ins << "number of photos: " << oldPhotos.size() << ".\n";
//	ins << "number of events: " << oldEventIdx.size() << ".\n";
//	CCluster cluster(photos);
//	cluster.Clustering(inConfig.use_gps);
	//if all images in subevent is in a groundtruth event
//	cluster.PrintGroundTruth();
//	vector<vector<int>> eventIdx;
//	cluster.GetEventIndex(eventIdx);
//	Performance perf = cluster.GetPerformance(eventIdx, inConfig.threshold, inConfig.timeK, inConfig.gpsK);
//	cout << "Pre: " << perf.precision << " rec: " << perf.recall << " Fscore: " << perf.FScore << "\n";
//	cout << "Merge sub events to final events...\n";
//	cluster.MergeEvents2Scale(eventIdx, inConfig.threshold, inConfig.timeK, inConfig.gpsK);
//	cluster.GetEventIndex(eventIdx);
//	cout << "With " << eventIdx.size() << " events in final segmentation.\n";
	//merge new and old into new
//	cluster.MergeTwoSegSet(photos, eventIdx, oldPhotos, oldEventIdx);
//	if (inConfig.eventFileTxt.size() > 0){
//		cluster.ExportEventInfo2Txt(inConfig.eventFileTxt);
//	}
//	string outImageFolder(inConfig.tszOutImageDir.begin(), inConfig.tszOutImageDir.end());
//	string subEventFolder = outImageFolder + "_subEvents";
//	SaveImagesToFolder(photos, eventIdx, subEventFolder);
//	cluster.GetPhotoFeatures(photos);
//	cluster.GetEventIndex(eventIdx);
//	cluster.ExportEventInfo("event_infos.xml");
//	cluster.LoadEventInfo("event_infos.xml");

	//save photo collection information to xml or txt
	int pathLen = wcslen(inConfig.tszPhotoSegFile);
	if (wcscmp(inConfig.tszPhotoSegFile + pathLen - 3, L"xml") == 0){
		SavePhoto2EventAsXml(oldPhotos, inConfig.tszPhotoSegFile);
	}
	else{
		SavePhoto2EventAsText(oldPhotos, inConfig.tszPhotoSegFile);
	}
	//save event information into xml file
//	pathLen = wcslen(inConfig.tszEventSegFile);
//	if (wcscmp(inConfig.tszEventSegFile + pathLen - 3, L"xml") == 0){
//		SaveEvent2PhotosAsXml(oldEventIdx, oldPhotos, inConfig.tszEventSegFile);
//	}
//	else{
//		SaveEvent2PhotosAsText(oldEventIdx, oldPhotos, inConfig.tszEventSegFile);
//	}
//	//organize photos in specific folder by event id
//	string outImageFolder(inConfig.tszOutImageDir.begin(), inConfig.tszOutImageDir.end());
//	if (inConfig.tszOutImageDir.size() > 0){
//		t1 = clock();
//		cout << "Organize photos into result folder...\n";
//		SaveImagesToFolder(oldPhotos, oldEventIdx, outImageFolder);
//		t2 = clock();
//		ins << "Copy photos time: " << double(t2 - t1) / CLOCKS_PER_SEC << "sec.\n";
//	}
//	//Compute Performance
//	EvaluateSegment evPerf(oldPhotos, oldEventIdx);
//	evPerf.GetPerformance();
//	Performance perf = evPerf.meanPerf;
//	ins << "Precision: " << perf.precision << "\nRecall: " << perf.recall << "\nF-score: " << perf.FScore;
//	ins.close();
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




