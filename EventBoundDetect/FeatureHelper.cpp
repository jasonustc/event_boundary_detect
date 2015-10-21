#include "FeatureHelper.h"
#include "3rdparty\pugixml.hpp"
#include <omp.h>

using std::string;
using std::vector;
using std::ostringstream;
using namespace std;

bool isVecEmpty(vector<float>& vec){
	if (vec.size() == 0){
		return true;
	}
	for (size_t i = 0; i < vec.size(); i++){
		if (vec[i] != 0){
			return false;
		}
	}
	return true;
}

//dot correlation
float GetSimOfTwoVec(vector<float>& feat1, vector<float>& feat2){
	if (feat1.size() != feat2.size() || feat1.size() == 0){
		return 0;
	}
	float coSum(0), sum1(0), sum2(0);
	for (size_t i = 0; i < feat1.size(); i++){
		coSum += feat1[i] * feat2[i];
		sum1 += feat1[i] * feat1[i];
		sum2 += feat2[i] * feat2[i];
	}
	if (sum1 == 0 || sum2 == 0){
		return 0;
	}
	return coSum / (sqrt(sum1) * sqrt(sum2));
}

//dot correlation
float GetSimOfTwoVec2(vector<float>& feat1, vector<float>& feat2){
	if (feat1.size() != feat2.size() || feat1.size() == 0){
		return 0;
	}
	float coSum(0), sum1(0), sum2(0);
	for (int i = 0; i < feat1.size(); i++){
		coSum += (feat1[i] - feat2[i]) * (feat1[i] - feat2[i]);
		sum1 += feat1[i] * feat1[i];
		sum2 += feat2[i] * feat2[i];
	}
	if (sum1 == 0 || sum2 == 0){
		return 0;
	}
	return exp(- coSum / (sqrt(sum1) * sqrt(sum2)));
}

//dot correlation
float GetSimOfTwoVecNorm(vector<float>& feat1, vector<float>& feat2,
	vector<float>& avg, vector<float>& stdv){
	if (feat1.size() != feat2.size() || feat1.size() == 0){
		return 0;
	}
	if (avg.size() != feat1.size() || stdv.size() != feat1.size()){
		float coSum(0), sum1(0), sum2(0);
		for (size_t i = 0; i < feat1.size(); i++){
			coSum += (feat1[i] - feat2[i]) * (feat1[i] - feat2[i]);
			sum1 += feat1[i] * feat1[i];
			sum2 += feat2[i] * feat2[i];
		}
		if (sum1 == 0 || sum2 == 0){
			return 0;
		}
		return exp(-coSum / (sqrt(sum1) * sqrt(sum2)));
	}
	else{
		float coSum(0), var(0);
		for (size_t i = 0; i < feat1.size(); i++){
			coSum += (feat1[i] - feat2[i]) * (feat1[i] - feat2[i]);
			var += stdv[i] * stdv[i];
		}
		if (var == 0){
			return 0;
		}
		return exp(-coSum / var);
	}
}

void SaveImagesToFolder(const vector<Photo_Feature_Set>& photos, 
	vector<vector<int>>& images, const string& saveFolder){
	if (_access(saveFolder.c_str(), 0) == -1){
		_mkdir(saveFolder.c_str());
	}
	ostringstream os;
#pragma omp parallel for 
	{
		int i = 0;
		int j = 0;
		for (i = 0; i < images.size(); i++){
			os.str("");
			vector<int> eventImgs = images[i];
			os << i;
			string subFolderPath;
			cout << os.str() << "\t";
			MakeSubFolder(saveFolder, os.str(), subFolderPath);
			for (j = 0; j < images[i].size(); j++){
				int p = eventImgs[j];
//				cout << "Thread num == " << omp_get_thread_num() << endl;
				CopyFileToFolder(photos[p].tszFileName, subFolderPath);
			}
		}
	}
}

