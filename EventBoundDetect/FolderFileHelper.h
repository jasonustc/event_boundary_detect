// 
// Copyright (c) Microsoft Corporation. All rights reserved. 
// 
// File: FolderFileHelper.h
// 
// Description: 
//     Defines helper functions to access the files inside a folder on disk.
// 
#pragma once

#include <vector>
#include <string>

StatusCode GetSubFolders(const char *dir, std::vector<std::string> &sub_folders);
StatusCode GetImageFilesInDir(const char * wzPicDir, std::vector<std::string>& vecPicFile);

StatusCode GetFilesInDirWithExt(const char * wzTxtDir, std::vector<string>& vecTxtFile, string& ext);
StatusCode GetTxtFilesInDir(const char * wzTxtDir, std::vector<string>& vecTxtFile);
//include .\ and all 1-level subfolders


StatusCode GetAllLeafSubFolders_FullPath(char *wzDir, std::vector<std::string> &vSubFoldersLeaf);

StatusCode GetAllImageFilesInSubfolders(
	__in const char *wzDir,
	__out std::vector<std::string> &vAllFiles,
	__in bool bTestReadable = true);

StatusCode GetAllFilesFromListFile(
	__in const char *wzFileList,
	__in const char *wzRoot,
	__out std::vector<std::string> &vAllFiles,
	__in bool bTestReadable = true);

StatusCode DeleteAllFilesInFolder(char *wzDir);

StatusCode GetFilename(const char* wzFilePath, std::string& filename);

bool CopyFileToFolder(const string& filePath, const string& folderPath);
bool MakeSubFolder(const string &parentFolder, const string& subFolder, string& fullFolderPath);
int GetDirectoryName(const string& filePath, string& dirName, string& fileName);
bool IsInSameFolder(const string& file1, const string& file2);
bool GetFileNameAndExt(const string& filePath, string& fileName, string& ext);
bool FileExist(const string& filePath);
