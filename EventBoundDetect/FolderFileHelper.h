// 
// Copyright (c) Microsoft Corporation. All rights reserved. 
// 
// File: FolderFileHelper.h
// 
// Description: 
//     Defines helper functions to access the files inside a folder on disk.
// 
#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <io.h>
#include <direct.h>


HRESULT GetFilesInDirWithExt(const WCHAR * wzTxtDir, std::vector<wstring>& vecTxtFile, wstring& ext);
HRESULT GetTxtFilesInDir(const WCHAR * wzTxtDir, std::vector<wstring>& vecTxtFile);
//include .\ and all 1-level subfolders
HRESULT GetImageFilesInDir(const WCHAR * wzPicDir, std::vector<std::wstring>& vecPicFile);

HRESULT GetSubFolders(const WCHAR *wzDir, std::vector<std::wstring> &vSubFolders);

HRESULT GetAllLeafSubFolders_FullPath(WCHAR *wzDir, std::vector<std::wstring> &vSubFoldersLeaf);

HRESULT GetAllImageFilesInSubfolders(
	__in const WCHAR *wzDir,
	__out std::vector<std::wstring> &vAllFiles,
	__in bool bTestReadable = true);

HRESULT GetAllFilesFromListFile(
	__in const WCHAR *wzFileList,
	__in const WCHAR *wzRoot,
	__out std::vector<std::wstring> &vAllFiles,
	__in bool bTestReadable = true);

HRESULT DeleteAllFilesInFolder(WCHAR *wzDir);

HRESULT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

bool ChooseFolder(HWND hParent, const std::wstring& title, std::wstring& folder);

HRESULT GetFilename(const WCHAR* wzFilePath, std::wstring& filename);

bool CopyFileToFolder(const string& filePath, const string& folderPath);
bool MakeSubFolder(const string &parentFolder, const string& subFolder, string& fullFolderPath);
int GetDirectoryName(const string& filePath, string& dirName, string& fileName);
bool IsInSameFolder(const string& file1, const string& file2);
bool GetFileNameAndExt(const string& filePath, string& fileName, string& ext);
bool FileExist(const string& filePath);
