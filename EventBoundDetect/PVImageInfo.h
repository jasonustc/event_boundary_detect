#ifndef PVIMAGEINFO_H
#define PVIMAGEINFO_H

//-----------------definition of class CPVImageInfo---------------------------
// description:
// This class is used to read JPEG files and get informations of this photo
//
#include <Shlwapi.h>

class CPVImageInfo
{
private:
	BYTE      *m_pbImgInfo;       //pointer to a buffer of file data
	int        m_iTiffOffset;     //TIFF offset
	DWORD      m_u1IFDOffset;     //1st IFD offset

	bool       m_fBigEdian;       //indicates BigEdian or LittleEndian

	WORD       m_uImageWidth;     //photo width
	WORD       m_uImageHeight;    //photo height
	SYSTEMTIME m_stTakenDateTime; //photo's taken time
	DWORD      m_uThumbOffset;
	DWORD      m_uThumbLength;
	WORD       m_uThumbWidth;     //Thumbnail width
	WORD       m_uThumbHeight;    //Thumbnail height
	TCHAR       m_tszFileName[MAX_PATH];	// Image File Name, add by Tao Mei

private:
	HRESULT GetFileCreationTime(SYSTEMTIME& SysTime);

	HRESULT GetString(int iOffset, int iCount, TCHAR *pszStr) const;
	HRESULT GetUShort(int iOffset, WORD &refusRlt) const;
	HRESULT GetUInt(int iOffset, DWORD &refuRlt) const;
	HRESULT GetExifInfo();
	HRESULT GetThumbInfo();
	HRESULT GetResolution(WORD &uWidth, WORD &uHeight);

public:

	explicit CPVImageInfo();
	~CPVImageInfo();

	static HRESULT ExifDTToDateTime(TCHAR *pszExifDT, SYSTEMTIME *pst);
	int GetImageHeight() const;
	int GetImageWidth() const;
	HRESULT GetDTOrig(SYSTEMTIME *pst) const;
	HRESULT SetImageFile(TCHAR *ptszImgFile);
	void GetThumbnail(int &iOffset, int &iLength, int &iWidth, int &iHeight);

};

#endif
