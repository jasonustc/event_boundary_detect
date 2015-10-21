// PVImageInfo.cpp: implementation of the CPVImageInfo class.
//
// Author: Wangke
// Last update: 12/27/2004
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PVImageInfo.h"
#include <stdio.h>
#include <tchar.h>

//-----------------const value for reading JPEG file-------------------------
int const IMGINFO_LENGTH = 8192;
int const TAG_LENGTH = 12;

WORD const PROPERTYTAG_DTO = 0x9003;
WORD const PROPERTYTAG_DTD = 0x9004;
WORD const PROPERTYTAG_EXIFPTR = 0x8769;
WORD const PROPERTYTAG_IMGWIDTH = 0x100;
WORD const PROPERTYTAG_IMGHEIGHT = 0x101;
WORD const PROPERTYTAG_COLORSPACE = 0xA001;
WORD const PROPERTYTAG_IMGWIDTH_VALID = 0xA002;
WORD const PROPERTYTAG_IMGHEIGHT_VALID = 0xA003;
WORD const PROPERTYTAG_IMGBITS = 0x102;
WORD const PROPERTYTAG_THUMBOFFSET = 0x201;
WORD const PROPERTYTAG_THUMBLENGTH = 0x202;

enum
{
	SOF0 = 0xFFC0,
	SOF1,
	SOF2,
	SOF3,
	SOF5 = 0xFFC5,
	SOF6,
	SOF7,
	SOF9 = 0xFFC9,
	SOF10,
	SOF11,
	SOF13 = 0xFFCD,
	SOF14,
	SOF15
};

WORD const SOF_IMGWIDTH_OFFSET = 7;
WORD const SOF_IMGHEIGHT_OFFSET = 5;

/*--------------------------------------------------------------------
description:
Construction

parameter:
NONE

return:
NONE
*/
CPVImageInfo::CPVImageInfo()
{
	//initilize member data
	m_pbImgInfo = NULL;
	m_fBigEdian = true;
	m_uImageWidth = 0;
	m_uImageHeight = 0;
	m_uThumbOffset = 0;
	m_uThumbLength = 0;
	m_uThumbWidth = 0;
	m_uThumbHeight = 0;
	m_stTakenDateTime.wDay = m_stTakenDateTime.wDayOfWeek = m_stTakenDateTime.wHour =
		m_stTakenDateTime.wMilliseconds = m_stTakenDateTime.wMinute = m_stTakenDateTime.wMonth =
		m_stTakenDateTime.wSecond = m_stTakenDateTime.wYear = 0;
}

/*--------------------------------------------------------------------
description:
Destruction

parameter:
NONE

return:
NONE
*/
CPVImageInfo::~CPVImageInfo()
{
	//delete the buffer
	if (NULL != m_pbImgInfo)
	{
		delete[] m_pbImgInfo;
		m_pbImgInfo = NULL;
	}
}

/*--------------------------------------------------------------------
description:
Set file name and get information of this file

parameter:
ptszImgFile: JPEG file name

return:
HRESULT
*/
HRESULT CPVImageInfo::SetImageFile(TCHAR *ptszImgFile)
{
	_tcscpy_s(m_tszFileName, ptszImgFile);

	//create a buffer to load file data into memory
	if (NULL == m_pbImgInfo)
	{
		m_pbImgInfo = new BYTE[IMGINFO_LENGTH];
	}

	if (NULL == m_pbImgInfo)
	{
		return E_OUTOFMEMORY;
	}

	//open the file and read header info to buffer
	HANDLE hFile = CreateFile(ptszImgFile,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return E_FAIL;
	}

	DWORD dwBytesRead;
	if ((0 == ReadFile(hFile,
		m_pbImgInfo,
		IMGINFO_LENGTH,
		&dwBytesRead,
		NULL)) ||
		(dwBytesRead < IMGINFO_LENGTH)
		)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}
	CloseHandle(hFile);

	//get information of this file
	HRESULT hRlt = GetExifInfo();
	// if (S_OK != hRlt)				// Modified by Tao Mei
	if ((m_stTakenDateTime.wDay == 0) && (m_stTakenDateTime.wHour == 0)
		&& (m_stTakenDateTime.wMinute == 0) && (m_stTakenDateTime.wSecond == 0))
	{
		// GetSystemTime(&m_stTakenDateTime);		    // Original: Ke Wang
		// GetFileCreationTime(m_stTakenDateTime);		// Modified: Tao Mei

		WIN32_FIND_DATA wfd;
		HANDLE hFile = FindFirstFile(ptszImgFile, &wfd);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			// Tao Mei
			// FileTimeToSystemTime(&wfd.ftCreationTime, &m_stTakenDateTime);
			// FileTimeToSystemTime(&wfd.ftLastAccessTime, &m_stTakenDateTime);
			FileTimeToSystemTime(&wfd.ftLastWriteTime, &m_stTakenDateTime);
			FindClose(hFile);
		}

		//Get image's width and height from SOF0
		return GetResolution(m_uImageWidth, m_uImageHeight);
	}

	//open the file and read header info to buffer
	FILE *fp = NULL;	
	if (EINVAL == _tfopen_s(&fp, ptszImgFile, _T("rb")))
	{
		return E_FAIL;
	}
	if (fp == NULL)
	{
		return E_FAIL;
	}
	fseek(fp, m_u1IFDOffset, SEEK_SET);
	fread(m_pbImgInfo, 1, IMGINFO_LENGTH, fp);
	fclose(fp);

	if (S_OK != GetThumbInfo())
	{
		m_uThumbOffset = 0;
		m_uThumbLength = 0;
		m_uThumbWidth = 0;
		m_uThumbHeight = 0;
	}

	return S_OK;
}

