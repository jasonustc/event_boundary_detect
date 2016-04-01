#ifndef PVIMAGEINFO_H
#define PVIMAGEINFO_H

//-----------------definition of class CPVImageInfo---------------------------
// description:
// This class is used to read JPEG files and get informations of this photo
//

class CPVImageInfo
{
private:
	BYTE      *m_pbImgInfo;       //pointer to a buffer of file data
	int        m_iTiffOffset;     //TIFF offset
	DWORD      m_u1IFDOffset;     //1st IFD offset

	bool       m_fBigEdian;       //indicates BigEdian or LittleEndian

	WORD       m_uImageWidth;     //photo width
	WORD       m_uImageHeight;    //photo height
	struct tm m_stTakenDateTime; //photo's taken time
	DWORD      m_uThumbOffset;
	DWORD      m_uThumbLength;
	WORD       m_uThumbWidth;     //Thumbnail width
	WORD       m_uThumbHeight;    //Thumbnail height
	char       m_tszFileName[MAX_PATH];	// Image File Name, add by Tao Mei

private:
	StatusCode GetString(int iOffset, int iCount, char *pszStr) const;
	StatusCode GetUShort(int iOffset, WORD &refusRlt) const;
	StatusCode GetUInt(int iOffset, DWORD &refuRlt) const;
	StatusCode GetExifInfo();
	StatusCode GetThumbInfo();
	StatusCode GetResolution(WORD &uWidth, WORD &uHeight);

public:

	explicit CPVImageInfo();
	~CPVImageInfo();

	static StatusCode ExifDTToDateTime(const char *pszExifDT, struct tm *pst);
	int GetImageHeight() const;
	int GetImageWidth() const;
	StatusCode GetDTOrig(struct tm *pst) const;
	StatusCode SetImageFile(const char *ptszImgFile);
	void GetThumbnail(int &iOffset, int &iLength, int &iWidth, int &iHeight);

};

#endif
