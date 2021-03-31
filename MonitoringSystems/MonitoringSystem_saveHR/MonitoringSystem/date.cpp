#include "date.h"

////////////////////////////////////
// Dateクラス
//
Date::Date() 
	: year(2013), month(1), day(1), hour(1), minute(1), second(1){
}

Date::Date(int year, int month, int day, int hour, int minute, int second) 
	: year(year), month(month), day(day), hour(hour), minute(minute), second(second){

}

int Date::getYear() {
	return year;
}
int Date::getMonth() {
	return month;
}
int Date::getDay() {
	return day;
}
int Date::getHour() {
	return hour;
}
int Date::getMinute() {
	return minute;
}
int Date::getSecond() {
	return second;
}
void Date::setYear( int year ) {
	this->year = year;
}
void Date::setMonth( int month ) {
	this->month = month;
}
void Date::setDay( int day ) {
	this->day = day;
}
void Date::setHour( int hour ) {
	this->hour = hour;
}
void Date::setMinute( int minute ) {
	this->minute = minute;
}
void Date::setSecond( int second ) {
	this->second = second;
}
void Date::show(){
	std::cout << "現在の時刻は:" << std::endl
		<< year << "/" << month << "/" << day << " " << hour << ":" << minute << ":" << second << std::endl; 
}


////////////////////////////////////
// DateMangerクラス
//
DateManager::DateManager() {

}
void DateManager::update() {
	time( &now );
	date = localtime( &now );
	year = date->tm_year + 1900;
	month = date->tm_mon + 1;
	day = date->tm_mday;
	hour = date->tm_hour;
	minute = date->tm_min;
	second = date->tm_sec;
}
Date DateManager::getCurrentDate(){
	update();
	return Date(year, month, day, hour, minute, second);
}