/*--------------------------------------------------------------------
description:
Get taken time of the photo

parameter:
pst: [out] the taken time

return:
HRESULT
*/
HRESULT CPVImageInfo::GetDTOrig(SYSTEMTIME *pst) const
{
	*pst = m_stTakenDateTime;
	return S_OK;
}

/*--------------------------------------------------------------------
description:
Convert file's data info to datetime

parameter:
pszExifDT: file's data info
pst: [out] the converted datetime

return:
HRESULT
*/
HRESULT CPVImageInfo::ExifDTToDateTime(TCHAR *pszExifDT, SYSTEMTIME *pst)
{
	enum euDateTime
	{
		Year,
		Month,
		Day,
		Hour,
		Minute,
		Second
	};

	int const EUDATE_SIZE = 6;

	if (NULL == pszExifDT)
	{
		return E_POINTER;
	}

	TCHAR *pszSpace = NULL;

	if (NULL != (pszSpace = StrChr(pszExifDT, _T(' '))))
	{
		*pszSpace = _T(':');
	}
	TCHAR *next_token;
	TCHAR *ptszSeps = _T(":");
	TCHAR *ptszToken = _tcstok_s(pszExifDT, ptszSeps, &next_token);
	int  iCount = 0;

	while ((NULL != ptszToken) && (iCount < EUDATE_SIZE))
	{
		int i = _ttol(ptszToken);
		/* While there are tokens in "string" */
		switch (iCount++)
		{
		case Year:
		{
					 pst->wYear = i;
					 break;
		}

		case Month:
		{
					  pst->wMonth = i;
					  break;
		}

		case Day:
		{
					pst->wDay = i;
					break;
		}

		case Hour:
		{
					 pst->wHour = i;
					 break;
		}

		case Minute:
		{
					   pst->wMinute = i;
					   break;
		}

		case Second:
		{
					   pst->wSecond = i;
					   break;
		}

		default:
			;
		}

		/* Get next token: */
		ptszToken = _tcstok_s(NULL, ptszSeps, &next_token);
	}

	if ((NULL == ptszToken) && (iCount == EUDATE_SIZE))
	{
		return S_OK;
	}
	else
	{
		ZeroMemory(pst, sizeof(SYSTEMTIME));
		return E_FAIL;
	}
}

