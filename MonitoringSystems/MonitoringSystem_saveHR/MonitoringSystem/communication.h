#include <iostream>
#include <vector>
using namespace std;
//ネットワークカメラのIP表
//const string networkcamera1="133.19.22.240";//第１研　第２件寄
const string networkcamera1="10.40.0.239";
const string networkcamera2="133.19.22.239";//第２研　
const string networkcamera3="133.19.22.237";//第１研　エレベータ寄


class Communication{
public:
	vector<string> ipNetworkCameraList;
	string ipNetworkCamera;
public:
	Communication(){
		ipNetworkCameraList=vector<string>();
		//ip
		ipNetworkCameraList.push_back(networkcamera1);
		ipNetworkCameraList.push_back(networkcamera2);
		ipNetworkCameraList.push_back(networkcamera3);

		//初期追加
		ipNetworkCamera=ipNetworkCameraList[max(ipNetworkCameraList.size()-1,0)];
	}
	~Communication(){

	}

	int sendCommand();
	void changeIp(int n);
	string getIpNetwork();
	string getIpNetworkN(int n);
	vector<string> getIpNetworkList();

};


