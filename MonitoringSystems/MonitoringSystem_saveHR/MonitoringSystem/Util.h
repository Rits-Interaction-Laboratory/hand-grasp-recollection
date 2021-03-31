#pragma once
#ifndef __UTIL_H__
#define __UTIL_H__
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "bmp.h"
#include <vector>

std::size_t file_count_native(const std::string src);
namespace Util{
	cv::Mat convertBmpChar2Mat(bmp imgBmp);
	void convertMat2BmpChar(cv::Mat imgMat, bmp &imgBmp);
	void convertIplImage2BmpChar(IplImage *imgIpl, bmp &imgBmp);

	std::string toString(int n);
	void replaceAll(std::string &str,const std::string from, const std:: string to);
	std::vector<std::string> split(std::string s, std::string t);
	std::vector<std::string> splitFirst(std::string s, const std::string t) ;
	cv::Mat convertVec2Mat( const std::vector<unsigned char>& vec );
	std::string CString2String(CString s);
	cv::Mat convertString2Mat(std::string str);
	std::string convertMat2String(cv::Mat mat);
	std::string convertMat2String2(cv::Mat mat);
	char * string2Char(std::string s);
}
#endif