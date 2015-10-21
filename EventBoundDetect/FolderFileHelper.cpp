// 
// Copyright (c) Microsoft Corporation. All rights reserved. 
// 
// File: FolderFileHelper.cpp
// 
// Description: 
//     Implements helper functions to access the files inside a folder on disk.
// 
#include "stdafx.h"
#include "FolderFileHelper.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <cassert>
#include <strsafe.h>
#include "StringHelper.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

using namespace std;

HRESULT GetSubFolders(const WCHAR *wzDir, std::vector<wstring> &vSubFolders)
{
	assert(nullptr != wzDir);
	if (nullptr == wzDir) return E_INVALIDARG;
	if (!PathFileExists(wzDir)) return E_INVALIDARG;

	vSubFolders.clear();

	WIN32_FIND_DATA wfData;
	WCHAR wzFilter[MAX_PATH];
	StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*", wzDir);
	HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return E_INVALIDARG;
	}
	else
	{
		BOOL fNext = TRUE;
		while (TRUE == fNext)
		{
			if (FILE_ATTRIBUTE_DIRECTORY ==
				(wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wstring strFolderName(wfData.cFileName);
				if (0 != strFolderName.compare(L".") && 0 != strFolderName.compare(L".."))
				{
					vSubFolders.push_back(strFolderName);
				}
			}
			fNext = ::FindNextFileW(hFind, &wfData);
		}
		::FindClose(hFind);
	}

	return S_OK;
}

