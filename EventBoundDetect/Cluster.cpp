#include "stdafx.h"
#include "Cluster.h"
#include <math.h>
#include <algorithm>

#include <sstream>
using namespace std;

CCluster::CCluster(vector<Photo_Feature_Set> &m_vecPhotos)
{
    for(unsigned int i=0;i<m_vecPhotos.size();i++){
        double dTime = this->GetSecondTime(m_vecPhotos[i].SysTime);
        m_vecPhotos[i].dTimeStamp = dTime;
	}
	this->m_vecPhoto = m_vecPhotos;
	m_flPrevCost_MDL = -DBL_MAX;
	m_flCurrCost_MDL = -DBL_MAX;
	m_iBestK_MDL = 0;
	m_flCurrML = 0;
}

CCluster::~CCluster() { }

void CCluster::GetEventIndex(vector<vector<int>> &eventPhotoIndex)
{
//	this->BuildIndex();
    eventPhotoIndex = this->indexOfEventPhotos;
}

void CCluster::GetPhotoFeatures(vector<Photo_Feature_Set> &photos)
{
    photos = this->m_vecPhoto;
}

//transform system time into absolute time
double CCluster::GetSecondTime(IN SYSTEMTIME SysTime)
{
	double dSecond = 0;

	int iYearOffset = SysTime.wYear - YEAR_OFFSET;
	int iYear = SysTime.wYear;
	int iMonth = SysTime.wMonth;
	int iDay = SysTime.wDay;
	int iHour = SysTime.wHour;
	int iMin = SysTime.wMinute;
	int iSec = SysTime.wSecond;

	bool leapYear = ((iYear % 4 == 0 && iYear % 100 != 0) || iYear % 400 == 0 );
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

	//convert to minutes
	dSecond = double(iYearOffset * iDayPerYear * 24 * 60 + iMonDays * 24 * 60 + iDay * 24 * 60 + iHour * 60 + iMin) + iSec / 60.0;
	return dSecond;
}

//Set the initial number of event
HRESULT CCluster::Preprocess()
{
	HRESULT hr = S_OK;
	int i = 0, j = 0;
	double dStartTime = 0, dEndTime = 0, dInterval = 0;

	m_iInitK = 1;
	m_iK = 0;
	m_iN = m_vecPhoto.size();//initialized as the # of photos

	if (m_iN <= 0)
	{
		return E_FAIL;
	}
	this->SortPhotos();
	dStartTime = m_vecPhoto[0].dTimeStamp;
	double dTime0 = m_vecPhoto[0].dTimeStamp;
	for (unsigned int i = 0; i<m_vecPhoto.size(); i++){
		m_vecPhoto[i].dTimeStamp = (m_vecPhoto[i].dTimeStamp - dTime0) / TIME_INTERVAL;
	}
	m_vecPhoto[0].fRep = FALSE;
	for (i = 1; i < m_iN; i++)
	{
		m_vecPhoto[i].fRep = FALSE;//mark of best photo
		// calculate the number of scene
		dEndTime = m_vecPhoto[i].dTimeStamp;
		dInterval = (dEndTime - dStartTime);
		//we segment the initial events by every 12 hours.
		if (int(fabs(dInterval)) >= PHOTO_INTERVAL / TIME_INTERVAL)
		{
			m_iInitK++;//initialized as one day an event.
		}
		dStartTime = dEndTime;
	}
	m_pBestLabel_MDL = new int[m_iN];

	if (m_pBestLabel_MDL == NULL)
	{
		hr = E_OUTOFMEMORY;
	}
	//max events a day is set to 4
	iTotalEvent = m_iInitK * MAX_EVENTS_PERDAY;
	if (iTotalEvent > m_iN / 4)//can not exceed the 1/4+2 of the photos
	{
		iTotalEvent = m_iN / 4 + 2;
	}
	if (m_iInitK >= iTotalEvent)
	{
		m_iInitK = iTotalEvent - 1;//can not exceed the max NO. of events
	}

	return hr;
}

