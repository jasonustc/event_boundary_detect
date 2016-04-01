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
#include <ctime>

int GetDirectoryName(const string& filePath, string& dirName, string& fileName);
bool MakeSubFolder(const string &parentFolder, const string& subFolder, string& fullFolderPath);
bool CopyFileToFolder(const string& filePath, const string& folderPath);
StatusCode GetSubFolders(const char *dir, std::vector<std::string> &sub_folders);
StatusCode GetTxtFilesInDir(const char * wzTxtDir, std::vector<string>& vecTxtFile);
void MakeDir(const string& dir);
bool IsInSameFolder(const string& file1, const string& file2);
bool FileExist(const string& filePath);
time_t LastWriteTime(const string &file_path);

StatusCode GetImageFilesInDir(const char * wzPicDir, std::vector<std::string>& vecPicFile);

StatusCode GetFilesInDirWithExt(const char * wzTxtDir, const string& ext, std::vector<string>& vecTxtFile);
//include .\ and all 1-level subfolders

//StatusCode GetAllLeafSubFolders_FullPath(char *wzDir, std::vector<std::string> &vSubFoldersLeaf);
//
//StatusCode GetAllImageFilesInSubfolders(
//    const char *wzDir,
//    std::vector<std::string> &vAllFiles,
//    bool bTestReadable = true);
//
//StatusCode GetAllFilesFromListFile(
//    const char *wzFileList,
//    const char *wzRoot,
//    std::vector<std::string> &vAllFiles,
//    bool bTestReadable = true);
//
//StatusCode DeleteAllFilesInFolder(char *wzDir);
//
//StatusCode GetFilename(const char* wzFilePath, std::string& filename);
//
//bool GetFileNameAndExt(const string& filePath, string& fileName, string& ext);
