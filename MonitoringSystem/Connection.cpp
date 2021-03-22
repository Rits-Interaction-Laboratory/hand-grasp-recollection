#include "stdafx.h"
#include "Connectioni.h"
#include <fstream>
#include <vector>
#include "Util.h"
using namespace std;
Connection::Connection(){
	initialize();
	// initialize_recognition_server();
	// initialize_interface_system();
}


Connection::Connection( const int threads ){
}

void Connection::initialize(){
	cout << __FUNCTION__ << endl;
	try
	{
		zmq::context_t *conetext = new zmq::context_t( 10 );
		socket = new zmq::socket_t( *conetext, ZMQ_REQ );

		ifstream ifs( "configKinectServer.txt" );
		string line;
		string ipaddress;
		string port;
		while( getline( ifs, line ) )
		{
			vector< string > result = Util::split( line, ":" );
			if( result.size() != 2 )
			{
				cout << "file format error" << endl;				
			}
			ipaddress = result[ 0 ];
			port = result[ 1 ];
		}

		try
		{
			cout << "connect:" << ipaddress + ":" + port << endl;
			addr = "tcp://" + ipaddress + ":" + port;
			socket->connect( addr.c_str() );
		}
		catch ( const zmq::error_t &e )
		{
			cout << addr.c_str() << endl;
			cerr << __FUNCTION__ << ":" << e.what() << endl;
		}
	}
	catch( const zmq::error_t &e )
	{
		cerr << __FUNCTION__ << ":" << e.what() << endl;
	}
}

void Connection::start(){
	

}

void Connection::reconnect(){
	if ( !socket->connected() )
	{
		socket->connect( addr.c_str() ); 
	}
}
void Connection::send( const string message ){
	zmq::message_t message_send( message.size() );
	memcpy( message_send.data(), message.data(), message.size() );
	socket->send( message_send );
}
void Connection::recv( string &message ){
	zmq::message_t message_recv;
	socket->recv( &message_recv );
	message = std::string(static_cast<char*>(message_recv.data()), message_recv.size());;
}

void Connection::initialize_recognition_server(){
	cout << __FUNCTION__ << endl;
	try
	{
		zmq::context_t *conetext = new zmq::context_t(1);
		socket2 = new zmq::socket_t(*conetext, ZMQ_REQ);
		ifstream ifs("configKinectServer3.txt");
		string line;
		string ipaddress;
		string port;
		while (getline(ifs, line))
		{
			vector< string > result = Util::split(line, ":");
			if (result.size() != 2)
			{
				cout << "file format error" << endl;
			}
			ipaddress = result[0];
			port = result[1];
		}

		try
		{
			cout << "connect:" << ipaddress + ":" + port << endl;
			addr2 = "tcp://" + ipaddress + ":" + port;
			socket2->connect(addr2.c_str());
			// socket2->bind("tcp://*:7890");
		}
		catch (const zmq::error_t &e)
		{
			cout << "error" << endl;
			cout << addr2.c_str() << endl;
			cerr << __FUNCTION__ << ":" << e.what() << endl;
		}
	}
	catch (const zmq::error_t &e)
	{
		cout << "error2" << endl;
		cerr << __FUNCTION__ << ":" << e.what() << endl;
	}
}

void Connection::start_recognition_server(){


}

void Connection::send_recognition_server(const string imgStr){
	// cout << imgStr.length() << endl;
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, imgStr);
	zmq::message_t message_send(sbuf.size());
	// cout << sbuf.size() << endl;
	memcpy(message_send.data(), sbuf.data(), sbuf.size());
	socket2->send(message_send);
}
void Connection::recv_recognition_server(vector<map<string, string>> &objResult){
	zmq::message_t message_recv;
	socket2->recv(&message_recv);
	string strRequest = std::string(static_cast< char* >(message_recv.data()), message_recv.size());
	msgpack::unpacked msg;
	msgpack::unpack(&msg, reinterpret_cast< const char* >(strRequest.data()), strRequest.size());
	msg.get().convert(&objResult);
	// 受け取ったデータの確認
	/*
	for (int n = 0; n < request.size(); n++){
		cout << request[n]["class_name"] << ", " << request[n]["id"] << ", " << request[n]["x"] << ", " << request[n]["y"] << ", " << request[n]["width"] << ", " << request[n]["height"] << ", " << request[n]["datetime"] << endl;
	}
	*/
}

