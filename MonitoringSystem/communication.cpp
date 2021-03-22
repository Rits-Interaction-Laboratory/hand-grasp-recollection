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
	char szBuf[32768];//受信データ格納 1024*32
//	char szTotalBuf[300000];               //受信データを格納
	int sizeRecvData;                              //受信データのサイズ
//	int nTotalRcv;                              //受信データのサイズ

	char szHost[20];       //自分のIPアドレス
	HOSTENT *ipHost;
	unsigned int addr;

	static unsigned long frame=0;   //フレーム数(何枚目のフレームか)

	//画像の幅と高さの初期化(とりあえず、高さ・幅一定)
/*	Width = 640;
	Height = 480;

	//モードの設定
	mode = ONLINE;
	input.set_mode(ONLINE);
	intpre.set_mode(ONLINE);
	intpre.set_pParent(pParent);
	*/
	//WinSockの初期化
	if(WSAStartup(wversion,&wsaData)){
		WSACleanup();
		_getch();
		return -1;
	}

	//Winsockeのバージョンをチェック
	if(wversion != wsaData.wVersion){
		return -1;
	}

	//ソケットオープン
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		WSACleanup();
		_getch();
		return -2;
	}
	//DNS名前解決（サーバーのIPアドレスを割り出す）
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
	saddr.sin_family = AF_INET; //アドレスファミリーを指定
	memcpy( &saddr.sin_addr, ipHost->h_addr, ipHost->h_length);//接続先IPアドレス指定（ここでは名前から)
	saddr.sin_port = htons(80); //接続先ポート番号指定(ここではポート番号80で固定)

	//自分のIP調べる
	char ac[80];
	if(gethostname(ac,sizeof(ac))==SOCKET_ERROR)	return 1;

	struct hostent *phe = gethostbyname(ac);
	if(phe==0)	return 1;

	struct in_addr haddr;
	memcpy(&haddr,phe->h_addr_list[0],sizeof(struct in_addr));
	strcpy_s(szHost,inet_ntoa(haddr));

	int task=1;
	if(task==1){
		//カメラの向きを右に変える
		sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0607");
	}else if(task==2){
		//カメラの向きを左に変える
		sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0407");
	}else if(task==3){
		//カメラの向きを上に変える
		sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0807");
	}else if(task==4){
		//カメラの向きを下に変える
		/*sprintf_s(szBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
		strcat_s(szBuf, "\r\nrelative=0207");*/
		sprintf_s(szBuf, "GET /command/ptzf.cgi?Move=up HTTP/1.1\r\nHost:");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\n\r\n");
	}else if(task==5){
		//動画像を取得
		/////////////////////////////////////////////////
		// interval:○msec間隔に画像を送る
		// number  :○枚画像を送る
		//
		// speed   :30が最大(25fps)
		////////////////////////////////////////////////

		//sprintf(szBuf,"GET /image?interval=40&number=200\r\nHost: "); 
		sprintf_s(szBuf,"GET /image?speed=20 HTTP/1.1\r\nHost: ");
		//sprintf_s(szBuf,"GET /oneshotimage.jpg HTTP/1.1\r\nHost: ");
		strcat_s(szBuf,"szHost");
		strcat_s(szBuf,"\r\n\r\n");
	}

	//接続
	if(connect(sock,(SOCKADDR *)&saddr,sizeof(saddr))<0){
		closesocket(sock);
		WSACleanup();
		return -4;
	}

	//データを送信（要求）
	send(sock,szBuf,(int)strlen(szBuf),0);


	//データを受信(1回目は画像データではない)
	sizeRecvData = recv(sock,szBuf,sizeof(szBuf),0);


	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_DUPLEX, 0.4, 0.4);

	/*for( ; ( sizeRecvData = recv( sock, szBuf, sizeof(szBuf), 0)) > 0 ; ) {
		cout<<"recieve:"<<endl;
		do{
			//確保したサイズを超えたとき
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
	//指定した番号がリストの範囲外だった場合
	if(n<0||(!ipNetworkCameraList.empty()&&n>=(int)ipNetworkCameraList.size())){
		cout<<__FUNCTION__<<" : wrong arg"<<endl;
		exit(0);
	}
	//リストになにも登録されていない場合
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