HRESULT GetImageFilesInDir(const WCHAR * wzPicDir, std::vector<wstring>& vecPicFile)
{
	assert(nullptr != wzPicDir);
	if (nullptr == wzPicDir) return E_INVALIDARG;
	if (!PathFileExists(wzPicDir)) return E_INVALIDARG;
	//load all images in subFolders
	std::vector<wstring> subFolders;
	GetSubFolders(wzPicDir, subFolders);
	for (unsigned int i = 0; i < subFolders.size(); i++){
		wstring fullPath(wzPicDir);
		fullPath = fullPath + subFolders[i];
		GetImageFilesInDir(fullPath.c_str(), vecPicFile);
	}
//	vecPicFile.clear();
	WIN32_FIND_DATA wfData;
	WCHAR wzFilter[MAX_PATH];

	const int nFileType = 5;
	WCHAR wzFileType[nFileType][MAX_PATH] = { L"jpg", L"bmp", L"png", L"gif", L"jpeg" };

	for (int i = 0; i < nFileType; i++)
	{
		StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.%s", wzPicDir, wzFileType[i]);
		HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
		if (INVALID_HANDLE_VALUE == hFind) //we couldn't find any jpeg files under specified folder
		{
			//there are no jpg files under specified folder.     
		}
		else
		{
			BOOL fNext = TRUE;
			while (TRUE == fNext)
			{
				if (FILE_ATTRIBUTE_DIRECTORY ==
					(wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					fNext = ::FindNextFileW(hFind, &wfData);
					continue;
				}

				wstring strPicFile(wfData.cFileName);
				wstring folderPath(wzPicDir);
				strPicFile = folderPath + _T("\\") + strPicFile;
				vecPicFile.push_back(strPicFile);

				fNext = ::FindNextFileW(hFind, &wfData);

			}
			::FindClose(hFind);
		}
	}

	return S_OK;

}

HRESULT GetTxtFilesInDir(const WCHAR * wzTxtDir, std::vector<wstring>& vecTxtFile)
{
	assert(nullptr != wzTxtDir);
	if (nullptr == wzTxtDir) return E_INVALIDARG;
	if (!PathFileExists(wzTxtDir)) return E_INVALIDARG;
	//load all images in subFolders
	std::vector<wstring> subFolders;
	GetSubFolders(wzTxtDir, subFolders);
	for (unsigned int i = 0; i < subFolders.size(); i++){
		wstring fullPath(wzTxtDir);
		fullPath = fullPath + subFolders[i];
		GetTxtFilesInDir(fullPath.c_str(), vecTxtFile);
	}
//	vecTxtFile.clear();
	WIN32_FIND_DATA wfData;
	WCHAR wzFilter[MAX_PATH];

	const int nFileType = 1;
	WCHAR wzFileType[nFileType][MAX_PATH] = { L"txt"};

	for (int i = 0; i < nFileType; i++)
	{
		StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.%s", wzTxtDir, wzFileType[i]);
		HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
		if (INVALID_HANDLE_VALUE == hFind) //we couldn't find any jpeg files under specified folder
		{
			//there are no txt files under specified folder.     
		}
		else
		{
			BOOL fNext = TRUE;
			while (TRUE == fNext)
			{
				if (FILE_ATTRIBUTE_DIRECTORY ==
					(wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					fNext = ::FindNextFileW(hFind, &wfData);
					continue;
				}

				wstring strPicFile(wfData.cFileName);
				wstring folderPath(wzTxtDir);
				strPicFile = folderPath + _T("\\") + strPicFile;
				vecTxtFile.push_back(strPicFile);

				fNext = ::FindNextFileW(hFind, &wfData);

			}
			::FindClose(hFind);
		}
	}

	return S_OK;

}

//get files in dir with given extention ext
//only search for files in the current folder
HRESULT GetFilesInDirWithExt(const WCHAR * wzTxtDir, std::vector<wstring>& vecTxtFile, wstring& ext)
{
	assert(nullptr != wzTxtDir);
	if (nullptr == wzTxtDir) return E_INVALIDARG;
	if (!PathFileExists(wzTxtDir)) return E_INVALIDARG;
	WIN32_FIND_DATA wfData;
	WCHAR wzFilter[MAX_PATH];

	const int nFileType = 1;

	for (int i = 0; i < nFileType; i++)
	{
		StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.%s", wzTxtDir, ext.c_str());
		HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
		if (INVALID_HANDLE_VALUE == hFind) 
		{
			//there are no txt files under specified folder.     
		}
		else
		{
			BOOL fNext = TRUE;
			while (TRUE == fNext)
			{
				if (FILE_ATTRIBUTE_DIRECTORY ==
					(wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					fNext = ::FindNextFileW(hFind, &wfData);
					continue;
				}

				wstring strPicFile(wfData.cFileName);
				wstring folderPath(wzTxtDir);
				strPicFile = folderPath + _T("\\") + strPicFile;
				vecTxtFile.push_back(strPicFile);

				fNext = ::FindNextFileW(hFind, &wfData);

			}
			::FindClose(hFind);
		}
	}

	return S_OK;

}

HRESULT GetAllLeafSubFolders_FullPath(const WCHAR *wzDir, std::vector<wstring> &vSubFoldersLeaf)
{
	vSubFoldersLeaf.clear();

	//recursively collect all subforders
	std::vector<wstring> vSubFolders;
	GetSubFolders(wzDir, vSubFolders);

	while (vSubFolders.size() > 0)
	{
		std::vector<wstring> vSubFoldersTmp;

		for (size_t i = 0; i < vSubFolders.size(); i++)
		{
			WCHAR wzSubDirPath[MAX_PATH];
			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFolders[i]);

			std::vector<wstring> vSubFoldersBottom;
			GetSubFolders(wzSubDirPath, vSubFoldersBottom);

			if (vSubFoldersBottom.size() == 0)
			{
				vSubFoldersLeaf.push_back(wzSubDirPath);
				wprintf(L"%s\n", wzSubDirPath);
			}
			else
			{
				for (size_t j = 0; j < vSubFoldersBottom.size(); j++)
				{
					wstring strTmp = vSubFolders[i] + L"\\" + vSubFoldersBottom[j];
					vSubFoldersTmp.push_back(strTmp);
				}
			}
		}

		vSubFolders.swap(vSubFoldersTmp);
	}

	return S_OK;
}

//recursively collect all subforders
HRESULT GetAllImageFilesInSubfolders(
	__in const WCHAR *wzDir,
	__out std::vector<wstring> &vAllFiles,
	__in bool bTestReadable)
{
	vAllFiles.clear();

	std::vector<wstring> vSubFoldersAllLevel;

	vSubFoldersAllLevel.push_back(L"");   //includ ./

	//recursively collect all subforders
	std::vector<wstring> vSubFolders;
	GetSubFolders(wzDir, vSubFolders);

	while (vSubFolders.size() > 0)
	{
		std::vector<wstring> vSubFoldersTmp;

		for (size_t i = 0; i < vSubFolders.size(); i++)
		{
			vSubFoldersAllLevel.push_back(vSubFolders[i]);

			WCHAR wzSubDirPath[MAX_PATH];
			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFolders[i].c_str());

			wprintf(L"%s\n", wzSubDirPath);

			std::vector<wstring> vSubFoldersBottom;
			GetSubFolders(wzSubDirPath, vSubFoldersBottom);

			for (size_t j = 0; j < vSubFoldersBottom.size(); j++)
			{
				wstring strTmp = vSubFolders[i] + L"\\" + vSubFoldersBottom[j];
				vSubFoldersTmp.push_back(strTmp);
			}
		}

		vSubFolders.clear();
		for (size_t i = 0; i < vSubFoldersTmp.size(); i++)
		{
			vSubFolders.push_back(vSubFoldersTmp[i]);
		}
	}

	//////////////////////////////////
	size_t iProgress = 0;
	for (size_t i = 0; i < vSubFoldersAllLevel.size(); i++)
	{
		WCHAR wzSubDirPath[MAX_PATH];
		std::vector<wstring> vecPicFile;

		if (vSubFoldersAllLevel[i].length() > 0)
			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFoldersAllLevel[i].c_str());
		else
			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s", wzDir);

		GetImageFilesInDir(wzSubDirPath, vecPicFile);

		for (size_t j = 0; j < vecPicFile.size(); j++)
		{
			WCHAR wzPicPath[MAX_PATH];
			StringCchPrintfW(wzPicPath, MAX_PATH, L"%s\\%s", wzSubDirPath, vecPicFile[j].c_str());

			wstring strFile(wzPicPath);
			if (bTestReadable)
			{
				Gdiplus::Bitmap bmp(wzPicPath);
				if (bmp.GetWidth() != 0)
				{
					vAllFiles.push_back(strFile);
				}
				else
				{
					wprintf(L"Read failed: %s\n", wzPicPath);
				}
			}
			else
			{
				vAllFiles.push_back(strFile);
			}
		}

		size_t iP = (i * 100) / vSubFoldersAllLevel.size();
		if (iP > iProgress)
		{
			iProgress = iP;
			//printf( "\b\b\b\b\b\b\b\b\b\b%d%", iProgress );
		}
	}

	return S_OK;
}

