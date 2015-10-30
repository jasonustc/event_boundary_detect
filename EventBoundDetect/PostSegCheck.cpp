#include "PostSegCheck.h"

int PostSegCheck::EventNumCheck(){
	for (size_t i = 0; i < eventIdx_.size(); i++){
		if (eventIdx_[i].size() < minPhotoNumInEvent_){
			//if the number of photos in event less than given number
			//we don't consider this group as an event
			eventInd_[i] = -1;
		}
	}
}

float PostSegCheck::LocSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2){
	return 0;
}

float PostSegCheck::TimeSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2){
	return 0;
}

int PostSegCheck::TimeLocationCheck(){
	for (size_t i = 1; i < eventIdx_.size(); i++){
		int mid_1 = eventIdx_[i - 1].size() / 2;
		int mid_2 = eventIdx_[i].size() / 2;
		Photo_Feature_Set photo1 = photos_[eventIdx_[i - 1][mid_1]];
		Photo_Feature_Set photo2 = photos_[eventIdx_[i][mid_2]];
		if (LocSim(photo1, photo2) < maxEventDistSpan_){
			eventInd_[i] = i - 1;
		}
		if (LocSim(photo1, photo2) < maxEventTimeSpan_){
			eventInd_[i] = i - 1;
		}
	}
}
