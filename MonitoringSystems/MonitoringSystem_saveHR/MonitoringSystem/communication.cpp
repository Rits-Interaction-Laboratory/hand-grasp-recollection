#include "stdafx.h"
#include <conio.h>
#include <winsock2.h>
#include "communication.h"
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

int Communication::sendCommand(){
	SOCKET sock;

	WSADATA wsaData;
	WORD wversion = WINSOCK_VERSION;
	SOCKADDR_IN saddr;
	char szBuf[32768];//��M�f�[�^�i�[ 1024*32
//	char szTotalBuf[300000];               //��M�f�[�^���i�[
	int sizeRecvData;                              //��M�f�[�^�̃T�C�Y
//	int nTotalRcv;                              //��M�f�[�^�̃T�C�Y

	char szHost[20];       //������IP�A�h���X
	HOSTENT *ipHost;
	unsigned int addr;

	static unsigned long frame=0;   //�t���[����(�����ڂ̃t���[����)

	//�摜�̕��ƍ����̏�����(�Ƃ肠�����A�����E�����)
/*	Width = 640;
	Height = 480;

	//���[�h�̐ݒ�
	mode = ONLINE;
	input.set_mode(ONLINE);
	intpre.set_mode(ONLINE);
	intpre.set_pParent(pParent);
	*/
	//WinSock�̏�����
	if(WSAStartup(wversion,&wsaData)){
		WSACleanup();
		_getch();
		return -1;
	}

	//Winsocke�̃o�[�W�������`�F�b�N
	if(wversion != wsaData.wVersion){
		return -1;
	}

	//�\�P�b�g�I�[�v��
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		WSACleanup();
		_getch();
		return -2;
	}
	//DNS���O�����i�T�[�o�[��IP�A�h���X������o���j
	ipHost = gethostbyname(ipNetworkCamera.c_str());
	if(ipHost == NULL){
		addr = inet_addr(ipNetworkCamera.c_str());
		ipHost = gethostbyaddr((char *)&addr,4,AF_INET);
	}
	if(ipHost == NULL){
		closesocket(sock);
		WSACleanup();
		_getch();
		return -3;
	}

	memset(&saddr,0,sizeof(SOCKADDR_IN));
	saddr.sin_family = AF_INET; //�A�h���X�t�@�~���[���w��
	memcpy( &saddr.sin_addr, ipHost->h_addr, ipHost->h_length);//�ڑ���IP�A�h���X�w��i�����ł͖��O����)
	saddr.sin_port = htons(80); //�ڑ���|�[�g�ԍ��w��(�����ł̓|�[�g�ԍ�80�ŌŒ�)

	//������IP���ׂ�
	char ac[80];
	if(gethostname(ac,sizeof(ac))==SOCKET_ERROR)	return 1;

	struct hostent *phe = gethostbyname(ac);
	if(phe==0)	return 1;

	struct in_addr haddr;
	memcpy(&haddr,phe->h_addr_list[0],sizeof(struct in_addr));
	strcpy_s(szHost,inet_ntoa(haddr));

	int task=1;
	if(task==1){
		//�J�����̌������E�ɕς���
		sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0607");
	}else if(task==2){
		//�J�����̌��������ɕς���
		sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0407");
	}else if(task==3){
		//�J�����̌�������ɕς���
		sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0807");
	}else if(task==4){
		//�J�����̌��������ɕς���
		/*sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0207");*/
		sprintf_s(szBuf, "GET /command/ptzf.cgi?Move=up HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\n\r\n");
	}else if(task==5){
		//���摜���擾
		/////////////////////////////////////////////////
		// interval:��msec�Ԋu�ɉ摜�𑗂�
		// number  :�����摜�𑗂�
		//
		// speed   :30���ő�(25fps)
		////////////////////////////////////////////////

		//sprintf(szBuf,"GET /image?interval=40&number=200\r\nHost: "); 
		sprintf_s(szBuf,"GET /image?speed=20 HTTP/1.1\r\nHost: ");
		//sprintf_s(szBuf,"GET /oneshotimage.jpg HTTP/1.1\r\nHost: ");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\n\r\n");
	}

	//�ڑ�
	if(connect(sock,(SOCKADDR *)&saddr,sizeof(saddr))<0){
		closesocket(sock);
		WSACleanup();
		return -4;
	}

	//�f�[�^�𑗐M�i�v���j
	send(sock,szBuf,(int)strlen(szBuf),0);


	//�f�[�^����M(1��ڂ͉摜�f�[�^�ł͂Ȃ�)
	sizeRecvData = recv(sock,szBuf,sizeof(szBuf),0);


	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_DUPLEX, 0.4, 0.4);

	/*for( ; ( sizeRecvData = recv( sock, szBuf, sizeof(szBuf), 0)) > 0 ; ) {
		cout<<"recieve:"<<endl;
		do{
			//�m�ۂ����T�C�Y�𒴂����Ƃ�
			if(nTotalRcv +sizeRecvData > sizeof(szTotalBuf))				
				break;

			memcpy(&szTotalBuf[nTotalRcv], szBuf, sizeRecvData);	
			nTotalRcv += sizeRecvData;

			if(sizeRecvDat >= 4){
				if(cerama==1 || camera==2){
					if(((unsigned char)szBuf[nRcv-4] == 255) && ((unsigned char)szBuf[nRcv-3] == 217) &&
						szBuf[nRcv-2] == '\r' && szBuf[nRcv-1] == '\n'){									
							break;
					}
				}
				if(camera==3){
					if(((unsigned char)szBuf[nRcv-4] == '\r') && ((unsigned char)szBuf[nRcv-3] == '\n') &&
						szBuf[nRcv-2] == '\r' && szBuf[nRcv-1] == '\n'){									
							break;
					}
				}
			}
		}while ((nRcv = recv( s, szBuf, sizeof(szBuf), 0)) > 0);


	}
	*/

	return 1;
}

void Communication::changeIp(int n){
	//�w�肵���ԍ������X�g�͈̔͊O�������ꍇ
	if(n<0||(!ipNetworkCameraList.empty()&&n>=(int)ipNetworkCameraList.size())){
		cout<<__FUNCTION__<<" : wrong arg"<<endl;
		exit(0);
	}
	//���X�g�ɂȂɂ��o�^����Ă��Ȃ��ꍇ
	else if(ipNetworkCameraList.empty()){
		cout<<__FUNCTION__<<" : (warning) ipNetworkCameraList is empty"<<endl;
		ipNetworkCamera="";
	}
	else ipNetworkCamera=ipNetworkCameraList[n];
}
string Communication::getIpNetwork(){
	return ipNetworkCamera;
}
string Communication::getIpNetworkN(int n){
	if(n<0||n>=(int)ipNetworkCameraList.size()){
		cout<<__FUNCTION__<<" : wrong arg"<<endl;
		exit(0);
	}
	return ipNetworkCameraList[n];
}
vector<string> Communication::getIpNetworkList(){
	return ipNetworkCameraList;
}