/*--------------------------------------------------------------------
description:
Get information of this file

parameter:
NONE

return:
HRESULT
*/
HRESULT CPVImageInfo::GetExifInfo()
{
	struct ImageProperty
	{
		WORD  usTag;   //property tag 2bytes
		WORD  usType;  //data type 2bytes
		DWORD uCount;  //number of values
		DWORD uOffset; //offset of values from TIFF header
	}
	imgPropItem;

	DWORD   uIFDOffset, uExifOffset;
	//DWORD   u1IFDOffset;
	WORD    us0thIFDFld; //number of fields in the 0th IFD
	WORD    usExifIFDFld;
	int     i;
	int     iPos = 0;
	WORD    usRlt;
	HRESULT hRlt;
	bool    fContainMakers = true;

	if (NULL == m_pbImgInfo)
	{
		return E_POINTER;
	}

	//1.Check image info
	//check SOI(start of image) Marker
	if ((m_pbImgInfo[0] != 0xFF) || (m_pbImgInfo[1] != 0xD8))
	{
		//The file does not contain SOI marker
		return E_UNEXPECTED;
	}
	//check whethe APP0 marker exists(JFIF Section)
	if ((0xFF == m_pbImgInfo[2]) && (0xE0 == m_pbImgInfo[3]))
	{
		//JFIF Section Exists, then read its field length
		m_fBigEdian = true;
		if (S_OK != (hRlt = GetUShort(4, usRlt)))
		{
			return hRlt;
		}
		iPos = 4 + (int)usRlt; //iOffset + APP0 length
	}
	else
	{
		iPos = 2;
	}

	//check APP1 marker
	if ((iPos < 0) || (iPos + 7 >= IMGINFO_LENGTH))
	{
		return E_FAIL;
	}

	//APP1 Marker
	if ((m_pbImgInfo[iPos] != 0xFF) || (m_pbImgInfo[iPos + 1] != 0xE1) ||
		(m_pbImgInfo[iPos + 4] != 'E') || (m_pbImgInfo[iPos + 5] != 'x') ||
		(m_pbImgInfo[iPos + 6] != 'i') || (m_pbImgInfo[iPos + 7] != 'f'))
	{
		//The file does not contain APP1 marker

		return E_UNEXPECTED;
	}

	//2.read tiff header to get 0th IFD's iOffset
	m_iTiffOffset = iPos + 0x0A;  //attribute information
	if ((m_iTiffOffset < 0) || (m_iTiffOffset >= IMGINFO_LENGTH))
	{
		return E_FAIL;
	}

	if (0x49 == m_pbImgInfo[m_iTiffOffset])
	{
		m_fBigEdian = false;
	}
	else if (0x4D == m_pbImgInfo[m_iTiffOffset])
	{
		m_fBigEdian = true;
	}
	else
	{
		//The TIFF header does not contain byte order of 0th IFD data
		return E_UNEXPECTED;
	}

	if (S_OK != (hRlt = GetUShort(m_iTiffOffset + 2, usRlt)))
	{
		return hRlt;
	}

	if (42 != (int)usRlt)
	{
		//The TIFF header does not contain '42' field
		return E_UNEXPECTED;
	}

	if (S_OK != (hRlt = GetUInt(m_iTiffOffset + 4, uIFDOffset)))
	{
		return hRlt;
	}

	//3.read the 0th IFD to get exif IFD
	hRlt = GetUShort(m_iTiffOffset + (int)uIFDOffset, us0thIFDFld);
	if (S_OK != hRlt)
	{
		return hRlt;
	}

	iPos = m_iTiffOffset + (int)uIFDOffset + 2;
	uExifOffset = 0;
	for (i = 0; i < us0thIFDFld; i++)
	{
		if (S_OK != (hRlt = GetUShort(iPos, usRlt)))
		{
			return hRlt;
		}

		if (PROPERTYTAG_EXIFPTR == usRlt)
		{
			if (S_OK != (hRlt = GetUInt(iPos + 8, uExifOffset)))
			{
				return hRlt;
			}

			//break;
		}

		iPos += TAG_LENGTH;
	}//for    

	//get the 1st IFD offset
	if (S_OK == GetUInt(iPos, m_u1IFDOffset))
	{
		m_u1IFDOffset += m_iTiffOffset;
	}

	if (uExifOffset <= (unsigned)m_iTiffOffset)
	{
		//The Exif iOffset is error
		return E_UNEXPECTED;
	}

	//4.read the exif IFD
	hRlt = GetUShort(m_iTiffOffset + (int)uExifOffset, usExifIFDFld);
	if (S_OK != hRlt)
	{
		return hRlt;
	}

	iPos = m_iTiffOffset + (int)uExifOffset + 2;
	for (i = 0; i < usExifIFDFld; ++i)
	{
		if (S_OK != (hRlt = GetUShort(iPos, imgPropItem.usTag)))
		{
			return hRlt;
		}

		if (S_OK != (hRlt = GetUShort(iPos + 2, imgPropItem.usType)))
		{
			return hRlt;
		}

		if (S_OK != (hRlt = GetUInt(iPos + 4, imgPropItem.uCount)))
		{
			return hRlt;
		}

		if (S_OK != (hRlt = GetUInt(iPos + 8, imgPropItem.uOffset)))
		{
			return hRlt;
		}

		//datetime
		if (PROPERTYTAG_DTO == imgPropItem.usTag)
		{
			TCHAR szStr[MAX_PATH];

			hRlt = GetString(imgPropItem.uOffset + m_iTiffOffset,
				imgPropItem.uCount,
				szStr);
			if (S_OK == hRlt)
			{
				hRlt = ExifDTToDateTime(szStr, &m_stTakenDateTime);
			}

			if (S_OK != hRlt)
			{
				return hRlt;
			}
		}

		//width and height
		if (PROPERTYTAG_IMGWIDTH_VALID == imgPropItem.usTag)
		{
			m_uImageWidth = (unsigned)imgPropItem.uOffset;
		}

		if (PROPERTYTAG_IMGHEIGHT_VALID == imgPropItem.usTag)
		{
			m_uImageHeight = (unsigned)imgPropItem.uOffset;
		}

		iPos += TAG_LENGTH;
	}//for

	if ((m_uImageWidth == 0) || (m_uImageHeight == 0))
	{
		return GetResolution(m_uImageWidth, m_uImageHeight);
	}
	else
	{
		return S_OK;
	}
}

