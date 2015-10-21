// 
// Copyright (c) Microsoft Corporation. All rights reserved. 
// 
// File: StringHelper.h
// 
// Description: 
//     Defines helper class for strings used in FaceSDK demo.
// 
#pragma once

#include <algorithm> 
#include <functional>
#include <cctype>
#include <locale>


template<typename T>
static inline std::basic_string<T>& trim_left(std::basic_string<T> &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

template<typename T>
static inline std::basic_string<T>& trim_right(std::basic_string<T> &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

template<typename T>
static inline std::basic_string<T>& trim(std::basic_string<T> &s) {
	return trim_left(trim_right(s));
}