HRESULT GetAllFilesFromListFile(
	__in const WCHAR *wzFileList,
	__in const WCHAR *wzRoot,
	__out std::vector<wstring> &vAllFiles,
	__in bool bTestReadable)
{
	FILE *fp = nullptr;
	_wfopen_s(&fp, wzFileList, L"rt");
	if (!fp)
	{
		return E_INVALIDARG;
	}

	WCHAR wzTmp[MAX_PATH];
	while (fgetws(wzTmp, MAX_PATH, fp) != nullptr)
	{
		wstring strFile(wzTmp);
		trim(strFile);

		WCHAR wzFilePath[MAX_PATH];
		if (nullptr != wzRoot)
		{
			StringCchPrintfW(wzFilePath, MAX_PATH, L"%s\\%s", wzRoot, strFile);
		}
		else
		{
			StringCchPrintfW(wzFilePath, MAX_PATH, L"%s", strFile);
		}
		wstring strFilePath(wzFilePath);
		if (bTestReadable)
		{
			Gdiplus::Bitmap bmp(wzFilePath);
			if (bmp.GetWidth() != 0)
			{
				vAllFiles.push_back(strFilePath);
			}
		}
		else
		{
			vAllFiles.push_back(strFilePath);
		}

	}
	fclose(fp);
	return S_OK;
}

