#include "NetworkCamera.h"

namespace NetworkCamera{
	void getCommand(char *strBuf, int numCommand){
		int size=26000;
		switch(numCommand){
		case 1:
			//カメラの向きを右に変える
			sprintf_s(strBuf, size,"POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
			strcat_s(strBuf,size,"szHost");
			strcat_s(strBuf,size,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
			strcat_s(strBuf, size,"\r\nrelative=0607");
		case 2:
			//カメラの向きを左に変える
			sprintf_s(strBuf, size,"POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
			strcat_s(strBuf,size,"szHost");
			strcat_s(strBuf,size,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
			strcat_s(strBuf,size,"\r\nrelative=0407");
		case 3:
			sprintf_s(strBuf, size,"POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
			strcat_s(strBuf,size,"szHost");
			strcat_s(strBuf,size,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
			strcat_s(strBuf,size,"\r\nrelative=0807");
		case 4:
			//カメラの向きを下に変える
			/*sprintf_s(strBuf, "POST /command/ptzf.cgi HTTP/1.1\r\nHost:");
			strcat_s(strBuf,"szHost");
			strcat_s(strBuf,"\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\nContent-Length: 13\r\n");
			strcat_s(strBuf, "\r\nrelative=0207");*/
			sprintf_s(strBuf,size,"GET /command/ptzf.cgi?Move=up HTTP/1.1\r\nHost:");
			strcat_s(strBuf,size,"szHost");
			strcat_s(strBuf,size,"\r\n\r\n");
		case 5:
			//動画像を取得
			/////////////////////////////////////////////////
			// interval:○msec間隔に画像を送る
			// number  :○枚画像を送る
			//
			// speed   :30が最大(25fps)
			////////////////////////////////////////////////

			//sprintf(strBuf,"GET /image?interval=40&number=200\r\nHost: "); 
			sprintf_s(strBuf,size,"GET /image?speed=10 HTTP/1.1\r\nHost: ");
			//sprintf_s(strBuf,"GET /oneshotimage HTTP/1.1\r\nHost: ");
			//sprintf_s(strBuf,"GET /oneshotimage.jpg HTTP/1.1\r\nHost: ");
			strcat_s(strBuf,size,"szHost");
			strcat_s(strBuf,size,"\r\n\r\n");
		}
	}
}