//get K_min and K_min based on the photo density of current day and global photo density
HRESULT CCluster::PreprocessNew()
{
	HRESULT hr = S_OK;
	int i = 0, j = 0;

	if (m_iN <= 0)
	{
		return E_FAIL;
	}
	//sort in time order
    this->SortPhotos();
	m_iInitK = GetDayInfoInCollection(this->m_vecPhoto);
	iTotalEvent = m_iInitK * MAX_EVENTS_PERDAY;
	if (iTotalEvent > m_iN / 4)//can not exceed the 1/4+2 of the photos
	{
		iTotalEvent = m_iN / 4 + 2;
	}
	if (m_iInitK >= iTotalEvent)
	{
		m_iInitK = iTotalEvent - 1;//can not exceed the max NO. of events
	}
	return hr;
}

/*********************************************************************************************\
SortPhotos

[IN]  m_vecPhotoLabel - unsorted
[OUT] m_vecPhotoLabel - sorted by time, in accending order
\*********************************************************************************************/
void CCluster::SortPhotos()
{
	//Sort by time
	std::sort(m_vecPhoto.begin(), m_vecPhoto.end(), timeComperer());
}

HRESULT CCluster::Clustering(bool use_gps)
{
	HRESULT hr = S_OK;
	Mat samples;
	// 0.1 Preprocess data.
	hr = Preprocess();
	if (FAILED(hr)){
		return hr;
	}
	// 0.2 load features.
	LoadFeatures(samples, use_gps);

	// 1. EM training
	EMtraining(samples, use_gps);

	// 2. set the label
	for (unsigned int i = 0; i < m_vecPhoto.size(); i++){
		m_vecPhoto[i].iEventLabel = m_pBestLabel_MDL[i];
	}

	// 5. merge too close sub event
	if(SUCCEEDED(hr)){
        this->MergeEvent();
	}
    // 6. build the index of events and best photos
    this->BuildIndex();
	return hr;
}

//load time features of photos
HRESULT CCluster::LoadFeatures(Mat &samples, bool use_gps){
	if (use_gps){
		samples = Mat::zeros(cv::Size(TIME_FEATURE_DIM + GPS_FEATURE_DIM, m_vecPhoto.size()), CV_32FC1);
		for (unsigned int i = 0; i < m_vecPhoto.size(); i++){
			float* ptrR = samples.ptr<float>(i);
			//time feature
			ptrR[0] = (float)m_vecPhoto[i].dTimeStamp;
			//gps feature
			ptrR[1] = (float)m_vecPhoto[i].latitude;
			ptrR[2] = (float)m_vecPhoto[i].longitude;
		}
	}
	else{
		samples = Mat::zeros(cv::Size(TIME_FEATURE_DIM, m_vecPhoto.size()), CV_32FC1);
		for (unsigned int i = 0; i < m_vecPhoto.size(); i++){
			//time feature
			samples.at<float>(i) = (float)m_vecPhoto[i].dTimeStamp;
		}
	}
	return S_OK;
}

//EM training
HRESULT CCluster::EMtraining(Mat & Samples, bool use_gps){
	TermCriteria Termination;
	Termination.type = CV_TERMCRIT_ITER | CV_TERMCRIT_EPS;
	Mat logLikelihoods, labels, Probs;
	float this_improve = 0;
	float best_improve = -FLT_MAX;
	m_flPrevCost_MDL = FLT_MAX;
	bool trainSucc;
	float currCost = 0, bestCost = FLT_MAX;
	for (m_iK = m_iInitK; m_iK < iTotalEvent /*m_iInitK + 1*/ ; m_iK += EVENT_STEP)
	{
		printf_s("Event iteration: %d\n", m_iK);
		EM em(m_iK, cv::EM::COV_MAT_SPHERICAL, Termination);
		try
		{
			trainSucc = em.train(Samples, logLikelihoods, labels, Probs); // EM training
		}
		catch (...)
		{
			return E_FAIL;
		}
		if (trainSucc == false)
		{
			return E_FAIL;
		}
		m_flCurrML = sum(logLikelihoods)[0]; // calculate the Maximum Likelihood
		std::printf("\nround %d, loglikelihood %f,\n", m_iK, m_flCurrML);

		// MDL 
		int featDim = use_gps == true ? TIME_FEATURE_DIM + GPS_FEATURE_DIM : TIME_FEATURE_DIM;
		//int featDim = Samples.cols;
		int mk = m_iK * featDim * (featDim + 1) / 2 + (m_iK - 1) + m_iK * featDim;
		m_flCurrCost_MDL = 2 * m_iN * m_flCurrML - mk * log((double)m_iN);
		this_improve = m_flCurrCost_MDL - m_flPrevCost_MDL;

		if (m_iK == m_iInitK || this_improve > best_improve)
		{
			best_improve = this_improve;
			bestCost=currCost;
			std::printf("update bestmodel in iteration %d\n", m_iK);
			// save the best result
			m_iBestK_MDL = m_iK;

			int *plabels = labels.ptr<int>(0);
			for (unsigned int i = 0; i < m_vecPhoto.size(); i++)
			{
				m_pBestLabel_MDL[i] = plabels[i];
//				cout<<m_pBestLabel_MDL[i]<<"\t";
			}
			m_matBestPex_MDL = Probs;
			m_matBestLikelihood = logLikelihoods;
		}
		m_flPrevCost_MDL = m_flCurrCost_MDL;
	}
//	printf_s("Event iteration finish\n");
	printf_s("with %d sub-events in the final model.\n",m_iBestK_MDL);
	return S_OK;
}

