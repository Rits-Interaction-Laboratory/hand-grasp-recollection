#pragma once
#include <iostream>
#include <ctime>

class Date {
protected:
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	struct tm *date;
	time_t now;
public:
	Date();
	Date(int year, int month, int day,
				int hour, int minute, int second);
	int getYear();
	int getMonth();
	int getDay();
	int getHour();
	int getMinute();
	int getSecond();
protected:
	void setYear(int year);
	void setMonth(int month);
	void setDay(int day);
	void setHour(int hour);
	void setMinute(int minute);
	void setSecond(int second);
	void show();
};

class DateManager : Date{
private:

public:
	DateManager();

	void update();
	Date getCurrentDate();

};