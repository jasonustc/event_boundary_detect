#include "stdafx.h"
#include "PhotoProcess.h"
#include "Cluster.h"
#include "FeatureHelper.h"

int GetDayInfoInCollection(vector<Photo_Feature_Set>& photos){
	int days = 0;
	vector<bool> is_assigned(photos.size(), false);
	for (size_t i = 0; i < photos.size(); i++){
		if (is_assigned[i] == false){
			is_assigned[i] = true;
			WORD day_time_i = photos[i].SysTime.tm_mday;
			WORD year_time_i = photos[i].SysTime.tm_year;
			WORD month_time_i = photos[i].SysTime.tm_mon;
			for (size_t j = i; j < photos.size(); j++){
				WORD day_time_j = photos[j].SysTime.tm_mday;
				WORD month_time_j = photos[j].SysTime.tm_mon;
				WORD year_time_j = photos[j].SysTime.tm_year;
				if (day_time_i == day_time_j && month_time_i == month_time_j && year_time_i == year_time_j){
					is_assigned[j] = true;
					photos[j].day_id = days;
				}
			}
			days++;
		}
	}
	printf("Photos consist of %d days.\n", days);
	return days;
}

int copyfile(string initialFilePath, string outputFilePath);
bool AssignPhotos2Event(vector<Photo_Feature_Set>& photos, int days, InputConfig& inputCfg){
	vector<double> local_average_time(days, 0);
	for (size_t i = 0; i < days; i++){
		int num_local_photos = 0;
		for (size_t j = 0; j < photos.size(); j++){
			if (photos[j].day_id == i){
				num_local_photos++;
				local_average_time[i] += photos[j].dTimeStamp;
			}
		}
		local_average_time[i] /= num_local_photos;
	}
	vector<vector<double>> similarity(days, vector<double>(days, 0));

	for (size_t i = 0; i < days; i++){
		for (size_t j = i; j < days; j++){
			similarity[i][j] = exp(-abs(local_average_time[i] - local_average_time[j]) / (inputCfg.K * 1440));
		}
	}

	vector<int> day2event(days, -1);
	day2event[0] = 0;
	for (size_t i = 1; i < days; i++){
		if (similarity[i - 1][i] > inputCfg.threshold){
			day2event[i] = day2event[i - 1];
		}
		else{
			day2event[i] = day2event[i - 1] + 1;
		}
	}

	int numEvents = day2event[days - 1] + 1;
	for (size_t i = 0; i < days; i++){
		for (size_t j = 0; j < photos.size(); j++){
			if (photos[j].day_id == i){
				photos[j].iEventLabel = day2event[i];
			}
		}
	}

	//copy photos to folder by event
	string outfolder= inputCfg.tszOutImageDir;
  MakeDir(outfolder);
	for (size_t i = 0; i < numEvents; i++) {
		stringstream ss;
		string s;
		ss << i;
		ss >> s;
    string newimgfolder;
    MakeSubFolder(outfolder, s, newimgfolder);
		for (size_t j = 0; j <photos.size(); j++) {
			if (photos[j].iEventLabel == i) {
        CopyFileToFolder(photos[j].tszFileName, newimgfolder);
			}
		}
	}
	return true;
}
