#include "PhotoInfo.h"
#include "Cluster.h"

class PreSegment{

public:
	PreSegment(vector<Photo_Feature_Set>& photos){ 
		this->photos = photos; 
		SortPhotos();
	}

public:
	int GetBoundaries();

private:
	int ComputeTOCscore();
	int CoarseCluster(int start, int end);
	int SortPhotos();

private:
	vector<float> tocScore;
	vector<Photo_Feature_Set> photos;
	int d = 50;
	int maxPhotoNum = 2000;//threshold of photo num, less than
	//threshold of day num, if the time span of the dataset is more than
	//30 days, we split it into two sub groups and cluter them independently
	int maxDayNum = 30;
	vector<int> boundIdx;

public:
	vector<vector<int>> coEventIdx;
};