/*--------------------------------------------------------------------
description:
Get a value(unsigned int) from the offset of the buffer

parameter:
iOffset: the offset of the buffer
refuRlt: [out] the value

return:
HRESULT
*/
HRESULT CPVImageInfo::GetUInt(int iOffset, DWORD &refuRlt) const
{
	if ((iOffset < 0) || (iOffset + 3 >= IMGINFO_LENGTH))
	{
		return E_INVALIDARG;
	}

	if (m_fBigEdian)
	{
		refuRlt = ((DWORD)m_pbImgInfo[iOffset] << 24) +
			((DWORD)m_pbImgInfo[iOffset + 1] << 16) +
			((DWORD)m_pbImgInfo[iOffset + 2] << 8) +
			((DWORD)m_pbImgInfo[iOffset + 3]);
	}
	else
	{
		refuRlt = (DWORD)m_pbImgInfo[iOffset] +
			((DWORD)m_pbImgInfo[iOffset + 1] << 8) +
			((DWORD)m_pbImgInfo[iOffset + 2] << 16) +
			((DWORD)m_pbImgInfo[iOffset + 3] << 24);
	}

	return S_OK;
}

/*--------------------------------------------------------------------
description:
Get a value(unsigned short) from the offset of the buffer

parameter:
iOffset: the offset of the buffer
refusRlt: [out] the value

return:
HRESULT
*/
HRESULT CPVImageInfo::GetUShort(int iOffset, WORD &refusRlt) const
{
	if ((iOffset < 0) || (iOffset + 1 >= IMGINFO_LENGTH))
	{
		return E_INVALIDARG;
	}

	if (m_fBigEdian)
	{
		refusRlt = (WORD)(((DWORD)m_pbImgInfo[iOffset] << 8) +
			(DWORD)m_pbImgInfo[iOffset + 1]);
	}
	else
	{
		refusRlt = (WORD)((DWORD)m_pbImgInfo[iOffset] +
			((DWORD)m_pbImgInfo[iOffset + 1] << 8));
	}

	return S_OK;
}

/*--------------------------------------------------------------------
description:
Get a value(char *) from the offset of the buffer

parameter:
iOffset: the offset of the buffer
iCount: the size of the data
pszStr: [out] the value

return:
HRESULT
*/
HRESULT CPVImageInfo::GetString(int iOffset, int iCount, TCHAR *pszStr) const
{
	if (NULL == pszStr)
	{
		return E_POINTER;
	}

	if ((iOffset < 0) || (iOffset + iCount >= IMGINFO_LENGTH))
	{
		return E_INVALIDARG;
	}
	int i;
	for (i = 0; i < iCount; ++i)
	{
		pszStr[i] = (TCHAR)m_pbImgInfo[iOffset + i];
	}

	pszStr[i] = _T('\0');
	return S_OK;
}

/*--------------------------------------------------------------------
description:
Get the image's width

parameter:
NONE

return:
the image's width
*/
int CPVImageInfo::GetImageWidth() const
{
	return m_uImageWidth;
}

