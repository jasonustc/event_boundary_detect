#include "PreSegment.h"

int PreSegment::ComputeTOCscore(){
	int intervalNum = photos.size() - 1;
	tocScore.resize(intervalNum, 0);
	for (int i = 0; i < intervalNum; i++){
		int start = i - d > 0 ? i - d : 0;
		int end = i + d > intervalNum - 1 ? intervalNum - 1 : i + d;
		double score = 0;
		for (int j = start; j <= end; j++)
		{
			if (photos[j + 1].dTimeStamp - photos[j].dTimeStamp > 0)
				score = score + log(photos[j + 1].dTimeStamp - photos[j].dTimeStamp);

		}
		score = score / (end - start + 1);
		photos[i].TOCscore = log(photos[i + 1].dTimeStamp - photos[i].dTimeStamp) - score;
		tocScore[i] = photos[i].TOCscore;
	}
	return 0;
}

int PreSegment::CoarseCluster(int start, int end){
	int countDayNum = 1;
	for (size_t i = start; i < end; i++)
	{
		if (photos[i].SysTime.wDay != photos[i + 1].SysTime.wDay || 
			photos[i].SysTime.wMonth != photos[i].SysTime.wMonth ||
			photos[i].SysTime.wYear != photos[i].SysTime.wYear)
		{
			countDayNum++;
		}
	}

	if ((end - start < maxPhotoNum) && (countDayNum < maxDayNum))
		return 0;

	int pos = (int)(max_element(tocScore.begin() + start, tocScore.begin() + end) - tocScore.begin());
	boundIdx.push_back(pos);
	CoarseCluster(start, pos);
	CoarseCluster(pos + 1, end);
	return 0;
}

int PreSegment::SortPhotos(){
	std::sort(photos.begin(), photos.end(), timeComperer());
	return 0;
}

int PreSegment::GetBoundaries(){
	if (photos.size() == 0){
		return 0;
	}
	ComputeTOCscore();
	CoarseCluster(0, photos.size() - 1);
	if (boundIdx.size() == 0){
		vector<int> currEvent;
		for (size_t i = 0; i < photos.size(); i++){
			currEvent.push_back(i);
		}
		coEventIdx.push_back(currEvent);
		return 0;
	}
	//bound index must be in ascend order
	std::sort(boundIdx.begin(), boundIdx.end());
	for (size_t i = 0; i < boundIdx.size(); i++){
		int startPos = i == 0 ? 0 : boundIdx[i - 1];
		int endPos = boundIdx[i];
		vector<int> currEvent;
		for (size_t p = startPos; p < endPos; p++){
			currEvent.push_back(p);
		}
		coEventIdx.push_back(currEvent);
	}
	int startPos = boundIdx[boundIdx.size() - 1];
	vector<int> currEvent;
	for (size_t i = startPos; i < photos.size(); i++){
		currEvent.push_back(i);
	}
	coEventIdx.push_back(currEvent);
	return 0;
}