void Connection::initialize_interface_system(){
	cout << __FUNCTION__ << endl;
	try
	{
		zmq::context_t *conetext = new zmq::context_t(1);
		// socket2 = new zmq::socket_t(*conetext, ZMQ_PUB);
		// int size = 1;
		// socket2->setsockopt(ZMQ_SNDHWM, &size, sizeof(int));
		// socket2 = new zmq::socket_t(*conetext, ZMQ_XREP);
		socket3 = new zmq::socket_t(*conetext, ZMQ_REP);
		ifstream ifs("configKinectServer2.txt");
		string line;
		string ipaddress;
		string port;
		while (getline(ifs, line))
		{
			vector< string > result = Util::split(line, ":");
			if (result.size() != 2)
			{
				cout << "file format error" << endl;
			}
			ipaddress = result[0];
			port = result[1];
		}

		try
		{
			cout << "bind:" << ipaddress + ":" + port << endl;
			addr3 = "tcp://" + ipaddress + ":" + port;
			socket3->bind(addr3.c_str());
			// socket2->bind("tcp://*:7890");
		}
		catch (const zmq::error_t &e)
		{
			cout << "error" << endl;
			cout << addr3.c_str() << endl;
			cerr << __FUNCTION__ << ":" << e.what() << endl;
		}
	}
	catch (const zmq::error_t &e)
	{
		cout << "error2" << endl;
		cerr << __FUNCTION__ << ":" << e.what() << endl;
	}
}

void Connection::start_interface_system(){


}

void Connection::send_interface_system(const string imgStr, vector<map<string, string>> objResult, unsigned long frameID){
	SendData sendData;
	sendData.colorImage = imgStr;
	string FrameNum = Util::toString(frameID);
	sendData.frameID = atoi(FrameNum.c_str());
	sendData.recoResult = objResult;
	// cout << message.length() << endl;
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, sendData);
	zmq::message_t message_send(sbuf.size());
	// cout << sbuf.size() << endl;
	memcpy(message_send.data(), sbuf.data(), sbuf.size());
	socket3->send(message_send);
}
void Connection::send_interface_system2(const string imgStr, const string depthStr, vector<map<string, string>> objResult, unsigned long frameID){
	SendData2 sendData;
	sendData.colorImage = imgStr;
	sendData.depthImage = depthStr;
	string FrameNum = Util::toString(frameID);
	sendData.frameID = atoi(FrameNum.c_str());
	sendData.recoResult = objResult;
	// cout << message.length() << endl;
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, sendData);
	zmq::message_t message_send(sbuf.size());
	// cout << sbuf.size() << endl;
	memcpy(message_send.data(), sbuf.data(), sbuf.size());
	socket3->send(message_send);
}
void Connection::send_interface_system3(const string imgStr, const string depthStr, vector<map<string, string>> objResult, unsigned long frameID, vector<float> layVec){
	SendData3 sendData;
	sendData.colorImage = imgStr;
	sendData.depthImage = depthStr;
	sendData.lay = layVec;
	string FrameNum = Util::toString(frameID);
	sendData.frameID = atoi(FrameNum.c_str());
	sendData.recoResult = objResult;
	// cout << message.length() << endl;
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, sendData);
	zmq::message_t message_send(sbuf.size());
	// cout << sbuf.size() << endl;
	memcpy(message_send.data(), sbuf.data(), sbuf.size());
	socket3->send(message_send);
}
void Connection::recv_interface_system(){
	zmq::message_t message_recv;
	socket3->recv(&message_recv);
}