HRESULT DeleteAllFilesInFolder(WCHAR *wzPicDir)
{
	assert(nullptr != wzPicDir);
	if (nullptr == wzPicDir) return E_INVALIDARG;
	if (!PathFileExists(wzPicDir)) return E_INVALIDARG;

	WIN32_FIND_DATA wfData;
	WCHAR wzFilter[MAX_PATH];
	WCHAR wzFile[MAX_PATH];

	StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.*", wzPicDir);
	HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);

	if (INVALID_HANDLE_VALUE != hFind) //we couldn't find any file
	{
		BOOL fNext = TRUE;
		while (TRUE == fNext)
		{
			if (FILE_ATTRIBUTE_DIRECTORY ==
				(wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				fNext = ::FindNextFileW(hFind, &wfData);
				continue;
			}

			StringCchPrintfW(wzFile, MAX_PATH, L"%s\\%s", wzPicDir, wfData.cFileName);
			if (FALSE == ::DeleteFileW(wzFile))
			{
				wprintf(L"Cannot delete file %s\n", wfData.cFileName);
			}

			fNext = ::FindNextFileW(hFind, &wfData);

		}
		::FindClose(hFind);
	}

	return S_OK;
}
HRESULT GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	unsigned int  num = 0;    // number of image encoders
	unsigned int  size = 0;   // size of the image encoder array in bytes

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)return -1;

	Gdiplus::ImageCodecInfo* imageCodecInfo = new Gdiplus::ImageCodecInfo[size];
	Gdiplus::GetImageEncoders(num, size, imageCodecInfo);

	for (unsigned int i = 0; i < num; ++i)
	{
		if (wcscmp(imageCodecInfo[i].MimeType, format) == 0)
		{
			*pClsid = imageCodecInfo[i].Clsid;
			delete[] imageCodecInfo;
			return i;
		}
	}
	delete[] imageCodecInfo;
	return -1;
}

bool ChooseFolder(HWND hParent, const wstring& title, wstring& folder)
{
	bool success = false;

	BROWSEINFO bi;
	::ZeroMemory(&bi, sizeof(bi));

	WCHAR pBuffer[MAX_PATH] = { 0 };

	bi.hwndOwner = hParent;
	bi.pszDisplayName = pBuffer;
	bi.lpszTitle = title.c_str();
	bi.pidlRoot = 0;
	bi.ulFlags = BIF_RETURNONLYFSDIRS |
		BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pItem = ::SHBrowseForFolder(&bi);
	if (pItem != nullptr)
	{
		::SHGetPathFromIDList(pItem, pBuffer);
		success = true;

		IMalloc* pMalloc = nullptr;
		if (SUCCEEDED(::SHGetMalloc(&pMalloc)))
			pMalloc->Free(pItem);
	}

	folder = wstring(pBuffer);
	return success;
}

HRESULT GetFilename(const WCHAR* wzFilePath, std::wstring& filename)
{
	if (wzFilePath == nullptr) return E_POINTER;

	size_t length = wcslen(wzFilePath);
	const WCHAR* token = wcsrchr(wzFilePath, '\\');
	filename.assign((token ? token + 1 : wzFilePath), wzFilePath + length);

	return S_OK;
}

bool CopyFileToFolder(const string &filePath, const string &folderPath){
	if (_access(folderPath.c_str(), 0) == -1){
		_mkdir(folderPath.c_str());
	}
	string oriImgPath = filePath;
	unsigned idx = oriImgPath.rfind("\\");
	string imgName = oriImgPath.substr(idx);
	string newImgPath = folderPath + imgName;
	CopyFileA(filePath.c_str(), newImgPath.c_str(), FALSE);
	return true;
}

bool MakeSubFolder(const string &parentFolder, const string& subFolder, string& fullFolderPath){
	fullFolderPath = parentFolder + "\\" + subFolder;
	if (_access(fullFolderPath.c_str(), 0) == -1){
		_mkdir(fullFolderPath.c_str());
		return true;
	}
	return false;
}

int GetDirectoryName(const string& filePath, string& dirName, string& fileName){
	int idx = filePath.find_last_of("\\");
	dirName = idx == -1 ? "" : filePath.substr(0, idx);
	fileName = filePath.substr(idx + 1);
	return 0;
}

bool IsInSameFolder(const string& file1, const string& file2){
	string dir1, fileName1, dir2, fileName2;
	GetDirectoryName(file1, dir1, fileName1);
	GetDirectoryName(file2, dir2, fileName2);
	return dir1 == dir2;
}

bool GetFileNameAndExt(const string& filePath, string& fileName, string& ext){
	int idx = filePath.find_last_of(".");
	fileName = filePath.substr(0, idx);
	ext = filePath.substr(idx + 1);
	return true;
}

bool FileExist(const string& filePath){
	return(_access(filePath.c_str(), 4) != -1);
}