vector<string>& split(const string& s, char delim, vector<string>& elems){
	if (elems.size() > 0){
		elems.clear();
	}
	string tempS = s;
	std::replace(tempS.begin(),tempS.end(), ' ', delim);
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

SYSTEMTIME String2SysTime(const string& str){
	SYSTEMTIME sysTime = {0};
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

int LoadPhotoFromXml(string& xmlFile, vector<Photo_Feature_Set>& photos){
	if (photos.size() > 0){
		photos.clear();
	}
	if (xmlFile.size() == 0){
		return -1;
	}
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xmlFile.c_str());
	if (!result){
		printf("check the feature file %s\n", xmlFile.c_str());
		return -1;
	}
	pugi::xml_node featNode = doc.child("Photo2Event");
	for (pugi::xml_node photo = featNode.child("Photo"); photo; photo = photo.next_sibling("Photo")){
		Photo_Feature_Set photoFeat;
		photoFeat.photoIndex = atoi(photo.child_value("id"));
		string photoPath = photo.child_value("path");
		sprintf(photoFeat.tszFileName, "%s",photoPath.c_str());
		photoFeat.iEventLabel = atoi(photo.child_value("event"));
		string timeStamp = photo.child_value("timestamp");
		photoFeat.SysTime = String2SysTime(timeStamp);
		photoFeat.dTimeStamp = stod(photo.child_value("dTimeStamp"));
		photoFeat.longitude = atof(photo.child_value("longitude"));
		photoFeat.latitude = atof(photo.child_value("latitude"));
		photoFeat.atitude = atof(photo.child_value("attitude"));
		photoFeat.pflQuality = atof(photo.child_value("quality"));
		photoFeat.brightValue = atof(photo.child_value("brightValue"));
		photos.push_back(photoFeat);
	}
	printf("Load feature of %d photos in file %s\n", photos.size(), xmlFile.c_str());
	return 0;
}

int LoadEventFromXml(string& xmlFile, vector<Photo_Feature_Set>& photos, vector<SimpleEventInfo>& simEventInfos){
	if (photos.size() > 0){
		photos.clear();
	}
	if (simEventInfos.size() > 0){
		simEventInfos.clear();
	}
	if (xmlFile.size() == 0){
		return 1;
	}
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(xmlFile.c_str());
	if (!result){
		printf("check the feature file %s\n", xmlFile.c_str());
		return 2;
	}
	pugi::xml_node featNode = doc.child("Event2Photos");
	if (!featNode){
		return 3;
	}
	int numPhotos = 0;
	for (pugi::xml_node eventNode = featNode.child("Event"); eventNode; eventNode = eventNode.next_sibling("Event")){
		SimpleEventInfo eInfo;
		pugi::xml_node startPhoto = eventNode.child("Photo");
		if (startPhoto){
			eInfo.startTime = stod(startPhoto.child_value("dTimeStamp"));
		}
		for (pugi::xml_node photo = eventNode.child("Photo"); photo; photo = photo.next_sibling("Photo")){
			Photo_Feature_Set photoFeat;
			photoFeat.photoIndex = atoi(photo.child_value("id"));
			string photoPath = photo.child_value("path");
			sprintf(photoFeat.tszFileName, "%s", photoPath.c_str());
			photoFeat.iEventLabel = atoi(photo.child_value("event"));
			string timeStamp = photo.child_value("timestamp");
			photoFeat.SysTime = String2SysTime(timeStamp);
			photoFeat.dTimeStamp = stod(photo.child_value("dTimeStamp"));
			photoFeat.longitude = atof(photo.child_value("longitude"));
			photoFeat.latitude = atof(photo.child_value("latitude"));
			photoFeat.atitude = atof(photo.child_value("attitude"));
			photoFeat.pflQuality = atof(photo.child_value("quality"));
			photoFeat.brightValue = atof(photo.child_value("brightValue"));
			photos.push_back(photoFeat);
			eInfo.photoIdx.push_back(numPhotos);
			numPhotos++;
		}
		eInfo.endTime = photos[photos.size() - 1].dTimeStamp;
		simEventInfos.push_back(eInfo);
	}
	printf("Load  %d photos, %d events from file %s\n", photos.size(), simEventInfos.size(), xmlFile.c_str());
	return 0;
}

void replaceStr(string& str,const string& from, const string& to){
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos){
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

int LoadPhotoFromTxtFolder(string& folderPath, vector<Photo_Feature_Set>& photos){
	if (photos.size() > 0){
		photos.clear();
	}
	if (folderPath.size() == 0){
		return -1;
	}
	vector<wstring> txtFiles;
	wstring wFolderPath(folderPath.begin(), folderPath.end());
	GetTxtFilesInDir(wFolderPath.c_str(), txtFiles);
	for (size_t i = 0; i < txtFiles.size(); i++){
		string path(txtFiles[i].begin(), txtFiles[i].end());
		Photo_Feature_Set photo;
		photo.LoadFeatFromTxt(path);
		string photoPath(photo.tszFileName);
		replaceStr(photoPath, "\\\\", "\\");
		size_t n = std::count(photoPath.begin(),photoPath.end(),'\\');
		//only consider photos that is already segmented by user
		if (n > 2){
			photos.push_back(photo);
			cout << photoPath << "\n";
		}
	}
	return 0;
}


string SysTime2String(const SYSTEMTIME & sysTime){
	ostringstream oss;
	oss << sysTime.wYear
		<< ':' << sysTime.wMonth
		<< ':' << sysTime.wDay
		<< ' ' << sysTime.wHour
		<< ':' << sysTime.wMinute
		<< ':' << sysTime.wSecond;
	return oss.str();
}

long SysTime2Long(const SYSTEMTIME& SysTime)
{
	long dSecond = 0;

	int iYearOffset = SysTime.wYear - YEAR_OFFSET;
	int iYear = SysTime.wYear;
	int iMonth = SysTime.wMonth;
	int iDay = SysTime.wDay;
	int iHour = SysTime.wHour;
	int iMin = SysTime.wMinute;
	int iSec = SysTime.wSecond;

	bool leapYear = ((iYear % 4 == 0 && iYear % 100 != 0) || iYear % 400 == 0);
	int iDayPerYear = leapYear ? 366 : 365;

	int iMonDays = 0;
	for (int i = 1; i < iMonth; i++)
	{
		int iDayPerMon = 31;
		switch (i)
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			iDayPerMon = 31;
			break;

		case 2:
			if (leapYear == true)
			{
				iDayPerMon = 29;
			}
			else
			{
				iDayPerMon = 28;
			}
			break;

		default:
			iDayPerMon = 30;
		}
		iMonDays += iDayPerMon;
	}



	dSecond =(iYearOffset * iDayPerYear * 24 * 60 + iMonDays * 24 * 60 + iDay * 24 * 60 + iHour * 60 + iMin) * 60 + iSec;
	return dSecond;
}

bool SavePhoto2EventAsXml(const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath) {
	pugi::xml_document doc;
	pugi::xml_node root_node = doc.append_child("Photo2Event");
	root_node.append_attribute("descr").set_value("Photo Information");

	for (size_t i = 0; i < photos.size(); ++i){
		pugi::xml_node photo_node = root_node.append_child("Photo");
		photo_node.append_child("id").text().set(photos[i].photoIndex);
		photo_node.append_child("path").text().set(photos[i].tszFileName);
		photo_node.append_child("event").text().set(photos[i].iEventLabel);
		photo_node.append_child("timestamp").text().set(SysTime2String(photos[i].SysTime).c_str());
		photo_node.append_child("dTimeStamp").text().set(SysTime2Long(photos[i].SysTime));
		photo_node.append_child("longitude").text().set(photos[i].longitude);
		photo_node.append_child("latitude").text().set(photos[i].latitude);
		photo_node.append_child("attitude").text().set(photos[i].atitude);
		photo_node.append_child("representativeness").text().set(photos[i].fRep);
		photo_node.append_child("quality").text().set(photos[i].pflQuality);
		photo_node.append_child("brightValue").text().set(photos[i].brightValue);
	}

	if (!doc.save_file(outFilePath)){
		cerr << "Open or create file " << outFilePath << " failed!" << endl;
		return false;
	}
	return true;
}

bool SavePhoto2EventAsText(const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath) {
	ofstream outFile(outFilePath, ios::out);
	if (!outFile){
		cerr << "Open or create file " << outFilePath << " failed!" << endl;
		return false;
	}
	else{

		outFile << "#Photo Information\n#\n#\n";
		for (size_t i = 0; i < photos.size(); ++i){
			outFile << photos[i].photoIndex << '\t' << photos[i].tszFileName << '\t' << photos[i].iEventLabel
				<< '\t' << SysTime2Long(photos[i].SysTime) << "(" << SysTime2String(photos[i].SysTime) << ")"
				<< '\t' << photos[i].longitude << '\t' << photos[i].latitude << '\t' << photos[i].atitude
				<< '\t' << photos[i].fRep << '\t' << photos[i].pflQuality
				<< '\n';
		}
		outFile.close();
	}

	return true;

}

bool SaveEvent2PhotosAsXml(const vector<vector<int>> eventIdx, const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath) {
	pugi::xml_document doc;
	pugi::xml_node root_node = doc.append_child("Event2Photos");
	root_node.append_attribute("descr").set_value("Event Information");

	for (size_t i = 0; i < eventIdx.size(); ++i){
		pugi::xml_node event_node = root_node.append_child("Event");
		event_node.append_attribute("id").set_value(i);
		for (auto photo_iter = eventIdx[i].begin(); photo_iter != eventIdx[i].end(); ++photo_iter){
			pugi::xml_node photo_node = event_node.append_child("Photo");
			int photo_id = *photo_iter;
			photo_node.append_child("id").text().set(photos[photo_id].photoIndex);
			photo_node.append_child("path").text().set(photos[photo_id].tszFileName);
			photo_node.append_child("event").text().set(photos[photo_id].iEventLabel);
			photo_node.append_child("timestamp").text().set(SysTime2String(photos[photo_id].SysTime).c_str());
			photo_node.append_child("dTimeStamp").text().set(SysTime2Long(photos[photo_id].SysTime));
			photo_node.append_child("longitude").text().set(photos[photo_id].longitude);
			photo_node.append_child("latitude").text().set(photos[photo_id].latitude);
			photo_node.append_child("attitude").text().set(photos[photo_id].atitude);
			photo_node.append_child("representativeness").text().set(photos[photo_id].fRep);
			photo_node.append_child("quality").text().set(photos[photo_id].pflQuality);
		}
	}

	if (!doc.save_file(outFilePath)){
		cerr << "Open or create file " << outFilePath << " failed!" << endl;
		return false;
	}
	return true;
}

bool SaveEvent2PhotosAsText(const vector<vector<int>> eventIdx, const vector<Photo_Feature_Set> &photos, const TCHAR* outFilePath) {
	ofstream outFile(outFilePath, ios::out);
	if (!outFile){
		cerr << "Open or create file " << outFilePath << " failed!" << endl;
		return false;
	}
	else{
		outFile << "#Event Information\n#\n#\n";
		int eventId = 0;
		for (auto iter = eventIdx.begin(); iter != eventIdx.end(); ++iter){
			outFile << eventId++ << ":\t";
			for (auto subIter = iter->begin(); subIter != iter->end(); ++subIter){
				outFile << *subIter << '\t';
			}
			outFile << '\n';
		}
		outFile.close();
	}
	return true;
}