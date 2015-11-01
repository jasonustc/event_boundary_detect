#include "PhotoInfo.h"

using std::vector;
using std::string;

class PostSegCheck{

public:

	PostSegCheck(vector<Photo_Feature_Set>& photos, vector<vector<int>>& eventIdx){
		if (photos.size() == 0 || eventIdx.size()){
			printf("empty photo data or event index data\n");
			return;
		}
		this->eventIdx_ = eventIdx;
		this->photos_ = photos;
		eventInd_.resize(eventIdx.size(), -1);
	}

	void FinalCheck(){
		EventNumCheck();
		TimeLocationCheck();
		GetFinalCheckResult();
	}

	vector<Photo_Feature_Set>& GetPhotos(){ return this->photos_; }
	vector<vector<int>>& GetEventIdx(){ return this->eventIdx_; }

private:

	int EventNumCheck();
	int TimeLocationCheck();

	void GenFinalEvents();

	void GetFinalCheckResult();

	vector<Photo_Feature_Set> photos_;
	vector<vector<int>> eventIdx_;

	//to indicate if this is a final event
	//-2: not an event, -1: is an event >=0: should merge event with this id
	vector<int> eventInd_; 

	int minPhotoNumInEvent_ = 14; // default as at least 14 photos
	int maxEventTimeSpan_ = 50; //max time span when merge two sub events
	int maxEventDistSpan_ = 125 * 1000;// max distance of photos in event

	float TimeSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2);
	float LocSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2);
};
