#include "PostSegCheck.h"
#include "xu_lib.h"

int PostSegCheck::EventNumCheck(){
	for (size_t i = 0; i < eventIdx_.size(); i++){
		if (eventIdx_[i].size() < minPhotoNumInEvent_){
			//if the number of photos in event less than given number
			//we don't consider this group as an event
			eventInd_[i] = -2;
		}
	}
	return 0;
}

double PostSegCheck::LocSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2){
	double lat1 = photo1.latitude;
	double lon1 = photo1.longitude;
	double alt1 = photo1.atitude;
	double lat2 = photo2.latitude;
	double lon2 = photo2.longitude;
	double alt2 = photo2.atitude;
	if (lat1 == 0 && lon1 == 0 && alt1 == 0){
		return -1;
	}
	if (lat2 == 0 && lon2 == 0 && alt2 == 0){
		return -1;
	}
	double  dlong = (lon2 - lon1) * D2R;
	double  dlat = (lat2 - lat1) * D2R;
	double  dalt = (alt2 - alt1);
	double  a1 = sin(dlat / 2.0);
	double  a2 = sin(dlong / 2.0);
	double  a = a1 * a1 + cos(lat1 * D2R) * cos(lat2 * D2R) * a2 * a2;
	double  c = 2 * atan2(sqrt(a), sqrt(1 - a));
	double  d = 6367 * c * 1000;
	double  ad = sqrt(d * d + dalt * dalt);
	//in meters
	return ad;
}

double PostSegCheck::TimeSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2){
	//in minutes
	double time1 = GetSecondTime(photo1.SysTime);
	double time2 = GetSecondTime(photo2.SysTime);
	return time2 - time1;
}

void PostSegCheck::InputGapInfo(){
	ifstream in_gap("gap_info.txt");
	if (!in_gap.is_open()){
		return;
	}
	float timeGap, distGap;
	if (in_gap >> timeGap){
		this->minEventTimeGap_ = timeGap;
	}
	if (in_gap >> distGap){
		this->minEventDistGap_ = distGap;
	}
}

int PostSegCheck::TimeLocationCheck(){
	InputGapInfo();
	for (size_t i = 1; i < eventIdx_.size(); i++){
		int mid_1 = eventIdx_[i - 1].size() / 2;
		int mid_2 = eventIdx_[i].size() / 2;
		Photo_Feature_Set photo1 = photos_[eventIdx_[i - 1][mid_1]];
		Photo_Feature_Set photo2 = photos_[eventIdx_[i][mid_2]];
		double dist = LocSim(photo1, photo2);
		double timeGap = TimeSim(photo1, photo2);
		if (dist > 0){
			if (dist < minEventDistGap_ &&  timeGap < minEventTimeGap_){
				cout << "merge by distance and time: " << dist << " " << time << "\n";
				eventInd_[i] = i - 1;
			}
		}
		else if (timeGap < minEventTimeGap_){
			cout << "merge by time: " << timeGap;
			eventInd_[i] = i - 1;
		}
	}
	return 0;
}

void PostSegCheck::GetFinalCheckResult(){
	vector<vector<int>> new_eventIdx;
	//merge event to corresponding event
	for (size_t i = 0; i < eventIdx_.size(); i++){
		int ind = eventInd_[i];
		if (ind >= 0){
			eventIdx_[ind].insert(eventIdx_[ind].end(), 
				eventIdx_[i].begin(), eventIdx_[i].end());
		}
	}
	for (size_t i = 0; i < eventIdx_.size(); i++){
		int ind = eventInd_[i];
		if (ind == -1){
			new_eventIdx.push_back(eventIdx_[i]);
		}
	}
	//update
	eventIdx_ = new_eventIdx;
}
