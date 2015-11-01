// cluster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "io.h"
#include "PhotoProcess.h"

#include "3rdparty/pugixml.hpp"
#include "FeatureHelper.h"

#include <direct.h>
#include <fstream>

using namespace std;


void ParseInputFlags(int argc, TCHAR** argv, InputConfig& inputConfig);
int _tmain(int argc, TCHAR** argv)
{
	//default folder is the images folder in the same direcoty of the executable
	if (argc < 2){
		cout << "Usage: PhotoClustering.exe imgDir\n"
			<< "       [-ps save_file_name]\n";
		return -1;
	}

	//parse parameters
	InputConfig inConfig;
	inConfig.tszImageDir = argv[1];
	std::replace(inConfig.tszImageDir.begin(), inConfig.tszImageDir.end(), '/', '\\');
	if (inConfig.tszImageDir.back() != '\\'){
		inConfig.tszImageDir.append(L"\\");
	} 
	ParseInputFlags(argc, argv, inConfig);

	//extract features
	vector<Photo_Feature_Set> photos;
	HRESULT hr = S_OK;
	CPhotoProcess photoProcess;
//	LoadPhotoFromXml(inConfig.photoFeatFile, photos);
	hr = photoProcess.ProcessPhotos(inConfig.tszImageDir.c_str(), inConfig.tszPhotoSegFile);
	photoProcess.GetPhotoFeats(photos);

	//save photo collection information to xml or txt
	int pathLen = wcslen(inConfig.tszPhotoSegFile);
	if (wcscmp(inConfig.tszPhotoSegFile + pathLen - 3, L"xml") == 0){
		SavePhoto2EventAsXml(photos, inConfig.tszPhotoSegFile);
	}
	else{
		SavePhoto2EventAsText(photos, inConfig.tszPhotoSegFile);
	}
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




