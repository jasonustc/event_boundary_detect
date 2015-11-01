// cluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "io.h"

#include "3rdparty/pugixml.hpp"
#include "FeatureHelper.h"

#include <direct.h>
#include <fstream>

using namespace std;


void ParseInputFlags(int argc, TCHAR** argv, InputConfig& inputConfig);
int TcharToString(TCHAR* m_tchar, string &m_string);
int _tmain(int argc, TCHAR** argv)
{
	//default folder is the images folder in the same direcoty of the executable
	if (argc < 2){
		cout << "Usage: PhotoClustering.exe photoFeatFile\n";
		return -1;
	}

	//parse parameters
	ofstream out_info("info.txt");
	clock_t t1, t2;
	InputConfig inConfig;
	TcharToString(argv[1], inConfig.photoFeatFile);
	std::replace(inConfig.photoFeatFile.begin(), inConfig.photoFeatFile.end(), '/', '\\');

	//Load photo features and split into different users
	vector<Photo_Feature_Set> photos;
	t1 = clock();
	if (inConfig.photoFeatFile.size() > 0){
		LoadPhotoFromXml(inConfig.photoFeatFile, photos);
	}
	vector<vector<Photo_Feature_Set>> splitUsers;
	vector<string> userNames;
	SplitPhotoToDifferentUsers(photos, splitUsers, userNames);
	t1 = clock();

	//save photo collection information to xml or txt
	int pathLen = wcslen(inConfig.tszPhotoSegFile);
	for (size_t i = 0; i < userNames.size(); i++){
		string userXmlFile = userNames[i] + ".xml";
		wstring tUserName(userXmlFile.begin(), userXmlFile.end());
		out_info << "username: " << userNames[i] << " numPhotos: " << splitUsers[i].size() << "\n";
		int n = CountNumEvents(splitUsers[i], out_info);
		out_info << " \nnumEvents: " << n << "\n";
		SavePhoto2EventAsXml(splitUsers[i], tUserName.c_str());
	}
	out_info.close();
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




