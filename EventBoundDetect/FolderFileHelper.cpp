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

int GetDirectoryName(const string& filePath, string& dirName, string& fileName){
  path file_path(filePath);
  dirName = file_path.parent_path().string();
  fileName = file_path.filename().string();
	return 0;
}

bool MakeSubFolder(const string &parentFolder, const string& subFolder, string& fullFolderPath) {
  path full_path = path(parentFolder) / subFolder;
	if (!exists(full_path)){
    create_directory(full_path);
    fullFolderPath = full_path.string();
		return true;
	}
	return false;
}

bool CopyFileToFolder(const string &filePath, const string &folderPath){
  path src_path (filePath);
  path dst_path(folderPath);
  if (!exists(dst_path)) {
    create_directory(dst_path);
  }
  dst_path = dst_path / src_path.filename();
  if (!exists(dst_path)) {
    copy_file(src_path,dst_path);
  }
	return true;
}

StatusCode GetSubFolders(const char *dir, std::vector<string> &sub_folders) {
	if (nullptr == dir) return StatusCode::InvalidArgs;
  path src_dir(dir);
	if (!exists(src_dir) || !is_directory(src_dir)) return StatusCode::InvalidArgs;

	sub_folders.clear();
  for(directory_iterator iter(src_dir); iter != directory_iterator(); ++iter) {
    if (is_directory(iter->status()))  sub_folders.push_back(iter->path().string());
  }

	return StatusCode::OK;
}

StatusCode GetTxtFilesInDir(const char * wzTxtDir, std::vector<string>& vecTxtFile)
{
	if (nullptr == wzTxtDir) return StatusCode::InvalidArgs;
  path txt_dir(wzTxtDir);
	if (!exists(txt_dir) || !is_directory(txt_dir)) return StatusCode::InvalidArgs;
	//load all images in subFolders
	std::vector<string> sub_folders;
	GetSubFolders(wzTxtDir, sub_folders);
  for(const string& sub_folder : sub_folders) {
		GetTxtFilesInDir(sub_folder.c_str(), vecTxtFile);
	}
	string wzFileTypes[] = { ".txt" };

  for(directory_iterator iter(txt_dir); iter != directory_iterator(); ++iter) {
    if (is_regular_file(iter->status())) {
      const string& ext = iter->path().extension().string();
      for(const string& ftype: wzFileTypes) {
        if (ext == ftype) {
          vecTxtFile.push_back(iter->path().string());
          break;
        }
      }
    }
  }

	return StatusCode::OK;
}

void MakeDir(const string& dir) {
  path dir_path(dir);
  if (!exists(dir_path)) {
    create_directory(dir_path);
  }
}

bool IsInSameFolder(const string& file1, const string& file2){
	return path(file1).parent_path().string() == path(file2).parent_path().string();
}

bool FileExist(const string& filePath){
  return exists(filePath);
}

time_t LastWriteTime(const string &file_path) {
  return last_write_time(file_path);
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
	string wzFileTypes[] = { ".jpg", ".bmp", ".png", ".gif", ".jpeg" };

  for(directory_iterator iter(img_dir); iter != directory_iterator(); ++iter) {
    if (is_regular_file(iter->status())) {
      const string& ext = iter->path().extension().string();
      for(const string& ftype: wzFileTypes) {
        if (ext == ftype) {
          vecPicFile.push_back(iter->path().string());
          break;
        }
      }
    }
  }

	return StatusCode::OK;
}

//get files in dir with given extention ext
//only search for files in the current folder
StatusCode GetFilesInDirWithExt(const char * wzTxtDir, const string& ext, std::vector<string>& vecTxtFile)
{
	if (nullptr == wzTxtDir) return StatusCode::InvalidArgs;
  path txt_dir(wzTxtDir);
	if (!exists(txt_dir) || !is_directory(txt_dir)) return StatusCode::InvalidArgs;

  for(directory_iterator iter(txt_dir); iter != directory_iterator(); ++iter) {
    if (is_regular_file(iter->status()) && ext == iter->path().extension().string()) {
      vecTxtFile.push_back(iter->path().string());
    }
  }

	return StatusCode::OK;
}

