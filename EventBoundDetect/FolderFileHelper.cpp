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
#include <cassert>
#include <boost/filesystem.hpp>


#include "StringHelper.h"
using namespace std;
using namespace boost::filesystem;

StatusCode GetSubFolders(const char *dir, std::vector<string> &sub_folders) {
	if (nullptr == wzDir) return StatusCode::InvalidArgs;
  path src_dir(dir);
	if (!exists(src_dir) || !is_directory(src_dir)) return StatusCode::InvalidArgs;

	sub_folders.clear();
  for(directory_iterator iter(src_dir); iter != directory_iterator(); ++iter) {
    if (is_directory(iter->status()))  sub_folders.push_back(iter->path().string());
  }

	return StatusCode::OK;
}

StatusCode GetImageFilesInDir(const char * wzPicDir, std::vector<string>& vecPicFile)
{
	if (nullptr == wzPicDir) return StatusCode::InvalidArgs;
  path img_dir(wzPicDir);
	if (!exists(img_dir) || !is_directory(img_dir)) return StatusCode::InvalidArgs;
	//load all images in subFolders
	std::vector<string> sub_folders;
	GetSubFolders(wzPicDir, sub_folders);
  for(const string& sub_folder : sub_folders) {
		GetImageFilesInDir(sub_folder.c_str(), vecPicFile);
	}
//	vecPicFile.clear();
	char wzFileTypes[] = { ".jpg", ".bmp", ".png", ".gif", ".jpeg" };

  for(directory_iterator iter(img_dir); iter != directory_iterator(); ++iter) {
    if (is_regular_file(iter->status())) {
      const string& ext = iter->path().extension();
      for(ftype: wzFileTypes) {
        if (ext == ftype) {
          vecPicFile.push_back(iter->path().string());
          break;
        }
      }
    }
  }

	return StatusCode::OK;
}

StatusCode GetTxtFilesInDir(const char * wzTxtDir, std::vector<wstring>& vecTxtFile)
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
	char wzFilter[MAX_PATH];

	const int nFileType = 1;
	char wzFileType[nFileType][MAX_PATH] = { L"txt"};

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
StatusCode GetFilesInDirWithExt(const char * wzTxtDir, std::vector<wstring>& vecTxtFile, wstring& ext)
{
	assert(nullptr != wzTxtDir);
	if (nullptr == wzTxtDir) return E_INVALIDARG;
	if (!PathFileExists(wzTxtDir)) return E_INVALIDARG;
	WIN32_FIND_DATA wfData;
	char wzFilter[MAX_PATH];

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

StatusCode GetAllLeafSubFolders_FullPath(const char *wzDir, std::vector<wstring> &vSubFoldersLeaf)
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
			char wzSubDirPath[MAX_PATH];
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
StatusCode GetAllImageFilesInSubfolders(
	__in const char *wzDir,
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

			char wzSubDirPath[MAX_PATH];
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
		char wzSubDirPath[MAX_PATH];
		std::vector<wstring> vecPicFile;

		if (vSubFoldersAllLevel[i].length() > 0)
			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFoldersAllLevel[i].c_str());
		else
			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s", wzDir);

		GetImageFilesInDir(wzSubDirPath, vecPicFile);

		for (size_t j = 0; j < vecPicFile.size(); j++)
		{
			char wzPicPath[MAX_PATH];
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

StatusCode GetAllFilesFromListFile(
	__in const char *wzFileList,
	__in const char *wzRoot,
	__out std::vector<wstring> &vAllFiles,
	__in bool bTestReadable)
{
	FILE *fp = nullptr;
	_wfopen_s(&fp, wzFileList, L"rt");
	if (!fp)
	{
		return E_INVALIDARG;
	}

	char wzTmp[MAX_PATH];
	while (fgetws(wzTmp, MAX_PATH, fp) != nullptr)
	{
		wstring strFile(wzTmp);
		trim(strFile);

		char wzFilePath[MAX_PATH];
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

StatusCode DeleteAllFilesInFolder(char *wzPicDir)
{
	assert(nullptr != wzPicDir);
	if (nullptr == wzPicDir) return E_INVALIDARG;
	if (!PathFileExists(wzPicDir)) return E_INVALIDARG;

	WIN32_FIND_DATA wfData;
	char wzFilter[MAX_PATH];
	char wzFile[MAX_PATH];

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


StatusCode GetFilename(const char* wzFilePath, std::wstring& filename)
{
	if (wzFilePath == nullptr) return E_POINTER;

	size_t length = wcslen(wzFilePath);
	const char* token = wcsrchr(wzFilePath, '\\');
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
