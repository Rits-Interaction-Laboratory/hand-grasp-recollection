#include <iostream>
#include <vector>
using namespace std;
//�l�b�g���[�N�J������IP�\
//const string networkcamera1="133.19.22.240";//��P���@��Q����
const string networkcamera1="10.40.0.239";
const string networkcamera2="133.19.22.239";//��Q���@
const string networkcamera3="133.19.22.237";//��P���@�G���x�[�^��


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

		//�����ǉ�
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


