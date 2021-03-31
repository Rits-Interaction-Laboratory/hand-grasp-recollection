#pragma once
#include <iostream>
#include <vector>
#include "Util.h"
#include "zmq.hpp"
#include "msgpack.hpp"
class SendData{
public:
	string colorImage;
	int frameID;
	vector<map<string, string>> recoResult;
	MSGPACK_DEFINE(
		colorImage,
		frameID,
		recoResult
	);
};
class SendData2{
public:
	string colorImage;
	string depthImage;
	int frameID;
	vector<map<string, string>> recoResult;
	MSGPACK_DEFINE(
		colorImage,
		depthImage,
		frameID,
		recoResult
		);
};
class SendData3{
public:
	string colorImage;
	string depthImage;
	vector<float> lay;
	int frameID;
	vector<map<string, string>> recoResult;
	MSGPACK_DEFINE(
		colorImage,
		depthImage,
		lay,
		frameID,
		recoResult
		);
};

class Connection{
private:
	int m_thread;
	std::string addr;
	std::string addr2;
	std::string addr3;
public:
	Connection();
	Connection( const int threads );
	zmq::socket_t *socket;
	zmq::socket_t *socket2;
	zmq::socket_t *socket3;

	void start();
	void reconnect();
	void send( const std::string message );
	void recv(std::string &messsage);
	void start_recognition_server();
	void send_recognition_server(const std::string imgStr);
	void recv_recognition_server(vector<map<string, string>> &objResult);
	void start_interface_system();
	void send_interface_system(const string imgStr, vector<map<string, string>> objResult, unsigned long frameID);
	void send_interface_system2(const string imgStr, const string depthStr, vector<map<string, string>> objResult, unsigned long frameID);
	void send_interface_system3(const string imgStr, const string depthStr, vector<map<string, string>> objResult, unsigned long frameID, vector<float> layVec);
	void recv_interface_system();
	int getNumThreads();

private:
	void initialize();
	void initialize_recognition_server();
	void initialize_interface_system();
};