/*--------------------------------------------------------------------
description:
Get the image's height

parameter:
NONE

return:
the image's height
*/
int CPVImageInfo::GetImageHeight() const
{
	return m_uImageHeight;
}



/*--------------------------------------------------------------------
description:
Get the image's GetThumbnail's offset and size

parameter:
NONE

return:
HRESULT
*/
HRESULT CPVImageInfo::GetThumbInfo()
{
	WORD    nFld;
	int     iPos = 0;
	HRESULT hRlt = S_OK;

	//Get thumbnail's offset and length
	if (S_OK != (hRlt = GetUShort(iPos, nFld)))
	{
		return hRlt;
	}

	iPos = iPos + 2;

	for (int i = 0; i < nFld; i++)
	{
		WORD uData;

		if (S_OK != (hRlt = GetUShort(iPos, uData)))
		{
			return hRlt;
		}

		switch (uData)
		{
		case PROPERTYTAG_THUMBOFFSET:
		{
										if (S_OK != (hRlt = GetUInt(iPos + 8, m_uThumbOffset)))
										{
											return hRlt;
										}
										m_uThumbOffset += m_iTiffOffset;
										break;
		}

		case PROPERTYTAG_THUMBLENGTH:
		{
										if (S_OK != (hRlt = GetUInt(iPos + 8, m_uThumbLength)))
										{
											return hRlt;
										}
										m_uThumbLength += m_iTiffOffset;
										break;
		}
		}//switch

		iPos += TAG_LENGTH;

	}//for


	//Get thumbnail's width and height from SOF0
	GetResolution(m_uThumbWidth, m_uThumbHeight);

	return S_OK;
}


/*--------------------------------------------------------------------
description:
Get the image's GetThumbnail's offset and size

parameter:
iOffset: Offset of the thumbnail position in the file
iLength: Lenght of the thumbnail
iWidth : Width of the thumbnail
iHeight: Height of the thumbnail

return:
NONE
*/
void CPVImageInfo::GetThumbnail(int &iOffset, int &iLength, int &iWidth, int &iHeight)
{
	iOffset = m_uThumbOffset;
	iLength = m_uThumbLength;
	iWidth = m_uThumbWidth;
	iHeight = m_uThumbHeight;
}

/*--------------------------------------------------------------------
description:
Get width and height from SOF0

parameter:
uWidth:  [out]Width
uHeight: [out]Height

return:
HRESULT
*/
HRESULT CPVImageInfo::GetResolution(WORD &uWidth, WORD &uHeight)
{
	int     iPos = IMGINFO_LENGTH - sizeof(WORD);
	HRESULT hRlt = S_OK;
	WORD    wTag;

	m_fBigEdian = true;

	while (iPos >= 0)
	{
		if (S_OK != (hRlt = GetUShort(iPos, wTag)))
		{
			return hRlt;
		}

		if ((wTag == SOF0) ||
			(wTag == SOF1) ||
			(wTag == SOF2) ||
			(wTag == SOF3) ||
			(wTag == SOF5) ||
			(wTag == SOF6) ||
			(wTag == SOF7) ||
			(wTag == SOF9) ||
			(wTag == SOF10) ||
			(wTag == SOF11) ||
			(wTag == SOF13) ||
			(wTag == SOF14) ||
			(wTag == SOF15))
		{
			if (S_OK != (hRlt = GetUShort(iPos + SOF_IMGWIDTH_OFFSET, uWidth)))
			{
				return hRlt;
			}

			if (S_OK != (hRlt = GetUShort(iPos + SOF_IMGHEIGHT_OFFSET, uHeight)))
			{
				return hRlt;
			}

			if ((uWidth != m_uThumbWidth) && (uHeight != m_uThumbHeight))
			{
				break;
			}
		}

		--iPos;
	}

	return S_OK;
}

HRESULT CPVImageInfo::GetFileCreationTime(SYSTEMTIME &SysTime)
{
	HRESULT hr = S_OK;

	WIN32_FIND_DATA FindFileData;
	HANDLE hd = FindFirstFile(m_tszFileName, &FindFileData);
	if (INVALID_HANDLE_VALUE != hd)
	{
		FileTimeToSystemTime(&FindFileData.ftCreationTime, &SysTime);
		//		FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &SysTime);
	}
	else
	{
		printf("\nError: Find file creation time failed!\n");
		return E_FAIL;
	}

	return hr;
}
