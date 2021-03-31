#pragma once
#ifndef __MONITORINGSYSTEM_TIME_H__
#define __MONITORINGSYSTEM_TIME_H__
#include <atltime.h>
#include <iostream>
#include "Util.h"
namespace Util{
	class Time{
	private:
		CTime time;
	public:
		Time(){
			time = CTime::GetCurrentTime();
		}
		Time(CTime time):time(time){
		}
		~Time(){

		}
		CTime getCurrentTime(){
			return CTime::GetCurrentTime();
		}
		CString format(std::string strFormat, CTime ctime){
			if(strFormat == "YmdHMS") return ctime.Format("%Y%m%d%H%M%S");
			else if(strFormat == "H") return ctime.Format("%H");
			else if(strFormat == "M") return ctime.Format("%M");
			return "";
		}
		std::string getCurrentTimeString(){
			return Util::CString2String(format("YmdHMS", getCurrentTime()));
		}
		std::string getCurrentTimeString(std::string f){
			if(f == "%H")return Util::CString2String(format("H", getCurrentTime()));
			else if(f == "%M")return Util::CString2String(format("M", getCurrentTime()));
			return "";
		}
	};
}

#endif