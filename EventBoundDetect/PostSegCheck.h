#include "PhotoInfo.h"

using std::vector;
using std::string;

#define D2R (3.1415926 / 180)

class PostSegCheck{

public:

	PostSegCheck(vector<Photo_Feature_Set>& photos, vector<vector<int>>& eventIdx){
		if (photos.size() == 0 || eventIdx.size() == 0){
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
	//50 minutes
	//min time gap between two events
	int minEventTimeGap_ = 50; 
	//min distance gap between two events
	//5000 meters
	int minEventDistGap_ = 5 * 1000;

	double TimeSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2);
	double LocSim(Photo_Feature_Set& photo1, Photo_Feature_Set& photo2);
	void InputGapInfo();
};