//Index photos by sub event
HRESULT CCluster::BuildIndex()
{
	m_iBestK_MDL = m_vecPhoto[m_vecPhoto.size() - 1].iEventLabel + 1;
	this->indexOfEventPhotos.clear();
    this->indexOfEventPhotos.resize(m_iBestK_MDL);
	//build index for event only
	for (size_t i = 0; i < m_vecPhoto.size(); i++)
	{
		//push back the index after sorted by time
//		this->indexOfEventPhotos[m_vecPhoto[i].iEventLabel].push_back(m_vecPhoto[i].photoIndex);
		this->indexOfEventPhotos[m_vecPhoto[i].iEventLabel].push_back(i);

	}
    return S_OK;
}

//Merge the event by minimum time gap.
HRESULT CCluster::MergeEvent()
{
	int iEventlabel;
	int iNextEventlabel;
	double min_gap = 2;//if the time interval less than 2*5=10mins, merge.
	double max_gap = 720 / TIME_INTERVAL;// if the time interval exceed a day, split.
	int count = 0;
	double timegap = 0;
	HRESULT hr = S_OK;
	// sort the photos by event.
	map<int, int> EventMap; // eventLabel -> vecEventSet Index
	vector<Photo_Feature_Set>* vecEventSet = new vector<Photo_Feature_Set>[m_iBestK_MDL];
	//order by event.
	for (unsigned int i = 0; i < m_vecPhoto.size(); i++)
	{
		iEventlabel = m_vecPhoto[i].iEventLabel;//current user photo features.
		if (EventMap.find(iEventlabel) == EventMap.end())
		{
			EventMap[iEventlabel] = count;
			vecEventSet[count].push_back(m_vecPhoto[i]);
			count++;
		}
		else
		{
			vecEventSet[EventMap[iEventlabel]].push_back(m_vecPhoto[i]);
		}
	}
	m_vecPhoto.clear();
	for (int k = 0; k < m_iBestK_MDL; k++)
	{
		if (vecEventSet[k].size() > 0)
		{
			for (unsigned int j = 0; j < vecEventSet[k].size(); j++)
			{
				vecEventSet[k][j].iEventLabel = k;
				m_vecPhoto.push_back(vecEventSet[k][j]);
			}
		}
	}

	EventMap.clear();
	for (int i = 0; i < m_iBestK_MDL; i++)
	{
		vecEventSet[i].clear();
	}
	//merge by minimum gap.
	for (unsigned int i = 0; i < m_vecPhoto.size() - 1; i++)
	{
		iEventlabel = m_vecPhoto[i].iEventLabel;//current user photo features.
		iNextEventlabel = m_vecPhoto[i + 1].iEventLabel;
		timegap = m_vecPhoto[i + 1].dTimeStamp - m_vecPhoto[i].dTimeStamp;
		if (iEventlabel != iNextEventlabel)
		{
			if (timegap <= min_gap)
			{
				for (size_t j = i + 1; j < m_vecPhoto.size(); j++)
				{
					m_vecPhoto[j].iEventLabel = m_vecPhoto[j].iEventLabel - 1;
				}
			}
		}
	}

	m_iBestK_MDL = m_vecPhoto[m_vecPhoto.size() - 1].iEventLabel + 1;
	DEL_ARRAY(vecEventSet);
	return hr;
}