//StatusCode GetAllLeafSubFolders_FullPath(const char *wzDir, std::vector<string> &vSubFoldersLeaf) {
//	vSubFoldersLeaf.clear();
//
//	//recursively collect all subforders
//	std::vector<string> vSubFolders;
//	GetSubFolders(wzDir, vSubFolders);
//
//	while (vSubFolders.size() > 0) {
//		std::vector<string> vSubFoldersTmp;
//    for(const string& sub_folder: vSubFolders) {
//			std::vector<string> vSubFoldersBottom;
//			GetSubFolders(sub_folder, vSubFoldersBottom);
//
//			if (vSubFoldersBottom.size() == 0) {
//				vSubFoldersLeaf.push_back(sub_folder);
//				printf("%s\n", sub_folder.c_str());
//			}
//			else {
//        for(const string& sub_folder_btm: vSubFoldersBottom) {
//					vSubFoldersTmp.push_back(sub_folder_btm);
//				}
//			}
//		}
//
//		vSubFolders.swap(vSubFoldersTmp);
//	}
//
//	return StatusCode::OK;
//}
//
////recursively collect all subforders
//StatusCode GetAllImageFilesInSubfolders(const char *wzDir,
//    std::vector<string> &vAllFiles,
//    bool bTestReadable) {
//	vAllFiles.clear();
//
//	std::vector<string> vSubFoldersAllLevel;
//
//	vSubFoldersAllLevel.push_back("");   //includ ./
//
//	//recursively collect all subforders
//	std::vector<string> vSubFolders;
//	GetSubFolders(wzDir, vSubFolders);
//
//	while (vSubFolders.size() > 0) {
//		std::vector<string> vSubFoldersTmp;
//
//    for(const string& sub_folder: vSubFolders) {
//			vSubFoldersAllLevel.push_back(sub_folder);
//
//			printf("%s\n", sub_folder.c_str());
//
//			std::vector<string> vSubFoldersBottom;
//			GetSubFolders(sub_folder.c_str(), vSubFoldersBottom);
//
//      for(const string& sub_folder_btm : vSubFoldersBottom) {
//				vSubFoldersTmp.push_back(sub_folder_btm);
//			}
//		}
//
//		vSubFolders.swap(vSubFoldersTmp);
//	}
//
//	//////////////////////////////////
//	size_t iProgress = 0;
//	for (size_t i = 0; i < vSubFoldersAllLevel.size(); i++)
//	{
//		char wzSubDirPath[MAX_PATH];
//		std::vector<wstring> vecPicFile;
//
//		if (vSubFoldersAllLevel[i].length() > 0)
//			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s\\%s", wzDir, vSubFoldersAllLevel[i].c_str());
//		else
//			StringCchPrintfW(wzSubDirPath, MAX_PATH, L"%s", wzDir);
//
//		GetImageFilesInDir(wzSubDirPath, vecPicFile);
//
//		for (size_t j = 0; j < vecPicFile.size(); j++)
//		{
//			char wzPicPath[MAX_PATH];
//			StringCchPrintfW(wzPicPath, MAX_PATH, L"%s\\%s", wzSubDirPath, vecPicFile[j].c_str());
//
//			wstring strFile(wzPicPath);
//			if (bTestReadable)
//			{
//				Gdiplus::Bitmap bmp(wzPicPath);
//				if (bmp.GetWidth() != 0)
//				{
//					vAllFiles.push_back(strFile);
//				}
//				else
//				{
//					wprintf(L"Read failed: %s\n", wzPicPath);
//				}
//			}
//			else
//			{
//				vAllFiles.push_back(strFile);
//			}
//		}
//
//		size_t iP = (i * 100) / vSubFoldersAllLevel.size();
//		if (iP > iProgress)
//		{
//			iProgress = iP;
//			//printf( "\b\b\b\b\b\b\b\b\b\b%d%", iProgress );
//		}
//	}
//
//	return S_OK;
//}
//
//StatusCode GetAllFilesFromListFile(
//	__in const char *wzFileList,
//	__in const char *wzRoot,
//	__out std::vector<wstring> &vAllFiles,
//	__in bool bTestReadable)
//{
//	FILE *fp = nullptr;
//	_wfopen_s(&fp, wzFileList, L"rt");
//	if (!fp)
//	{
//		return E_INVALIDARG;
//	}
//
//	char wzTmp[MAX_PATH];
//	while (fgetws(wzTmp, MAX_PATH, fp) != nullptr)
//	{
//		wstring strFile(wzTmp);
//		trim(strFile);
//
//		char wzFilePath[MAX_PATH];
//		if (nullptr != wzRoot)
//		{
//			StringCchPrintfW(wzFilePath, MAX_PATH, L"%s\\%s", wzRoot, strFile);
//		}
//		else
//		{
//			StringCchPrintfW(wzFilePath, MAX_PATH, L"%s", strFile);
//		}
//		wstring strFilePath(wzFilePath);
//		if (bTestReadable)
//		{
//			Gdiplus::Bitmap bmp(wzFilePath);
//			if (bmp.GetWidth() != 0)
//			{
//				vAllFiles.push_back(strFilePath);
//			}
//		}
//		else
//		{
//			vAllFiles.push_back(strFilePath);
//		}
//
//	}
//	fclose(fp);
//	return S_OK;
//}
//
//StatusCode DeleteAllFilesInFolder(char *wzPicDir)
//{
//	assert(nullptr != wzPicDir);
//	if (nullptr == wzPicDir) return E_INVALIDARG;
//	if (!PathFileExists(wzPicDir)) return E_INVALIDARG;
//
//	WIN32_FIND_DATA wfData;
//	char wzFilter[MAX_PATH];
//	char wzFile[MAX_PATH];
//
//	StringCchPrintfW(wzFilter, MAX_PATH, L"%s\\*.*", wzPicDir);
//	HANDLE hFind = ::FindFirstFileW(wzFilter, &wfData);
//
//	if (INVALID_HANDLE_VALUE != hFind) //we couldn't find any file
//	{
//		BOOL fNext = TRUE;
//		while (TRUE == fNext)
//		{
//			if (FILE_ATTRIBUTE_DIRECTORY ==
//				(wfData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//			{
//				fNext = ::FindNextFileW(hFind, &wfData);
//				continue;
//			}
//
//			StringCchPrintfW(wzFile, MAX_PATH, L"%s\\%s", wzPicDir, wfData.cFileName);
//			if (FALSE == ::DeleteFileW(wzFile))
//			{
//				wprintf(L"Cannot delete file %s\n", wfData.cFileName);
//			}
//
//			fNext = ::FindNextFileW(hFind, &wfData);
//
//		}
//		::FindClose(hFind);
//	}
//
//	return S_OK;
//}
//
//
//StatusCode GetFilename(const char* wzFilePath, std::string& filename)
//{
//	if (wzFilePath == nullptr) return StatusCode::PointerError;
//
//  path file_path(wzFilePath);
//  filename = path(wzFilePath).filename().string();
//	return StatusCode::OK;
//}
//
//bool GetFileNameAndExt(const string& filePath, string& fileName, string& ext){
//	int idx = filePath.find_last_of(".");
//	fileName = filePath.substr(0, idx);
//	ext = filePath.substr(idx + 1);
//	return true;
//}
//

