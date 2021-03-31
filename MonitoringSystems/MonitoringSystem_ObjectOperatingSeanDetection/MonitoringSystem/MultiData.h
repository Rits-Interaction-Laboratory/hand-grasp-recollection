#pragma once
#include <iostream>
#include "msgpack.hpp"

class MultiData{
private:
	std::string data;
	std::string dataType;
	std::string title;
public:
	std::string layout;
	MultiData():data(""), dataType(""){
	}
	MultiData( std::string _data, 
					std::string _dataType, 
					std::string _title ):data(_data), dataType(_dataType), title(_title){
	}
	MSGPACK_DEFINE(data, dataType, layout, title);
};