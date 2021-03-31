//------------------------------------------------------------------------
// FileName : network.cpp
// Context  : ネットワークカメラにWinSockを使って接続し、カメラを制御
//
// Author   : Kazuhiro MAKI (CV-lab.)
// Updata   : 2006.07.26
//------------------------------------------------------------------------
#pragma execution_character_set("utf-8")
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <vector>
#include "network.h"

#include "Labeling.h"
#include "jpeglib.h"
#include <time.h>

#include "Util.h"
#include "KinectPack.h"
#include <zmq.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
#include "Connectioni.h"
#include <opencv2\opencv.hpp>
#include "MessagePackUtility.hpp"
#include "KinectPackUtil.hpp"
//#include <boost/thread.hpp>


#include<mmsystem.h>
//#include "file.h"
#include <math.h>
#include <direct.h>
#include <fstream>
#include <string>
#include <shlwapi.h>
#include <windows.h>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include "opencv/cv.h"
#include "opencv/highgui.h"
//日付クラス
#include <ctime>
#include "Util.h"
#include "NetworkCamera.h"
//common
#include "common.h"

//MySQL
#include <mysql/mysql.h>
#include <atlstr.h>
#include <string.h>

//時間計測
#pragma comment(lib, "winmm.lib")

//templete
//DWORD start = timeGetTime();  // 開始時間
//計測処理
//DWORD end = timeGetTime();    // 終了時間
//std::cout << "duration = " << (double)(end - start) / 1000 << "sec." << endl;

//#include "Kinectsensor.h"

#include <zmq.hpp>
#include <msgpack.hpp>
#include <atltime.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define debug(x) cout << #x << " = " << (x) << " (L" << __LINE__ << ")" << endl;
#define BB(IMG, X,Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)*3]
#define GG(IMG, X,Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)*3+1]
#define RR(IMG, X,Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)*3+2]
#define I(IMG, X, Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)]


//Cinterpretation *p_event;//kawa
//Cinterpretation d_event;

//MYSQL用変数-----------------------------------------------------
MYSQL *mysql;
long long int mysqlSize;
int sceneID = 0;
int sceneFrame = 0;
int HumanCount = 0;
int humanIN = 0;
int humanOUT = 0;
int humanONnum = 0;
bool DBbool;
char ColorImgStr[1000 * 1024], DepthImgStr[1000 * 1024];
/*char TouchObjectImgStr[1000 * 1024], TouchHumanImgStr[1000 * 1024];
char BringObjectImgStr[1000 * 1024], BringHumanImgStr[1000 * 1024];
char TakeObjectImgStr[1000 * 1024], TakeHumanImgStr[1000 * 1024];
char HaveObjectImgStr[1000 * 1024], HaveHumanImgStr[1000 * 1024];
char DrinkObjectImgStr[1000 * 1024], DrinkHumanImgStr[1000 * 1024];
char HideObjectImgStr[1000 * 1024], HideHumanImgStr[1000 * 1024];
char MoveObjectImgStr[1000 * 1024], MoveHumanImgStr[1000 * 1024];*/
char query[1000 * 50000];
//----------------------------------------------------------------

//↓設定群をconfigテキストから自動読み込み設定するかどうか
bool autoExecute = false;

//解像度変更用変数------------------------------------------------2018/12/14
int resoInput;
int resoMagnification;
int resoX;
int resoY;
int resoInputMS;
int resoX_MS;
int resoY_MS;
//----------------------------------------------------------------

//画像受け取りモード変更用変数------------------------------------2018/12/14
int receiveMode;
int recvColor, recvDepth, recvBodyIndex, recvBodyInfo, recvSTime, recvCReactiveTime, recvDReactiveTime;
//----------------------------------------------------------------

//DB名受け取り用変数
string DBname;

bmp d_input;
bmp d_det;
unsigned long d_frame;
using namespace std;

//mutex
HANDLE saveMutex;
HANDLE stateMutex;
HANDLE rsvMutex;
HANDLE managerMutex;

struct ImgSet{
	cv::Mat color;
	cv::Mat depth;
	cv::Mat bodyindex;
};

// 画像保存用グローバル変数群-------------------------------------2019/1/11
//vector<std::pair<long long, std::pair<cv::Mat, cv::Mat>>> memImg;	// Mat image をメモリに蓄積するための配列
vector<std::pair<long long, ImgSet>> memImg;						// Mat image をメモリに蓄積するための配列(3点セット)
int num_memImgs;													// ↑に画像が何枚入っているかをカウントする
vector<std::pair<long long, pair<HANDLE, bool>>> threadState;		// 各スレッドは何フレーム目からを処理するか、その担当スレッドハンドル、スレッド終了判定
vector<std::pair<int, pair<long long, long long>>> saveRsv;			// イベントID、保存開始フレーム、保存終了フレーム
vector<std::pair<int, pair<int, int>>> monitorEvFin;				// イベントIDごとにスレッド数と終了数を監視
char filePath[256];													// "./img/[日時]/touch"

struct ev_manager{					// 以前の複雑な画像保存管理用配列を1つに集約した構造体
	int eventID;					//	イベントID
	long long start_frame;			//	保存最初フレーム(初期値で固定)
	long long end_frame;			//	保存最後フレーム(毎フレーム変化する)
	bool end_updateRange;			//	最後フレームの更新がストップしたかどうか
	bool fin_save;					//	イベントID:Xの保存がすべて完了したかどうか
};
vector<ev_manager> save_events;		// ↑構造体の配列
//----------------------------------------------------------------

//画像保存スレッド関数(改良版)------------------------------------2019/07/09
DWORD WINAPI imageSaver2( LPVOID arg ){
	long long beginFrame = *(long long*)arg;
	long long mem_at;
	long long last_savedFrame;

	char dirPath[256];
	char colorDirPath[256];
	char depthDirPath[256];
	char bodyindexDirPath[256];
	char colorFilePath[256];
	char depthFilePath[256];
	char bodyindexFilePath[256];
	char finTextFileName[256];

	vector<int> encodeParam = vector<int>(2);
	encodeParam[0] = CV_IMWRITE_PXM_BINARY;
	encodeParam[1] = 1;

	//WINAPIのスレッドで引数を一つしか渡せないため、保存開始フレーム番号(beginFrame)だけ渡して、各行列から目標データを検索する必要があった
	//イベント管理用構造体配列とスレッド自身を紐付け、内容をコピー(初期範囲指定)
	pair<long long, long long> saveRange;
	int eventID;

	WaitForSingleObject( managerMutex, INFINITE );
	for( auto i = save_events.begin(); i != save_events.end(); i++ ){	//save_events(イベント管理用構造体配列)を探索
		if( (*i).start_frame == beginFrame ){							//スレッド生成時に渡されたbeginFrameと一致するsave_events.start_frameの値をもつ配列要素をつかむ
			saveRange = make_pair( (*i).start_frame, (*i).end_frame );	//save_events.start_frame, save_events.end_frame を保存範囲としてコピー
			eventID = (*i).eventID;										//イベントIDも合わせてコピー
			//cout << eventID << "(eventID)" << endl;
			//cout << "saveRange[ " << saveRange.first << ", " << saveRange.second << " ](Ev." << eventID << ")" << endl;

			//接触イベントIDを元に保存先の指定
			sprintf_s( dirPath, "%s/%d", filePath, eventID );					//2019/05/23現在は /img/yyyymmdd_hhmmss/touch/の下に
			CreateDirectory( dirPath, NULL );									//接触イベントごとに0, 1, 2, ...(eventID)という名前のフォルダが生成され、その下に、
			sprintf_s( colorDirPath, "%s/%d/color", filePath, eventID );		//colorフォルダと
			sprintf_s( depthDirPath, "%s/%d/depth", filePath, eventID );		//depthフォルダが
			sprintf_s( bodyindexDirPath, "%s/%d/bodyindex", filePath, eventID );//bodyindexフォルダが
			CreateDirectory( colorDirPath, NULL );								//生成される
			CreateDirectory( depthDirPath, NULL );
			CreateDirectory( bodyindexDirPath, NULL );
			break;
		}
	}
	ReleaseMutex( managerMutex );

	//画像保存
	//開始フレームから11枚分を初めに保存
	if( saveRange.first == (long long)0 ){
		for( long long i = 0; i < saveRange.second - saveRange.first + 1; i++ ){
			WaitForSingleObject(saveMutex, INFINITE);

			//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
			mem_at = saveRange.first + i - memImg[0].first;											// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
			//cout << mem_at << "(mem_at)" << endl;
																									// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
			sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
			sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
			sprintf_s( bodyindexFilePath, "%s/%08lu.pgm", bodyindexDirPath, memImg[mem_at].first );
			cv::imwrite( string( colorFilePath ), memImg[mem_at].second.color );
			cv::imwrite( string( depthFilePath ), memImg[mem_at].second.depth );
			cv::imwrite( string( bodyindexFilePath ), memImg[mem_at].second.bodyindex, encodeParam );
			last_savedFrame = memImg[mem_at].first;
			//cout << "Last Saved: " << last_savedFrame << "(Ev." << eventID << ")" << endl;

			ReleaseMutex( saveMutex );
		}
	}
	else{
		for( long long i = 0; i < 11; i++ ){
			WaitForSingleObject(saveMutex, INFINITE);

			//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
			mem_at = saveRange.first + i - memImg[0].first;											// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
			//cout << mem_at << "(mem_at)" << endl;
																									// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
			sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
			sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
			sprintf_s( bodyindexFilePath, "%s/%08lu.pgm", bodyindexDirPath, memImg[mem_at].first );
			cv::imwrite( string( colorFilePath ), memImg[mem_at].second.color );
			cv::imwrite( string( depthFilePath ), memImg[mem_at].second.depth );
			cv::imwrite( string( bodyindexFilePath ), memImg[mem_at].second.bodyindex, encodeParam );
			last_savedFrame = memImg[mem_at].first;
			//cout << "Last Saved: " << last_savedFrame << "(Ev." << eventID << ")" << endl;

			ReleaseMutex( saveMutex );
		}
	}

	//ここからメインスレッドで画像が貯まるたびに追加保存を行う待機ループ
	bool save_complete = false;
	long long currentFrame;
	ev_manager myEvent;

	while( !save_complete )
	{	
		//現時点のイベント管理配列の情報を取得
		WaitForSingleObject( managerMutex, INFINITE );

		for( auto i = save_events.begin(); i != save_events.end(); i++ )
		{
			if( (*i).eventID == eventID )
			{
				myEvent.eventID = (*i).eventID;
				myEvent.start_frame = (*i).start_frame;
				myEvent.end_frame = (*i).end_frame;
				myEvent.end_updateRange = (*i).end_updateRange;
				myEvent.fin_save = (*i).fin_save;

				//cout << "Range Update?   : " << myEvent.end_updateRange << "(Ev. " << eventID << ")" << endl;
				//cout << "Last Saved(+11) : " << last_savedFrame << "(Ev. " << eventID << ")" << endl;
				//cout << "Final Frame(+11): " << myEvent.end_frame << "(Ev. " << eventID << ")" << endl;

				//イベントの要保存範囲の更新が完全終了、かつ、最後フレームまで保存済みならスレッド終了
				if( myEvent.end_updateRange && last_savedFrame == myEvent.end_frame )
				{
					//スレッドの終了
					cout << "Save Completed.(Ev. " << eventID << ")" << endl;
					(*i).fin_save = true;
					save_complete = true;
					sprintf( finTextFileName, "%s/fin_save.txt", dirPath );
					cout << "Generated: " << finTextFileName << endl;
					FILE *fp;
					fp = fopen( finTextFileName, "w" );
					fclose( fp );
				}
				break;
			}
		}

		ReleaseMutex( managerMutex );

		//現時点のフレーム番号を取得
		WaitForSingleObject( saveMutex, INFINITE );
		currentFrame = ( *( memImg.end() - 1 ) ).first;
		//cout << "Current Frame: " << currentFrame << endl;
		ReleaseMutex( saveMutex );

		if( last_savedFrame < myEvent.end_frame )
		{
			if( last_savedFrame != currentFrame )
			{
				if( currentFrame >= myEvent.end_frame )
				{
					for( long long i = last_savedFrame + (long long)1; i <= myEvent.end_frame; i++ ){
						WaitForSingleObject(saveMutex, INFINITE);

						//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
						mem_at = i - memImg[0].first;															// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
						//cout << "mem_at(11+): " << mem_at << endl;
																												// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
						sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
						sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
						sprintf_s( bodyindexFilePath, "%s/%08lu.pgm", bodyindexDirPath, memImg[mem_at].first );
						cv::imwrite( string( colorFilePath ), memImg[mem_at].second.color );
						cv::imwrite( string( depthFilePath ), memImg[mem_at].second.depth );
						cv::imwrite( string( bodyindexFilePath ), memImg[mem_at].second.bodyindex, encodeParam );
						last_savedFrame = memImg[mem_at].first;
						//cout << "last_savedFrame(11+): " << last_savedFrame << "(Ev." << eventID << ")" << endl;

						ReleaseMutex( saveMutex );
					}
				}
				else if( currentFrame < myEvent.end_frame )
				{
					for( long long i = last_savedFrame + (long long)1; i <= currentFrame; i++ ){
						WaitForSingleObject(saveMutex, INFINITE);

						//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
						mem_at = i - memImg[0].first;															// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
						//cout << i << "(last_savedFrame + 1)" << endl;
						//cout << memImg[0].first << "(memImg[0].first)" << endl;
						//cout << "mem_at(11+): " << mem_at << " ([last_savedFrame + 1] - [memImg[0].first])" << endl;
																												// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
						sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
						sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
						sprintf_s( bodyindexFilePath, "%s/%08lu.pgm", bodyindexDirPath, memImg[mem_at].first );
						cv::imwrite( string( colorFilePath ), memImg[mem_at].second.color );
						cv::imwrite( string( depthFilePath ), memImg[mem_at].second.depth );
						cv::imwrite( string( bodyindexFilePath ), memImg[mem_at].second.bodyindex, encodeParam );
						last_savedFrame = memImg[mem_at].first;
						//cout << "last_savedFrame(11+): " << last_savedFrame << "(Ev." << eventID << ")" << endl;

						ReleaseMutex( saveMutex );
					}
				}
			}
		}
	}

	//while( !save_complete ){
	//	//イベントの保存範囲の更新が終了,かつ最後フレームまで保存済みならループを抜ける
	//
	//	//現時点でのイベント管理配列の情報をつかむ
	//	WaitForSingleObject( managerMutex, INFINITE );
	//	
	//	for( auto i = save_events.begin(); i != save_events.end(); i++ ){									//save_events(イベント管理用構造体配列)を探索
	//		if( (*i).eventID == eventID ){																	//このスレッドID(eventID)と一致するsave_events.eventIDの値をもつ配列要素をつかむ
	//			cout << "(*i).end_updateRange: " << (*i).end_updateRange << "(Ev." << eventID << ")" << endl;
	//			cout << "last_savedFrame: " << last_savedFrame << "(Ev." << eventID << ")" << endl;
	//			cout << "(*i).end_frame: " << (*i).end_frame << "(Ev." << eventID << ")" << endl;
	//			if( (*i).end_updateRange && last_savedFrame == (*i).end_frame ){							//もし、保存範囲更新が終了、かつ、最後フレームまで保存済みの場合
	//				cout << " save completed: event " << eventID << endl;
	//				(*i).fin_save = true;																	//イベント管理構造体のイベント保存終了判定(fin_save)をtrueに
	//				save_complete = true;																	//このループ全体を抜ける
	//				sprintf( finTextFileName, "%s/fin_save.txt", dirPath );
	//				cout << finTextFileName << endl;
	//				FILE *fp;
	//				fp = fopen( finTextFileName, "w" );
	//				fclose( fp );
	//				break;
	//			}
	//			else{																						//まだ保存が終了していない場合は
	//				saveRange.second = (*i).end_frame;														//最後フレーム(save_events.end_frame)をチェックして範囲更新
	//				cout << "saveRange.second(updated): " << saveRange.second << "(Ev." << eventID << ")" << endl;
	//				break;
	//			}
	//		}
	//	}
	//
	//	ReleaseMutex( managerMutex );
	//
	//	if( save_complete ){
	//		cout << "save completed" << endl;
	//		break;		//ループ抜け判定
	//	}
	//
	//	//追加保存
	//	WaitForSingleObject( saveMutex, INFINITE );
	//	currentFrame = ( *( memImg.end() - 1 ) ).first;
	//	cout << "currentFrame: " << currentFrame << endl;
	//	ReleaseMutex( saveMutex );
	//
	//	if( currentFrame < saveRange.second ){
	//		if( saveRange.second > last_savedFrame ){
	//			for( long long i = last_savedFrame + (long long)1; i <= currentFrame; i++ ){
	//				WaitForSingleObject(saveMutex, INFINITE);
	//
	//				//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
	//				mem_at = i - memImg[0].first;															// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
	//				cout << i << "(last_savedFrame + 1)" << endl;
	//				cout << memImg[0].first << "(memImg[0].first)" << endl;
	//				cout << "mem_at(11+): " << mem_at << " ([last_savedFrame + 1] - [memImg[0].first])" << endl;
	//																										// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
	//				sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
	//				sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
	//				cv::imwrite( string( colorFilePath ), memImg[mem_at].second.first );
	//				cv::imwrite( string( depthFilePath ), memImg[mem_at].second.second );
	//				last_savedFrame = memImg[mem_at].first;
	//				cout << "last_savedFrame(11+): " << last_savedFrame << "(Ev." << eventID << ")" << endl;
	//
	//				ReleaseMutex( saveMutex );
	//			}
	//		}
	//	}
	//	else{
	//		if( saveRange.second > last_savedFrame ){
	//			for( long long i = last_savedFrame + (long long)1; i <= saveRange.second; i++ ){
	//				WaitForSingleObject(saveMutex, INFINITE);
	//
	//				//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
	//				mem_at = i - memImg[0].first;															// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
	//				//cout << "mem_at(11+): " << mem_at << endl;
	//																										// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
	//				sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
	//				sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
	//				cv::imwrite( string( colorFilePath ), memImg[mem_at].second.first );
	//				cv::imwrite( string( depthFilePath ), memImg[mem_at].second.second );
	//				last_savedFrame = memImg[mem_at].first;
	//				cout << "last_savedFrame(11+): " << last_savedFrame << "(Ev." << eventID << ")" << endl;
	//
	//				ReleaseMutex( saveMutex );
	//			}
	//		}
	//	}
	//
	//	if( saveRange.second > last_savedFrame && currentFrame > last_savedFrame ){						// 現在フレームより保存最後フレームが大きい、かつ、保存済みフレームより、現在フレームのほうが大きい
	//		for( long long i = last_savedFrame + (long long)1; i < currentFrame; i++ ){
	//			WaitForSingleObject(saveMutex, INFINITE);
	//
	//			//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
	//			mem_at = i - memImg[0].first;															// 開始フレームからi枚目のフレーム番号(saveRange.first + i)と
	//			//cout << "mem_at(11+): " << mem_at << endl;
	//																									// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが
	//			sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
	//			sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※必ず連番でフレーム画像がメモリ保持される前提)
	//			cv::imwrite( string( colorFilePath ), memImg[mem_at].second.first );
	//			cv::imwrite( string( depthFilePath ), memImg[mem_at].second.second );
	//			last_savedFrame = memImg[mem_at].first;
	//			cout << "last_savedFrame(11+): " << last_savedFrame << "(Ev." << eventID << ")" << endl;
	//
	//			ReleaseMutex( saveMutex );
	//		}
	//	}
	//
	//}
	return 0;
}

////画像保存スレッド関数--------------------------------------------2019/1/11
//DWORD WINAPI imageSaver( LPVOID arg ){ //型はpair<int, pair<long long, long long>>
////	pair<char*, pair<long long, long long>> pRsv;
////	pRsv.first = (*(pair<char*, pair<long long, long long>>*)arg).first;
////	pRsv.second.first = (*(pair<char*, pair<long long, long long>>*)arg).second.first;
////	pRsv.second.second = (*(pair<char*, pair<long long, long long>>*)arg).second.second;
//
//	long long beginFrame = *(long long*)arg;
//	long long mem_at;
//
//	//WINAPIのスレッドで引数を一つしか渡せないため、保存開始フレーム番号(beginFrame)だけ渡して、各行列から目標データを検索する必要があった
//	//保存予約行列の中から自身を生成した予約を探索し、内容をコピー
//	pair<long long, long long> pRsv;
//	int eventID;
//	WaitForSingleObject(rsvMutex, INFINITE);
//	for( auto i = saveRsv.begin(); i != saveRsv.end(); i++ ){			//saveRsv(保存予約行列)を探索
//		if( (*i).second.first == beginFrame ){							//saveRsv[ first, second[ first, second ] ] = [ イベントID, [ 保存開始フレーム, 保存終了フレーム ] ]
//			pRsv = make_pair( (*i).second.first, (*i).second.second );	//beginFrameと一致する、保存開始フレームを持つ保存予約を検索して、
//			eventID = (*i).first;										//イベントIDと保存終了フレームも合わせてコピー
//			break;
//		}
//	}
//	ReleaseMutex(rsvMutex);
//
//	//接触イベントIDを元に保存先の指定
//	char dirPath[256];
//	char colorDirPath[256];
//	char depthDirPath[256];
//	char colorFilePath[256];
//	char depthFilePath[256];
//	sprintf_s( dirPath, "%s/%d", filePath, eventID );							//2019/05/23現在は /img/yyyymmdd_hhmmss/touch/の下に
//	CreateDirectory( dirPath, NULL );											//接触イベントごとに0, 1, 2, ...(eventID)という名前のフォルダが生成され、その下に、
//	sprintf_s( colorDirPath, "%s/%d/color", filePath, eventID );				//colorフォルダと
//	sprintf_s( depthDirPath, "%s/%d/depth", filePath, eventID );				//depthフォルダが
//	CreateDirectory( colorDirPath, NULL );										//生成される
//	CreateDirectory( depthDirPath, NULL );
//
//	//画像保存ループ
//	for( long long i = 0; i < pRsv.second - pRsv.first + 1; i++ ){		//pRsv(コピーした保存予約)から保存枚数を計算してその回数分ループ
//		//lock
//		WaitForSingleObject(saveMutex, INFINITE);
//
//		//memImgの先頭のフレーム番号を確認して保存したいフレームまでの枚数差分値を計算			// memImg[ first, second ] = [ フレーム番号, 画像データ ](フレーム番号は必ず連番のはず)
//		mem_at = pRsv.first + i - memImg[0].first;												// 保存開始フレームからi枚目のフレーム番号(pRsv.first + i)と
//																								// 一時保存メモリ行列の先頭のフレーム番号(memImg[0].first)の差値mem_atが																					
//		sprintf_s( colorFilePath, "%s/%08lu.png", colorDirPath, memImg[mem_at].first );			// memImg[mem_at]としてindexの役割をはたす
//		sprintf_s( depthFilePath, "%s/%08lu.png", depthDirPath, memImg[mem_at].first );			// (※メモリには連番でフレーム画像が登録されることを想定して、探索時間をカットする意図)
//		cout << "colorDir: " << colorDirPath << endl;
//		cout << "depthDir: " << depthDirPath << endl;
//		cout << "colorFile: " << colorFilePath << endl;
//		cout << "depthFile: " << depthFilePath << endl;
//		cv::imwrite( string( colorFilePath ), memImg[mem_at].second.first );
//		cv::imwrite( string( depthFilePath ), memImg[mem_at].second.second );
//		//release
//		ReleaseMutex(saveMutex);
//	}
//	//保存終了処理
//	//スレッド終了をメインに通知するためスレッド行列の自スレッドの終了判定をON
//	//lockして、自分が担当する開始フレームの配列を探索
//	bool tEnd = false;
//	while(!tEnd){
//		WaitForSingleObject(stateMutex, INFINITE);
//
//		if( !threadState.empty() ){
//			for( auto itr = threadState.begin(); itr != threadState.end(); itr++ ){
//				if( pRsv.first == (*itr).first ){
//					(*itr).second.second = true;											// 「このスレッドは終わった」と通知
//					for( auto i = monitorEvFin.begin(); i != monitorEvFin.end(); i++ ){		// スレッド生起数・保存終了数監視配列の一致するイベントIDを探索
//						if( eventID == (*i).first ){
//							(*i).second.second++;		//該当IDの保存終了数を加算
//							cout << "event" << (*i).first << "save finished(" << (*i).second.second << "/" << (*i).second.first << ")" << endl;
//							if( (*i).second.second > (*i).second.first ){
//								cout << "the [number of finish] exceed the [number of occur] in event" << (*i).first << endl;
//							}
//							break;
//						}
//					}
//					ReleaseMutex(stateMutex);
//					tEnd = true;
//					break;
//				}
//			}
//		}
//		else{
//			//release
//			ReleaseMutex(stateMutex);
//		}
//	}
//	return 0;
//}
//----------------------------------------------------------------

//スレッド(人検知画像保存)
static UINT AFX_CDECL ThreadInterpretation( LPVOID pParam )
{
	//p_event=(Cinterpretation*)pParam;
	//p_event->increment(d_input,d_frame,d_det);//kawa
	//d_input.release();
	return 0;
}

using namespace std;



int parseDataLen( char *data, int *startIdx )
{
	int res = 0;				//返り値:画像データサイズ
	for ( int i = 0; data[i] != '\0'; i++ )
	{
		//		cout<<"data["<<i<<"]:"<<data[i]<<endl;
		if ( data[i] == '\r'&&data[i + 1] == '\n'&&data[i + 2] == '\0'&&data[i + 3] == '\0' )
		{
			*startIdx = i + 4;
			return res;
		}
		if ( data[i] == 'D'&&data[i + 1] == 'a' )
		{
			if ( !strncmp( data + i, "DataLen:", 8 ) )
			{
				string s = "";
				for ( int j = i + 9; data[j] != 13; j++ ) s += data[j];
				res = atoi( s.c_str() );
			}
		}
		if ( data[i] == 0 )return -1;

	}
	return res;
}

/*
ff d9 0d 0aを探すための関数
param:string s:受信した部分文字列(直前の受信データの末尾4文字が入ってる場合もある)
return ff d9 0d 0aが見つかったらその場所(0aの場所), そうでなければ0
*/
int jpegEndSeq( char *s, int size )
{
	/* 4文字ずつチェックしていく */
	for ( int i = 0; i < size; i++ )
	{
		if ( i >= 3 && ( unsigned char ) s[i - 3] == '\r' && ( unsigned char ) s[i - 2] == '\n' && s[i - 1] == '\r' && s[i] == '\n' )
		{
			return i;
		}
	}
	return 0;
}



//--------------------------------------------------------
// ファイルを指定ディレクトリ内から検索します
//【引数】FName：ファイル名、FDir：検索ディレクトリ
//--------------------------------------------------------
void SearchFile( CString FName, CString FDir )
{
	//cout << "search:" << FDir << endl;
	CFileFind FileFind;
	BOOL FndEndJug;

	//-----------------------------
	//検索ファイル名文字列を生成
	CString strSearchFile = FDir + _T( "\\" ) + FName;

	//----------------
	//検索実行
	if ( !FileFind.FindFile( strSearchFile ) )
		return;

	FndEndJug = TRUE;
	while ( FndEndJug )
	{

		//-------
		//検索
		FndEndJug = FileFind.FindNextFile();

		// "." , ".."を無視
		if ( FileFind.IsDots() )
			continue;

		//検索結果の判定
		if ( FileFind.IsDirectory() )
		{
			// サブ・ディレクトリ内を検索するための再帰呼び出し
			SearchFile( FName, FileFind.GetFilePath() );
		}
		else
		{
			// 検索結果をリスト・ボックスへ出力
			//cout << FileFind.GetFilePath() << endl; 
		}
	}
}

//****************************************************
//  フォルダごと削除するメソッド
//
//  引数：    lpszFolderPath    削除するフォルダのフルパス
//****************************************************
BOOL DeleteFolder( LPCTSTR lpszFolderPath )
{
	CFileFind fFind;

	// 検索用のワイルドカード付きパスを用意
	CString strFolderPath = lpszFolderPath;
	/*	if(!::PathIsDirectoryA( strFolderPath ))cout << strFolderPath << ": not found" << endl;
	else cout << strFolderPath << " found" << endl;*/
	strFolderPath.TrimRight( _T( "/" ) );
	strFolderPath += _T( "/*.*" );
	if ( !fFind.FindFile( strFolderPath ) )
		return FALSE;

	BOOL bRet = TRUE;
	while ( bRet )
	{
		bRet = fFind.FindNextFile();
		if ( fFind.IsDots() )
		{
			continue;
		}
		// GetFilePathを使うと
		// 正常にパスが取得できない場合があるので
		// フルパスを作成する処理を入れる
		CString strPath;
		strPath.Format
			( _T( "%s/%s" ), lpszFolderPath, fFind.GetFileName() );

		// フォルダがあれば再帰
		// ファイルなら削除
		if ( fFind.IsDirectory() )
			DeleteFolder( strPath );
		else
			DeleteFile( strPath );
	}
	fFind.Close();

	return RemoveDirectory( lpszFolderPath );
}

void CCamera::initDir()
{
	CString path = _T("");
	CString pathimg = _T("../img");
	CString symbol = _T("/");
	HANDLE hFile;
	path = pathimg;
	cout << path << "...";

	path = pathimg;
	cout << path << "...";
	if (::PathIsDirectoryA(path)) {}
	if (CreateDirectory(path, NULL))cout << "success" << endl;

	CString pathDB = _T( "../DB" );

	path = pathDB;
	cout << path << "...";

	path = pathDB;
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	// DBフォルダを再帰的に削除
	cout << " Folder all clean...";
	//	SearchFile(_T("*.*"), path);
	if ( !DeleteFolder( path ) )cout << "failed" << endl;
	else cout << "success" << endl;

	cout << "**** create ****" << endl;
	///////この部分をメソッド化してもよい/////
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	//////////////////////////////////////////

	path = pathDB + symbol + "Detected-Object";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	//hFileでつかんだハンドルは毎回クローズする. まとめてクローズしない
	path = pathDB + symbol + "Detected-Object" + symbol + "object.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "Human-DB";
	std::cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "Original";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "Process";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "Motion";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "Motion" + symbol + "bring";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "Motion" + symbol + "leave";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "Motion" + symbol + "traking";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "touch";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "Human-DB" + symbol + "touch" + symbol + "touch.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "Human-DB" + symbol + "Motion" + symbol + "motion.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "Human-DB" + symbol + "Skelton" + symbol + "skelton.log";
	cout << path << "...";

	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "Human-DB" + symbol + "human.log";
	cout << path << "...";

	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "Human-DB" + symbol + "human2.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "output-img";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "output-img" + symbol + "keypoint";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "output-img" + symbol + "roi";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "output-img" + symbol + "comparison.txt";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );


	path = pathDB + symbol + "output-img" + symbol + "description.txt";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "output-img" + symbol + "keypoint.txt";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "output-img" + symbol + "map.txt";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	path = pathDB + symbol + "output-img" + symbol + "object.txt";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );

	//------------------------------------------------イベント情報(隠す・飲む・移動持ち込み持ち去り)
	path = pathDB + symbol + "HI-Event-DB";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;
	path = pathDB + symbol + "HI-Event-DB" + symbol + "Event.log";;
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );
	//-------------------------------------------------固有ID人物情報
	path = pathDB + symbol + "Track-Human-DB";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;
	path = pathDB + symbol + "Track-Human-DB" + symbol + "Human.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );
	//-------------------------------------------------物体の出入り情報
	path = pathDB + symbol + "Object-in-out-DB";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;
	path = pathDB + symbol + "Object-in-out-DB" + symbol + "Object.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );
	//-------------------------------------------------

	path = pathDB + symbol + "rec_img";
	cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "track_img";//kawa
	std::cout << path << "...";
	if ( ::PathIsDirectoryA( path ) ) {}
	if ( CreateDirectory( path, NULL ) )cout << "success" << endl;

	path = pathDB + symbol + "object.log";
	cout << path << "...";
	if ( ::PathFileExistsA( path ) ) {}
	hFile = CreateFile( path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )cout << "success" << endl;
	CloseHandle( hFile );




	if ( pParent->getMode() == MODE_ONLINE )
	{
		CTime curTime = CTime::GetCurrentTime();
		CString strTime = curTime.Format( "%Y%m%d_%H%M%S" );
		strPathDir = "../img/" + strTime;
		if ( CreateDirectory( strPathDir, NULL ) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/RGB", NULL ) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/Depth", NULL ) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/Skeleton", NULL ) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/touch", NULL ) )cout << "success" << endl;			//2018/12/19 -> 2019/1/18 RGB_HRをやめてtouchに
		if ( CreateDirectory( "../img/" + strTime + "/HR", NULL) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/HR/color", NULL) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/HR/depth", NULL) )cout << "success" << endl;
		if ( CreateDirectory( "../img/" + strTime + "/HR/bodyindex", NULL) )cout << "success" << endl;
	}
	else
	{

	}

	//	RemoveDirectory("../DB/");
}

void CCamera::initImagePack()
{
	cv::Mat img = cv::Mat( 240, 320, CV_8UC3, cv::Scalar( 0, 0, 0 ) );

	pParent->imagepack["Event1"] = MultiData( "EVENT", "string", "イベント1" );
	pParent->imagepack["Event1"].layout = "Evnet1:";
	pParent->imagepack["Event2"] = MultiData( "EVENT", "string", "イベント2" );
	pParent->imagepack["Event2"].layout = "Evnet2:";
	pParent->imagepack["Event3"] = MultiData( "EVENT", "string", "イベント3" );
	pParent->imagepack["Event3"].layout = "Evnet3:";

	pParent->imagepack["event_image3"] = MultiData( Util::convertMat2String( img ), "image", "イベント履歴3" );
	pParent->imagepack["event_image2"] = MultiData( Util::convertMat2String( img ), "image", "イベント履歴2" );
	pParent->imagepack["event_image1"] = MultiData( Util::convertMat2String( img ), "image", "イベント履歴1" );
	pParent->imagepack["layer_image"] = MultiData( Util::convertMat2String( img ), "image", "レイヤ画像" );
	pParent->imagepack["layer_image_label"] = MultiData( "レイヤ画像", "string", "レイヤ画像ラベル" );
	pParent->imagepack["move_image"] = MultiData( Util::convertMat2String( img ), "image", "move_image" ); //TODO: 日本語に直そう
}

void CCamera::setSkeleton( cv::Mat depth_img )
{
	cv::Mat image_data;
	depth_img.copyTo( image_data );
	//cv::flip(image_data,image_data, 1);
	for ( int i = 0; i < ( int ) ( input.set_skeltons() ).size(); i++ )
	{
		for ( int j = 0; j < ( int ) ( input.set_skeltons() )[i].size(); j++ )
		{
			int colorX = ( input.set_skeltons() )[i][j].first;
			int colorY = ( input.set_skeltons() )[i][j].second;
			cv::Scalar color_data = cv::Scalar( 0, 0, 0 );
			switch ( i )
			{
			case 1:
				color_data[0] = 0;
				color_data[1] = 0;
				color_data[2] = 255;
				break;
			case 2:
				color_data[0] = 255;
				color_data[1] = 128;
				color_data[2] = 0;
				break;
			case 3:
				color_data[0] = 255;
				color_data[1] = 128;
				color_data[2] = 128;
				break;
			case 4:
				color_data[0] = 255 / 4;
				color_data[1] = 255;
				color_data[2] = 255;
				break;
			case 5:
				color_data[0] = 255;
				color_data[1] = 255;
				color_data[2] = 255 / 4;
				break;
			case 6:
				color_data[0] = 255 / 4;
				color_data[1] = 255;
				color_data[2] = 255 / 4;
				break;
			default:
				color_data[0] = 255;
				color_data[1] = 255;
				color_data[2] = 255;
				break;
			}
			if ( j == 7 )//赤右手
			{
				color_data[0] = 0;
				color_data[1] = 0;
				color_data[2] = 255;
			}
			if ( j == 11 )//青左手
			{
				color_data[0] = 255;
				color_data[1] = 0;
				color_data[2] = 0;
			}
			if ( j == 3 )//頭
			{
				color_data[0] = 255;
				color_data[1] = 255;
				color_data[2] = 128;
				cv::circle( image_data, cv::Point( colorX, colorY ), 3, color_data, 3 );
			}

			cv::circle( image_data, cv::Point( colorX, colorY ), 2.5, color_data, 2.5 );
		}
	}
	input.depth_skelton_copyTo( image_data );
}
//--------------------------------------------------
// Name     : SendCommand(int camera,int task)
// Author   : Kazuhiro MAKI (CV-lab.)
// Updata   : 2006.07.26
// Function : ネットワークカメラに文字列を送る
// Argument : cameraID / カメラの番号
//            task   / 実行する制御
// return   : int型の変数
//--------------------------------------------------
int CCamera::SendCommand( int cameraId, int task )
{

	//時間計測用変数
	DWORD time1cicle = 0;
	DWORD time2cicle;

	bool OfflineMode = false;
	bool isKinectFlip = true;

	int ColorTimes = 0, DepthTimes = 0, BodyIndexTimes = 0, BodyInfoTimes = 0, sTTimes = 0, cRTTimes = 0, dRTTimes = 0;

	//起動時に細かな設定をconfigテキストから読み込むAutoモードか、手動設定モードかを選ぶ
	ifstream ifs( "configAutoExecution.txt" );
	if (ifs.fail())
    {
        std::cerr << "invaild file : configAutoExecution.txt" << std::endl;
        return -1;
    }
	string line;
	getline( ifs, line );
	if( line == "true" ){
		autoExecute = true;
	}else{
		autoExecute = false;
	}

//--------------------------------------------------2018/12/14
	//ここから解像度＆モード初期化
	if( autoExecute ){
		ifstream ifs( "configAutoSetting.txt" );
		if (ifs.fail())
		{
			std::cerr << "invaild file : configAutoSetting.txt" << std::endl;
			return -1;
		}
		string line;
		vector<string> lines;
		while ( getline(ifs, line) )
		{
			lines.push_back(line);
		}
		int Online_or_Offline;
		int flip_or_noflip;
		resoInput = atoi( lines[0].c_str() );
		receiveMode = atoi( lines[1].c_str() );
		Online_or_Offline = atoi( lines[2].c_str() );
		flip_or_noflip = atoi( lines[3].c_str() );
		DBname = lines[4];

		while(1){
			if( resoInput <= 0 || resoInput > 120 ){
				std::cout << "file format error" << endl;
				std::cout << "input resolution ( 1 ～ 120 )" << endl;
				cin >> resoInput;
			}
			else{
				resoMagnification = resoInput/120.; //resoInputを120で割ったもの、解像度縮小倍率
				resoX = resoInput*16;				//画像横サイズ(基本縦横比16：9、120倍すればkinectのフルサイズ1920：1080)
				resoY = resoInput*9;				//画像縦サイズ
				resoInputMS = (int)(resoInput * 8. / 3.);
				resoX_MS = resoInputMS * 4;
				resoY_MS = resoInputMS * 3;
				std::cout << "accepted resolution ( " << resoX << ", " << resoY << " )" << endl;
				break;
			}
		}
		while(1){
			if( receiveMode < 0 || receiveMode > 15 ){
				std::cout << "file format error" << endl;
				std::cout << "input receive mode ( 1 ～ 15 )" << endl;
				std::cout << "number(int) -> [X X X X](binary) -> [BodyInfo BodyIndex Depth Color]" << endl;
				cin >> receiveMode;
			}
			else{
				if( receiveMode % 2 ) recvColor = true;
				else recvColor = false;
				if( ( receiveMode / 2) % 2 ) recvDepth = true;
				else recvDepth = false;
				if( ( ( receiveMode / 2) / 2 ) % 2 ) recvBodyIndex = true;
				else recvBodyIndex = false;
				if( ( ( ( receiveMode / 2) / 2 ) / 2 ) % 2) recvBodyInfo = true;
				else recvBodyInfo = false;
				recvSTime = true;
				recvCReactiveTime = true;
				recvDReactiveTime = true;
				std::cout << "mode " << receiveMode << "( BInfo: " << recvBodyInfo << " BIndex: " << recvBodyIndex << " Depth: " << recvDepth << " Color: " << recvColor << " )" << endl;
				break;
			}
		}
		while(1){
			if( Online_or_Offline == 1 ){ OfflineMode = false; break; }
			else if( Online_or_Offline == 0 ){ OfflineMode = true; break; }
			else{
				cout << "invalid input(0 or 1): " << Online_or_Offline << endl;
				std::cout << "online or offline? : on[1]/off[0]" << endl;
				cin >> Online_or_Offline;
			}
		}
		while(1){
			if( flip_or_noflip == 1 ){ isKinectFlip = true; break; }
			else if( flip_or_noflip == 0 ){ isKinectFlip = false; break; }
			else{
				cout << "invalid input(0 or 1): " << flip_or_noflip << endl;
				std::cout << "kinect horizontal flip? : on(slower)[1]/off(faster)[0]" << endl;
				cin >> flip_or_noflip;
			}
		}

	}
	else{
		while(1){
			std::cout << "input resolution ( 1 ～ 120 )" << endl;
			cin >> resoInput;
			if( resoInput <= 0 || resoInput > 120 ){
				std::cout << "file format error" << endl;
			}
			else{
				resoMagnification = resoInput/120.; //resoInputを120で割ったもの、解像度縮小倍率
				resoX = resoInput*16;				//画像横サイズ(基本縦横比16：9、120倍すればkinectのフルサイズ1920：1080)
				resoY = resoInput*9;				//画像縦サイズ
				resoInputMS = (int)(resoInput * 8. / 3.);
				resoX_MS = resoInputMS * 4;
				resoY_MS = resoInputMS * 3;
				std::cout << "accepted resolution ( " << resoX << ", " << resoY << " )" << endl;
				break;
			}
		}

		while(1){
			std::cout << "input receive mode ( 1 ～ 15 )" << endl;
			std::cout << "number(int) -> [X X X X](binary) -> [BodyInfo BodyIndex Depth Color]" << endl;
			cin >> receiveMode;
			if( receiveMode < 0 || receiveMode > 15 ){
				std::cout << "file format error" << endl;
			}
			else{
				if( receiveMode % 2 ) recvColor = true;
				else recvColor = false;
				if( ( receiveMode / 2) % 2 ) recvDepth = true;
				else recvDepth = false;
				if( ( ( receiveMode / 2) / 2 ) % 2 ) recvBodyIndex = true;
				else recvBodyIndex = false;
				if( ( ( ( receiveMode / 2) / 2 ) / 2 ) % 2) recvBodyInfo = true;
				else recvBodyInfo = false;
				recvSTime = true;
				recvCReactiveTime = true;
				recvDReactiveTime = true;
				std::cout << "mode " << receiveMode << "( BInfo: " << recvBodyInfo << " BIndex: " << recvBodyIndex << " Depth: " << recvDepth << " Color: " << recvColor << " )" << endl;
				break;
			}
		}

		while(1){
			std::cout << "online or offline? : on[1]/off[0]" << endl;
			int Online_or_Offline;
			cin >> Online_or_Offline;
			if( Online_or_Offline == 1 ){ OfflineMode = false; break; }
			else if( Online_or_Offline == 0 ){ OfflineMode = true; break; }
			else{ cout << "invalid input(0 or 1): " << Online_or_Offline << endl; }
		}
		while(1){
			std::cout << "kinect horizontal flip? : on(slower)[1]/off(faster)[0]" << endl;
			int flip_or_noflip;
			cin >> flip_or_noflip;
			if( flip_or_noflip == 1 ){ isKinectFlip = true; break; }
			else if( flip_or_noflip == 0 ){ isKinectFlip = false; break; }
			else{ cout << "invalid input(0 or 1): " << flip_or_noflip << endl; }
		}
	
	}
//--------------------------------------------------

	if ( remove( "DetectionRunningTime_log.csv" ) == 0 )
	{ //FPSのためのラベル
		cout << "delete  file" << endl;
	}
	else
	{
		cout << "missing file remove" << endl;
	}
	if ( remove( "imageEditRunningTime_log.csv" ) == 0 )
	{
		cout << "delete  file" << endl;
	}
	else
	{
		cout << "missing file remove" << endl;
	}
	if ( remove( "EventSearchRunningTime_log.csv" ) == 0 )
	{
		cout << "delete  file" << endl;
	}
	else
	{
		cout << "missing file remove" << endl;
	}
	if ( remove( "interpretationRunningTime_log.csv" ) == 0 )
	{
		cout << "delete  file" << endl;
	}
	else
	{
		cout << "missing file remove" << endl;
	}

	if ( pParent->getMode() == MODE_NONE )
	
	{
		//return 0;
	}
	initDir();
	initImagePack();

	CFileTime programTime;
	CFileTime programStart, programProgress;
	programStart = programTime.GetCurrentTime();

	clock_t tm_start = clock();
	input.init();

	Connection connection;
	frame = 0;
	CvFont font;
	cvInitFont( &font, CV_FONT_HERSHEY_DUPLEX, 0.4, 0.4 );

	std::cout << "recv input" << std::endl;
	clock_t tm_mid_pre = clock();

	zmq::context_t context( 1 );
	zmq::socket_t socket( context, ZMQ_PUSH );

	string VisualServerIP = "";
	{
		ifstream ifs( "configVisualServer.txt" );
		string line;
		for ( int i = 0; getline( ifs, line ); ++i )
		{
			if ( i >= 1 )
			{
				cout << "invaild file : configVisualServer.txt" << endl;
				return 1;
			}
			VisualServerIP = line;
		}
	}


	try
	{
		// VisualServerのIPアドレス
		socket.connect(VisualServerIP.c_str());
		//socket.connect( "tcp://10.40.1.105:8081" );　　決め打ちの名残
		//socket.connect( "tcp://133.19.23.114:8081" );
		//socket.connect("tcp://10.40.1.120:8081");
	}
	catch ( zmq::error_t e )
	{
		cout << "conection error:" << ( string ) e.what() << endl;
	}
	cout << "connect:" << VisualServerIP.c_str() << "(VisualServer)" <<endl;
	zmq::context_t contextReqester( 1 );
	zmq::socket_t socketReqester( contextReqester, ZMQ_PULL );
	try
	{
		socketReqester.bind( "tcp://*:50001" );
	}
	catch ( zmq::error_t e )
	{
		cout << "bind error:" << ( string ) e.what() << endl;
	}
	cout << "bind" << endl;
	input.init();
	input.kinect_on = true;
	input.set_depth_size( 320, 240 );
	CFileTime currentTime;
	CFileTime timeStart, timeEnd;
	double fps = 0.0;

	string senderName = "";
	{
		ifstream ifs( "configSenderName.txt" );
		string line;
		for ( int i = 0; getline( ifs, line ); ++i )

		{
			if ( i >= 1 )
			{
				cout << "invaild file : configSenderName.txt" << endl;
				return 1;
			}
			senderName = line;
		}
	}
	assert( senderName != "" );
	pParent->imagepack["senderName"] = MultiData( senderName, "string", "" );


	CFileTime Tlabel1, Tlabel2, Tlabel3, Tlabel4, Tlabel5, Tlabel6, Tlabel7, Tlabel8, Tlabel9; //TODO: FPSのためのラベル
	ofstream ofs_Time( "ProgramRunningTime_log.csv" );
	if ( !ofs_Time.is_open() )
	{
		cout << "ProgramRunningTime_log.csv not open" << endl;
		return 1;
	}
	CFileTimeSpan RunningTime;
	long long int spanTime;

	long long diffFrame = 0;
	timeStart = currentTime.GetCurrentTime();


	// DBの検索結果を送る用ソケット
	zmq::context_t ctxSearch( 1 );
	zmq::socket_t socketSearchResultSender( ctxSearch, ZMQ_PUSH );


	try
	{
		// VisualServerのIPアドレス
		socketSearchResultSender.connect(VisualServerIP.c_str());
		//socketSearchResultSender.connect( "tcp://10.40.1.105:8081" );　　決め打ちの名残
		//socketSearchResultSender.connect( "tcp://133.19.23.114:8081" );
		//socketSearchResultSender.connect( "tcp://10.40.1.120:8081" );
	}
	catch ( zmq::error_t e )
	{
		cout << "conection error:" << ( string ) e.what() << endl;
	}
	cout << "connect:" << VisualServerIP.c_str() << "(SearchResultSender)" <<endl;

	cv::Mat mask;

	if( isKinectFlip ){
		input.copy_obj_mask_area( cv::imread( "./mask_object_area_flip.png" ) );
		input.obj_mask_areaOrig( mask );
	}else{
		input.copy_obj_mask_area( cv::imread( "./mask_object_area.png" ) );
		input.obj_mask_areaOrig( mask );
	}

	DBConnect();

	vector<map<string, int>> detectedObj;
	vector<map<string, string>> detectedObj2;
	std::vector<float> layVec(320*240, 0);

	DWORD t_recv_start, t_imageSet_start, t_save_start, t_detection_start, t_output_start, t_connectDB_start, t_end;
	DWORD t_saveColor, t_saveDepth, t_saveSkelton, t_saveColorHR;
	DWORD t_recvColor, t_recvDepth, t_recvBodyIndex, t_recvBodyInfo, t_recvSTime, t_recvCReactiveTime, t_recvDReactiveTime;

	//for saving frame
	bool touch_flagT0 = false, touch_flagT1 = false;
	int lastEveID = 0;
	int noTouch = 0;
	int doubtTouch = 0;
	bool rmvStart = false;		//->廃止予定
	long long rmvFrame = 0;		//未使用
	long long numMemHRMax = 25;	//未使用
	int addEvent = -1;			//イベントが新しく増えた時にイベントIDを受け取る、通常-1

	/*struct saveReservation{
		int eventID;
		long long firstFrame;
		long long lastFrame;
	};

	struct memoryImages{
		long long frames;
		cv::Mat saveimages;
	};*/

	//vector<std::pair<char*, std::pair<long long, long long>>> saveRsv;	// グローバルに変更
	vector<HANDLE> saveThreads;
	vector<pair<long long, std::string>> memSkelton;
	int num_memSkelton = 0;
	long long pastFrame = 0;

	//Mutex作成
	saveMutex = CreateMutex(NULL, FALSE, NULL);
	stateMutex = CreateMutex(NULL, FALSE, NULL);
	rsvMutex = CreateMutex(NULL, FALSE, NULL);
	managerMutex = CreateMutex( NULL, FALSE, NULL );

	while ( 1 )
	{
		t_recv_start = timeGetTime();

		pParent->imagepack.clear();
		pParent->imagepack["senderName"] = MultiData( senderName, "string", "" );
		////Tlabel1 = currentTime.GetCurrentTime(); //TODO: FPSのためのラベル

		programProgress = currentTime.GetCurrentTime();
		CFileTimeSpan programTimeSpan;
		programTimeSpan = programProgress - programStart;
		long long int programProgressTime;
		char operatingTime[128];

		pParent->imagepack["Event1"] = MultiData( "none", "string", "イベント履歴1ラベル" );
		programProgressTime = ( long long int )programTimeSpan.GetTimeSpan() / 10000000; //秒に直す
		sprintf_s( operatingTime, "%d", programProgressTime % 60 );
		pParent->imagepack["programTime_s"] = MultiData( string( operatingTime ), "string", "稼働時間_秒" );
		programProgressTime /= 60; //分
		sprintf_s( operatingTime, "%d", programProgressTime % 60 );
		pParent->imagepack["programTime_m"] = MultiData( string( operatingTime ), "string", "稼働時間_分" );
		programProgressTime /= 60; //時間
		sprintf_s( operatingTime, "%d", programProgressTime % 24 );
		pParent->imagepack["programTime_h"] = MultiData( string( operatingTime ), "string", "稼働時間_時" );
		programProgressTime /= 24; //日
		sprintf_s( operatingTime, "%d", programProgressTime );
		pParent->imagepack["programTime_d"] = MultiData( string( operatingTime ), "string", "稼働時間_日" );

		//////Tlabel2 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel2 - Tlabel1;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";

		//connection.send( "ok" );

		//////Tlabel3 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel3 - Tlabel2;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";

		// データを受信する受け皿
		string message;
		//connection.recv( message );					// サーバから帰ってくる文字列 →ここでは受け取らない(2018/12/03変更)

		//////Tlabel4 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel4 - Tlabel3;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";

		KinectPack kinectPack;
		//cv::Mat imgKinectRGB;
		cv::Mat imageColorOriginal, imageColor, colorMat;
		//cv::Mat imgKinectDepthOriginal;
		cv::Mat imageDepthOriginal, imageDepth, depthMat, imageDepth8U;
		//cv::Mat imageBodyIndex_original;
		cv::Mat imageBodyIndexOriginal, imageBodyIndex, bodyindexMat;

		if( OfflineMode ){
			//KinectV2Serverと通信せず、用意した画像シーケンスを用いてテストを行うモード

		}
		else if(resoInput <= 0){
			connection.send( "ok" );
			connection.recv( message );					// サーバから帰ってくる文字列 →データ自体はまだ受け取らない(2018/12/03変更)
			MsgPackUtil::unpack( message, kinectPack );
			// 文字列をMat型に変換
			
			try
			{
				KinectPackUtil::convertImageToMat( kinectPack.imageColor, imageColor );
			}
			catch ( const cv::Exception &e )
			{
				std::cout << "Color:" << e.what() << endl;
			}
			try
			{
				KinectPackUtil::convertImageToMat( kinectPack.imageDepth, imageDepth );
			}
			catch ( const cv::Exception &e )
			{
				std::cout << "Depth:" << e.what() << endl;
			}
			try
			{
				KinectPackUtil::convertImageToMat( kinectPack.imageBodyIndex, imageBodyIndex );
			}
			catch ( const cv::Exception &e )
			{
				std::cout << "BodyIndex:" << e.what() << endl;
			}
		}
		else{
			string reqModeStrings;
			reqModeStrings = to_string( (long long)receiveMode );
			connection.send( reqModeStrings.c_str() );
			connection.recv( message );
			//std::cout << message << endl;

			while( recvColor == 1 || recvDepth == 1 || recvBodyIndex == 1 || recvBodyInfo == 1 || recvSTime == 1 || recvCReactiveTime == 1 || recvDReactiveTime == 1 ){
				if( recvColor == 1 ){

					t_recvColor = timeGetTime();
					ostringstream oss;
					oss << "Color" << ColorTimes;
					string reqStrColor = oss.str().c_str();

					connection.send( reqStrColor );//"Color"+ ColorTimes );
//					cout << "send request: " << reqStrColor << endl;
																								DWORD tC0 = timeGetTime();
					connection.recv( message );
//					cout << "receive image: Color" << endl;
					ColorTimes++;

					if( message.compare( "error" ) != 0 ){
																								DWORD tC1 = timeGetTime();
						vector<unsigned char> colorVec( message.begin(), message.end() );
																								DWORD tC2 = timeGetTime();
						cv::Mat colorVtoMat( colorVec );
																								DWORD tC3 = timeGetTime();
						imageColorOriginal = imdecode( colorVtoMat, 1 );
																								DWORD tC4 = timeGetTime();
						if(imageColorOriginal.cols == resoX && imageColorOriginal.rows == resoY){
							imageColor = imageColorOriginal.clone();
						}else{
							resize( imageColorOriginal, imageColor, cv::Size( resoX, resoY ) );
						}
																								DWORD tC5 = timeGetTime();
						//colorMat = imageColor.clone().reshape(0, 1);
																								//DWORD tC6 = timeGetTime();
						for( int col = 0; col < colorVtoMat.cols; col++ ){
							kinectPack.imageColor.image.push_back( colorVtoMat.data[ col * 3 ] );
							kinectPack.imageColor.image.push_back( colorVtoMat.data[ col * 3 + 1 ] );
							kinectPack.imageColor.image.push_back( colorVtoMat.data[ col * 3 + 2 ] );
						}
						kinectPack.imageColor.width = imageColor.cols;
						kinectPack.imageColor.height = imageColor.rows;
						kinectPack.imageColor.channels = imageColor.channels();
						//imageColorView = imageColor.clone();
																								DWORD tC6 = timeGetTime();
						recvColor = 2;
																								//cout << "Color" << endl;
																								//cout << "receive				: " << (double)( tC1 - tC0 ) << "msec" << endl;
																								//cout << "message->vector			: " << (double)( tC2 - tC1 ) << "msec" << endl;
																								//cout << "vector->Mat			: " << (double)( tC3 - tC2 ) << "msec" << endl;
																								//cout << "Mat->Mat(decode)		: " << (double)( tC4 - tC3 ) << "msec" << endl;
																								//cout << "Mat(decode)->Mat(resize)	: " << (double)( tC5 - tC4 ) << "msec" << endl;
																								////cout << "Mat(resize)->Mat(clone&reshape)	: " << (double)( tC6 - tC5 ) << "msec" << endl;
																								//cout << "KinectPack.imageColor		: " << (double)( tC6 - tC5 ) << "msec" << endl;
					}
				}
				if( recvDepth == 1 ){

					t_recvDepth = timeGetTime();
					ostringstream oss;
					oss << "Depth" << DepthTimes;
					string reqStrDepth = oss.str().c_str();
					
					connection.send( reqStrDepth );//"Depth" + ColorTimes );
//					cout << "send request: " << reqStrDepth << endl;

																									DWORD tD0 = timeGetTime();
					connection.recv( message );
//					cout << "receive image: Depth" << endl;
					DepthTimes++;

					if( message.compare( "error" ) != 0 ){
																									DWORD tD1 = timeGetTime();
						vector<unsigned char> depthVec( message.begin(), message.end() );
																									DWORD tD2 = timeGetTime();
						cv::Mat depthVtoMat( depthVec );
																									DWORD tD3 = timeGetTime();
						imageDepthOriginal = cv::imdecode( depthVtoMat, -1 );
																									DWORD tD4 = timeGetTime();
						if(imageDepthOriginal.cols == resoX && imageDepthOriginal.rows == resoY){
							imageDepth = imageDepthOriginal.clone();
						}else{
							resize( imageDepthOriginal, imageDepth, cv::Size( resoX, resoY ) );
						}
																									DWORD tD5 = timeGetTime();
						//depthMat = imageDepth.clone().reshape(0, 1);
																									//DWORD tD6 = timeGetTime();
						for( int col = 0; col < depthVtoMat.cols; col++ ){
							kinectPack.imageDepth.image.push_back( depthVtoMat.data[ col * 2 ] );
							kinectPack.imageDepth.image.push_back( depthVtoMat.data[ col * 2 + 1 ] );
						}
						kinectPack.imageDepth.width = imageDepth.cols;
						kinectPack.imageDepth.height = imageDepth.rows;
						kinectPack.imageDepth.channels = imageDepth.channels();
						//imageDepthView = imageDepth.clone();
																									DWORD tD6 = timeGetTime();
						imageDepth.convertTo( imageDepth8U, CV_8UC1, 255./8000., 0 );
																									DWORD tD7 = timeGetTime();
						recvDepth = 2;
																									//cout << "Depth" << endl;
																									//cout << "receive				: " << (double)( tD1 - tD0 ) << "msec" << endl;
																									//cout << "message->vector			: " << (double)( tD2 - tD1 ) << "msec" << endl;
																									//cout << "vector->Mat			: " << (double)( tD3 - tD2 ) << "msec" << endl;
																									//cout << "Mat->Mat(decode)		: " << (double)( tD4 - tD3 ) << "msec" << endl;
																									//cout << "Mat(decode)->Mat(resize)	: " << (double)( tD5 - tD4 ) << "msec" << endl;
																									////cout << "Mat(clone)->Mat(reshape)	: " << (double)( tD6 - tD5 ) << "msec" << endl;
																									//cout << "KinectPack.imageDepth		: " << (double)( tD6 - tD5 ) << "msec" << endl;
																									//cout << "convertToDepth8U		: " << (double)( tD7 - tD6 ) << "msec" << endl;
					}
				}
				if( recvBodyIndex == 1 ){

					t_recvBodyIndex = timeGetTime();
					ostringstream oss;
					oss << "BodyIndex" << BodyIndexTimes;
					string reqStrBodyIndex = oss.str().c_str();

					connection.send( reqStrBodyIndex );//"BodyIndex" + BodyIndexTimes );
//					cout << "send request: " << reqStrBodyIndex << endl;
																									DWORD tBI0 = timeGetTime();
					connection.recv( message );
//					cout << "receive image: BodyIndex" << endl;
					BodyIndexTimes++;

					if( message.compare( "error" ) != 0 ){
																									DWORD tBI1 = timeGetTime();
						vector<unsigned char> bodyindexVec( message.begin(), message.end() );
																									DWORD tBI2 = timeGetTime();
						cv::Mat bodyindexVtoMat( bodyindexVec );
																									DWORD tBI3 = timeGetTime();
						imageBodyIndexOriginal = cv::imdecode( bodyindexVtoMat, 0 );
																									DWORD tBI4 = timeGetTime();
						if(imageBodyIndexOriginal.cols == resoX && imageBodyIndexOriginal.rows == resoY){
							imageBodyIndex = imageBodyIndexOriginal.clone();
						}else{
							resize( imageBodyIndexOriginal, imageBodyIndex, cv::Size( resoX, resoY ) );
						}
																									DWORD tBI5 = timeGetTime();
						//bodyindexMat = imageBodyIndex.clone().reshape(0, 1);
																									//DWORD tBI6 = timeGetTime();
						for( int col = 0; col < bodyindexMat.cols; col++ ){
							if( bodyindexMat.data[col] >= 6 ){
								cout << bodyindexMat.data[col] << "(bodyindexMat.data[" << col << "])" << endl;
							}
							kinectPack.imageBodyIndex.image.push_back( bodyindexMat.data[ col ] );
						}
						kinectPack.imageBodyIndex.width = imageBodyIndex.cols;
						kinectPack.imageBodyIndex.height = imageBodyIndex.rows;
						kinectPack.imageBodyIndex.channels = imageBodyIndex.channels();
						//imageBodyIndexView = imageBodyIndex.clone();
																									DWORD tBI6 = timeGetTime();
						recvBodyIndex = 2;
																									//cout << "BodyIndex" << endl;
																									//cout << "receive				: " << (double)( tBI1 - tBI0 ) << "msec" << endl;
																									//cout << "message->vector			: " << (double)( tBI2 - tBI1 ) << "msec" << endl;
																									//cout << "vector->Mat			: " << (double)( tBI3 - tBI2 ) << "msec" << endl;
																									//cout << "Mat->Mat(decode)		: " << (double)( tBI4 - tBI3 ) << "msec" << endl;
																									//cout << "Mat(decode)->Mat(resize)	: " << (double)( tBI5 - tBI4 ) << "msec" << endl;
																									////cout << "Mat(clone)->Mat(reshape)	: " << (double)( tBI6 - tBI5 ) << "msec" << endl;
																									//cout << "KinectPack.imageBodyIndex	: " << (double)( tBI6 - tBI5 ) << "msec" << endl;
					}
				}
				if( recvBodyInfo == 1 ){

					t_recvBodyInfo = timeGetTime();
					ostringstream oss;
					oss << "BodyInfo" << BodyInfoTimes;

					string reqStrBodyInfo = oss.str().c_str();

					connection.send( reqStrBodyInfo );//"BodyInfo" + BodyInfoTimes );
//					cout << "send request: " << BodyInfoTimes << endl;
																		DWORD tS0 = timeGetTime();
					connection.recv( message );
//					cout << "receive data: BodyInfo" << endl;
					BodyInfoTimes++;

					if( message.compare( "error" ) != 0 ){
																		DWORD tS1 = timeGetTime();
						kinectPack.bodies.jsonBodyInfo += message;		// フルHD準拠のスケルトンを受け取ってしまっているので補正する必要有り->drawSkelton()にて調整処理追加
																		DWORD tS2 = timeGetTime();
						recvBodyInfo = 2;
																		//cout << "BodyInfo" << endl;
																		//cout << "receive				: " << (double)( tS1 - tS0 ) << "msec" << endl;
																		//cout << "KinectPack.bodies.jsonBodyInfo	: " << (double)( tS2 - tS1 ) << "msec" << endl;
					}
				}
				if( recvSTime == 1 ){

					t_recvSTime = timeGetTime();
					ostringstream oss;
					oss << "sTime" << sTTimes;
					string reqStrST = oss.str().c_str();

					connection.send( reqStrST );//"sTime" + sTTimes );
					connection.recv( message );
					if( message.compare( "error" ) != 0 ){
						kinectPack.sTime += message;
						recvSTime = 2;
					}
					sTTimes++;
				}
				if( recvCReactiveTime == 1 ){

					t_recvCReactiveTime = timeGetTime();
					ostringstream oss;
					oss << "cReactiveTime" << cRTTimes;
					string reqStrCRT = oss.str().c_str();

					connection.send( reqStrCRT );//"cReactiveTime" + cRTTimes );
					connection.recv( message );
					if( message.compare( "error" ) != 0 ){
						kinectPack.cReactiveTime = stoll( message );
						recvCReactiveTime = 2;
					}
					cRTTimes++;
				}
				if( recvDReactiveTime == 1 ){

					t_recvDReactiveTime = timeGetTime();
					ostringstream oss;
					oss << "dReactiveTime" << dRTTimes;
					string reqStrDRT = oss.str().c_str();

					connection.send( reqStrDRT );//"dReactiveTime" + dRTTimes );
					connection.recv( message );
					if( message.compare( "error" ) != 0 ){
						kinectPack.dReactiveTime = stoll( message );
						recvDReactiveTime = 2;
					}
					dRTTimes++;
				}
			}
		}
		cout << frame << "(frame)" << endl;

		t_imageSet_start = timeGetTime();

		imshow("Color", imageColor);										//☆
		//imshow("ColorOriginal;", imageColorOriginal); //full size			//☆
		//imshow("Depth", imageDepth);										//☆受信データ(画像)を見たい場合はここのコメントアウトを外す
		imshow("BodyIndex", imageBodyIndexOriginal);						//☆
		//imshow("Depth8U", imageDepth8U);									//☆
		//imshow("mask", mask);												//☆
		cv::waitKey(1);														//☆
		//cout << frame << "(frame)" << endl;

		//TODO: image→Matv2
		imgInput = Matv2( cv::Size( 320, 240 ), CV_8UC3 );
		input.set( 320, 240 );

		int x, y, width, heigth;
		x = ( imageColor.cols / 2 ) - 160;
		y = ( imageColor.rows / 2 ) - 120;
		width = 320;
		heigth = 240;
		cv::Mat roi_img( imageColor, cv::Rect( x, y, width, heigth ) );
		input.img_original = roi_img.clone();

		cv::Mat roi_img1( imageDepth, cv::Rect( x, y, width, heigth ) );
		input.img_depth = roi_img1.clone();

		cv::Mat roi_img2( imageBodyIndex, cv::Rect( x, y, width, heigth ) );
		//imageBodyIndexOriginal.copyTo(roi_img2);

		input.img_resize = input.img_original.clone();
		cv::Mat depth = input.img_depth.clone();
		//cv::blur(depth, input.img_depth,cv::Size(3,3));

		cv::cvtColor( input.img_resize, input.img_HLS, CV_BGR2HLS );
		input.img_HLS.convertTo( input.img_HLS, CV_32FC3 );
		//input.img_resize.convertTo( input.img_resize, CV_8UC3 );	// ←これいる？
		input.sizeWidth = input.img_original.cols / 320;
		input.sizeHeigth = input.img_original.rows / 240;

		//まとめ
		//imageXXXOriginal	: KinectV2から受信したデータをMat型にデコードしたそのもの
		//imageXXX			: MonitoringSystem起動時に入力した解像度でリサイズしたもの(resoInput = 30 のとき、480*270)
		//input.img_original: imageColorの中心を基準に、320*240で切り出したもの
		//input.img_depth	: ↑のimgeDepth版
		//img.resize		: input.img_originalのコピー
		//depth				: input.img_depthのコピー
		//input.img_HLS		: input.img_resizeをHLS表色系に変換したもの

		//////Tlabel5 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel5 - Tlabel4;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";

		int r, g, b;
		float L;
		for ( int y = 0; y < input.Height(); y++ )
		{
			for ( int x = 0; x<input.Width(); x++ )
			{
				r = input.img_resize.data[y * input.img_resize.step + x * input.img_resize.elemSize() + 2];
				g = input.img_resize.data[y * input.img_resize.step + x * input.img_resize.elemSize() + 1];
				b = input.img_resize.data[y * input.img_resize.step + x * input.img_resize.elemSize() + 0];
				USHORT distance_16U = input.img_depth.at<USHORT>( y, x ); // >> 3;									//320*240にresizeしたDepth画像の画素値を取得
				if( distance_16U > 0 && distance_16U < 500 ){ 
					//cout << "distance16U value[" << x << ", " << y << "]: " << distance_16U << endl;
				}
				if ( /*distance_16U >= 0 &&*/ distance_16U < 500 ){ distance_16U = DEPTH_RANGE_MAX; }				//↑の値が0の場合、DEPTH_RANGE_MAX(4500)に変更
				int id = ( roi_img2.data[roi_img2.step*y + roi_img2.elemSize()*x + 0] + 1 ) % 256;//&((1<<3)-1);	//id は320*240にclipingしたBodyIndex画像の画素値+1(人物IDは1から、人物領域外は0)
				//if ( x>280 ) { id = 0; }																			//xが280(つまり、右端40pixel幅分)は問答無用でid = 0 (人物を検出しないエリア指定か？なぜ右端のみ？)
				//L=input.img_HLS.data[y*input.img_HLS.step+x*input.img_HLS.elemSize()+1];
				//L=( 0.298912 * r + 0.586611 * g + 0.114478 * b );
				if( distance_16U != DEPTH_RANGE_MAX ){ distance_16U = min( distance_16U, ( USHORT ) 4500 ); }		//↑で取得したdepth画素値と4000のうち、小さい方を採用して上書き
				L = ( ( cv::Point3f* )( input.img_HLS.data + input.img_HLS.step.p[0] * y ) )[x].y;
				input.set_input_B( x, y, b );
				input.set_Out_B( x, y, b );
				input.set_input_G( x, y, g );
				input.set_Out_G( x, y, g );
				input.set_input_R( x, y, r );
				input.set_Out_R( x, y, r );
				input.set_input_L( x, y, L );
				input.set_depth( x, y, distance_16U );			//input.depth[x, y] = distance_16U(= 1～4000)
				input.set_human_id( x, y, id );					//input.depth_human_id[x, y] = id(0：人物なし、1～6：人物ID)
				layVec[y * input.Width() + x] = distance_16U;
			}
		}
		cv::Mat viewdepth, viewdepth8U;
		input.depth_copyTo(viewdepth);
		//viewdepth.convertTo( viewdepth8U, CV_8U, 255. / 2990., -1625. * 255. / 2990. );
		viewdepth.convertTo( viewdepth8U, CV_8U, 255. / 4500., 0. );
		//imshow("input.depth", viewdepth);
		//imshow("input.depth8U", viewdepth8U);

		KinectPackUtil::drawSkeleton( input, kinectPack );				//骨格を取得(KinectV2Server側で左右反転処理を【している場合】こちら)
		//KinectPackUtil::drawSkeletonFlip( input, kinectPack );		//骨格を取得(KinectV2Server側で左右反転処理を【していない場合】こちら)

		//////Tlabel6 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel6 - Tlabel5;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";

		t_save_start = timeGetTime();

#if 0
		//ファイル保存////////////////////////////////////////////////////////////////

		t_saveColor = timeGetTime();

		//画像保存(常時)(高解像度だと容量オーバーするのでコメントアウト)

		char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
		sprintf_s( filenameRGB, "%s/HR/color/%08lu_color.png", strPathDir, frame );
		cv::imwrite(string(filenameRGB), imageColorOriginal);
		//cv::imwrite( string( filenameRGB ), input.img_resize );
		cv::waitKey( 1 );

		t_saveDepth = timeGetTime();

		char filenameDepth[256];
		sprintf_s( filenameDepth, "%s/HR/depth/%08lu_depth.png", strPathDir, frame );
		cv::imwrite(string(filenameDepth), imageDepthOriginal);
		//cv::imwrite( string( filenameDepth ), imageDepth );
		cv::waitKey(1);

		t_saveSkelton = timeGetTime();

		char filenameBodyIndex[256];
		sprintf_s(filenameBodyIndex, "%s/HR/bodyindex/%08lu_bodyindex.png", strPathDir, frame);
		cv::imwrite(string(filenameBodyIndex), imageBodyIndexOriginal);
		cv::waitKey(1);
		

		//スケルトン保存(常時)(スケルトンはOpenPoseの方を信用するためコメントアウト)

		/*char filenameSkeleton[128];
		sprintf( filenameSkeleton, "%s/Skeleton/%08lu.txt", strPathDir, frame );
		{
			ofstream ofs( filenameSkeleton );

			ofs << input.set_skeltons().size() << endl;
			try
			{
				for ( int i = 0; i < input.set_skeltons().size(); i++ )
				{

					ofs << i << endl;
					for ( int j = 0; j < input.set_skeltons()[i].size(); j++ )
					{
						ofs << input.set_skeltons()[i][j].first << " " << input.set_skeltons()[i][j].second << " " << input.get_depth( input.set_skeltons()[i][j].first, input.set_skeltons()[i][j].second ) << endl;
					}

				}
			}
			catch ( const exception &e )
			{
				cout << frame << ":exception:" << e.what() << endl;
			}
		}
		cv::waitKey(1);*/

		t_saveColorHR = timeGetTime();

		//画像保存(高解像度)@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 2018/12/19->2019/1/8コメントアウト
		
		//if( frame == numSaveHRMax ){
		//	//古い画像データを随時削除するための開始フラグ
		//	rmvStart = true;
		//}
		//if( rmvStart ){
		//	//ここにファイルを削除する処理を追加
		//	char filenameRGB_remove[256];
		//	sprintf_s( filenameRGB_remove, "%s/RGB_HR/%08lu.png", strPathDir, rmvFileFrame );
		//	DeleteFile( filenameRGB_remove );
		//	rmvFileFrame ++;
		//}
		char filenameRGB_HR[256];
		char filenameDepth_HR[256];
		char filenameBodyIndex_HR[256];
		char filenameBodyInfo_HR[256];
		sprintf_s( filenameRGB_HR, "%s/HR/%08lu_color.ppm", strPathDir, frame );
		sprintf_s( filenameDepth_HR, "%s/HR/%08lu_depth.pgm", strPathDir, frame );
		sprintf_s( filenameBodyIndex_HR, "%s/HR/%08lu_bodyindex.pgm", strPathDir, frame );
		sprintf_s( filenameBodyInfo_HR, "%s/HR/%08lu_info.txt", strPathDir, frame );
		/*ofstream ofs_filenameBodyInfo_HR( filenameBodyInfo_HR );
		cv::imwrite( string( filenameRGB_HR ), imageColorOriginal );
		cv::imwrite( string( filenameDepth_HR ), imageDepthOriginal );
		cv::imwrite( string( filenameBodyIndex_HR ), imageBodyIndexOriginal );
		ofs_filenameBodyInfo_HR << kinectPack.bodies.jsonBodyInfo.c_str();
		ofs_filenameBodyInfo_HR.close();*/

#endif
		cv::waitKey( 1 );
		//////Tlabel7 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel7 - Tlabel6;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";


		time2cicle = timeGetTime();
		//cout << "one cicle : " << (double)( time2cicle - time1cicle ) << "msec" << endl;
		time1cicle = time2cicle;

		t_detection_start = timeGetTime();

		//画像処理////////////////////////////////////////////////////////////////

		Detection(frame, connection, &touch_flagT1);			//TODO: かかった時間　人物、物体領域なし平均0.1秒　あるとき？  // 2019/1/9 接触検知があった場合フラグ1を返すように変更
		// Detection2(frame, connection, detectedObj2);			//TODO: かかった時間　人物、物体領域なし平均0.1秒　あるとき？
		// Detection3(frame, connection, detectedObj2, layVec); //TODO: かかった時間　人物、物体領域なし平均0.1秒　あるとき？
		// DetectionOffline(frame);								//TODO: かかった時間　人物、物体領域なし平均0.1秒　あるとき？

		//////Tlabel8 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel8 - Tlabel7;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";

		sprintf_s( filePath, "%s/touch", strPathDir );
		std::ostringstream oss;
		oss << lastEveID;

		/*if( monitorEvFin.empty() ){
			WaitForSingleObject(stateMutex, INFINITE);
			monitorEvFin.push_back( make_pair( lastEveID, make_pair( 0, 0 ) ) );
			ReleaseMutex( stateMutex );
		}
		bool monitorCheck = false;
		for( auto i = monitorEvFin.begin(); i != monitorEvFin.end(); i++ ){
			if( (*i).first == lastEveID ){
				break;
			}
			monitorCheck = true;
		}
		if( monitorCheck == true ){
			WaitForSingleObject(stateMutex, INFINITE);
			monitorEvFin.push_back( make_pair( lastEveID, make_pair( 0, 0 ) ) );
			ReleaseMutex( stateMutex );
		}*/

		int touch_object_size = -1;
		pair<int, vector<pair<T_OBJECT_ID, T_HUMAN_ID>>> touched_object_infos;
		T_OBJECT_ID temp_t_object_id;
		T_HUMAN_ID temp_t_human_id;

		//接触有無判定(初回？連続？)
		if( touch_flagT1 == 1 && touch_flagT0 == 0 ){ // 接触が発生した
			cout << "touch!" << endl;
			noTouch = 0;
			if( doubtTouch > 0 ){	//接触判定が終了してから3フレーム以内に再発生した場合、前のイベントの続きとみなす
				cout << "may keep touching..." << endl;

				WaitForSingleObject( managerMutex, INFINITE );

				( *( save_events.end() - 1 ) ).end_frame = frame + 10;

				ReleaseMutex( managerMutex );

				//WaitForSingleObject(rsvMutex, INFINITE);
				//saveRsv.push_back( make_pair( lastEveID - 1, make_pair( frame + 10 - doubtTouch, frame + 10 ) ) );
				//ReleaseMutex( rsvMutex );
				//WaitForSingleObject(stateMutex, INFINITE);
				//monitorEvFin[( ( lastEveID - 1 ) - monitorEvFin[0].first )].second.first ++;
				//ReleaseMutex( stateMutex );

				//物体位置と人物位置情報をGET
				touch_object_size = input.size_t_object_id();
				touched_object_infos.first = lastEveID - 1;
				for( int t_objID = 0; t_objID < touch_object_size; t_objID++ ){
					if( input.get_t_object_id(t_objID).t_frame == frame ){
						temp_t_object_id.x1 = input.get_t_object_id( t_objID ).x1;						// 現フレームと同じフレーム番号で登録された接触情報を検索しGET
						temp_t_object_id.x2 = input.get_t_object_id( t_objID ).x2;
						temp_t_object_id.y1 = input.get_t_object_id( t_objID ).y1;
						temp_t_object_id.y2 = input.get_t_object_id( t_objID ).y2;
						temp_t_object_id.xcenter = input.get_t_object_id( t_objID ).xcenter;
						temp_t_object_id.ycenter = input.get_t_object_id( t_objID ).ycenter;
						temp_t_object_id.size = input.get_t_object_id( t_objID ).size;
						temp_t_object_id.touch = input.get_t_object_id( t_objID ).touch;
						temp_t_object_id.t_frame = input.get_t_object_id( t_objID ).t_frame;
						for( int t_humanID = 0; t_humanID < input.size_t_human_id(); t_humanID++ ){
							if( input.get_t_human_id( t_humanID ).id == temp_t_object_id.touch ){
								temp_t_human_id.x1 = input.get_t_human_id( t_humanID ).x1;				// ↑得られた物体接触情報より、接触人物情報を検索しGET
								temp_t_human_id.x2 = input.get_t_human_id( t_humanID ).x2;
								temp_t_human_id.y1 = input.get_t_human_id( t_humanID ).y1;
								temp_t_human_id.y2 = input.get_t_human_id( t_humanID ).y2;
								temp_t_human_id.xcenter = input.get_t_human_id( t_humanID ).xcenter;
								temp_t_human_id.ycenter = input.get_t_human_id( t_humanID ).ycenter;
								touched_object_infos.second.push_back( make_pair( temp_t_object_id, temp_t_human_id ) );
								break;
							}
						}
					}
				}
			}
			else if( frame >= (long long)11 ){
				//画像保存予約

				WaitForSingleObject( managerMutex, INFINITE );

				ev_manager newEvent;
				newEvent.eventID = lastEveID;		// イベントID			：lastEveID
				newEvent.start_frame = frame - 10;	// 保存最初				：現フレーム-10
				newEvent.end_frame = frame + 10;	// 保存最後				：現フレーム+10
				newEvent.end_updateRange = false;	// 保存範囲更新終了？	：false
				newEvent.fin_save = false;			// 画像保存完了？		：false

				addEvent = lastEveID;

				save_events.push_back( newEvent );

				ReleaseMutex( managerMutex );

				//WaitForSingleObject(rsvMutex, INFINITE);
				//saveRsv.push_back( std::make_pair( lastEveID, std::make_pair( frame - 10, frame + 10 ) ) );		// 10フレーム後に、10フレーム前から計21フレーム分を保存する予約
				//ReleaseMutex(rsvMutex);
				//WaitForSingleObject(stateMutex, INFINITE);
				//monitorEvFin[( lastEveID - monitorEvFin[0].first )].second.first ++;
				//ReleaseMutex( stateMutex );

				//物体位置と人物位置情報をGET
				touch_object_size = input.size_t_object_id();
				touched_object_infos.first = lastEveID;
				for( int t_objID = 0; t_objID < touch_object_size; t_objID++ ){
					if( input.get_t_object_id(t_objID).t_frame == frame ){
						temp_t_object_id.x1 = input.get_t_object_id( t_objID ).x1;						// 現フレームと同じフレーム番号で登録された接触情報を検索しGET
						temp_t_object_id.x2 = input.get_t_object_id( t_objID ).x2;
						temp_t_object_id.y1 = input.get_t_object_id( t_objID ).y1;
						temp_t_object_id.y2 = input.get_t_object_id( t_objID ).y2;
						temp_t_object_id.xcenter = input.get_t_object_id( t_objID ).xcenter;
						temp_t_object_id.ycenter = input.get_t_object_id( t_objID ).ycenter;
						temp_t_object_id.size = input.get_t_object_id( t_objID ).size;
						temp_t_object_id.touch = input.get_t_object_id( t_objID ).touch;
						temp_t_object_id.t_frame = input.get_t_object_id( t_objID ).t_frame;
						for( int t_humanID = 0; t_humanID < input.size_t_human_id(); t_humanID++ ){
							if( input.get_t_human_id( t_humanID ).id == temp_t_object_id.touch ){
								temp_t_human_id.x1 = input.get_t_human_id( t_humanID ).x1;				// ↑得られた物体接触情報より、接触人物情報を検索しGET
								temp_t_human_id.x2 = input.get_t_human_id( t_humanID ).x2;
								temp_t_human_id.y1 = input.get_t_human_id( t_humanID ).y1;
								temp_t_human_id.y2 = input.get_t_human_id( t_humanID ).y2;
								temp_t_human_id.xcenter = input.get_t_human_id( t_humanID ).xcenter;
								temp_t_human_id.ycenter = input.get_t_human_id( t_humanID ).ycenter;
								touched_object_infos.second.push_back( make_pair( temp_t_object_id, temp_t_human_id ) );
								break;
							}
						}
					}
				}
				lastEveID++;
			}
			else{	// プログラム開始から10フレーム経っていなかった場合の処理

				WaitForSingleObject( managerMutex, INFINITE );

				ev_manager newEvent;
				newEvent.eventID = lastEveID;			// イベントID			：lastEveID
				newEvent.start_frame = (long long)0;	// 保存最初				：0フレーム
				newEvent.end_frame = frame + 10;		// 保存最後				：現フレーム+10
				newEvent.end_updateRange = false;		// 保存範囲更新終了？	：false
				newEvent.fin_save = false;				// 画像保存完了？		：false

				addEvent = lastEveID;

				save_events.push_back( newEvent );

				ReleaseMutex( managerMutex );

				//WaitForSingleObject(rsvMutex, INFINITE);
				//saveRsv.push_back( std::make_pair( lastEveID, std::make_pair( (long long)0, frame + 10 ) ) );	
				//ReleaseMutex(rsvMutex);
				//WaitForSingleObject(stateMutex, INFINITE);
				//monitorEvFin[( lastEveID - monitorEvFin[0].first )].second.first ++;
				//ReleaseMutex( stateMutex );

				//物体位置と人物位置情報をGET
				touch_object_size = input.size_t_object_id();
				touched_object_infos.first = lastEveID;
				for( int t_objID = 0; t_objID < touch_object_size; t_objID++ ){
					if( input.get_t_object_id(t_objID).t_frame == frame ){
						temp_t_object_id.x1 = input.get_t_object_id( t_objID ).x1;						// 現フレームと同じフレーム番号で登録された接触情報を検索しGET
						temp_t_object_id.x2 = input.get_t_object_id( t_objID ).x2;
						temp_t_object_id.y1 = input.get_t_object_id( t_objID ).y1;
						temp_t_object_id.y2 = input.get_t_object_id( t_objID ).y2;
						temp_t_object_id.xcenter = input.get_t_object_id( t_objID ).xcenter;
						temp_t_object_id.ycenter = input.get_t_object_id( t_objID ).ycenter;
						temp_t_object_id.size = input.get_t_object_id( t_objID ).size;
						temp_t_object_id.touch = input.get_t_object_id( t_objID ).touch;
						temp_t_object_id.t_frame = input.get_t_object_id( t_objID ).t_frame;
						for( int t_humanID = 0; t_humanID < input.size_t_human_id(); t_humanID++ ){
							if( input.get_t_human_id( t_humanID ).id == temp_t_object_id.touch ){
								temp_t_human_id.x1 = input.get_t_human_id( t_humanID ).x1;				// ↑得られた物体接触情報より、接触人物情報を検索しGET
								temp_t_human_id.x2 = input.get_t_human_id( t_humanID ).x2;
								temp_t_human_id.y1 = input.get_t_human_id( t_humanID ).y1;
								temp_t_human_id.y2 = input.get_t_human_id( t_humanID ).y2;
								temp_t_human_id.xcenter = input.get_t_human_id( t_humanID ).xcenter;
								temp_t_human_id.ycenter = input.get_t_human_id( t_humanID ).ycenter;
								touched_object_infos.second.push_back( make_pair( temp_t_object_id, temp_t_human_id ) );
								break;
							}
						}
					}
				}
				lastEveID++;
			}
		}
		else if( touch_flagT1 == 1 && touch_flagT0 == 1 ){ // 接触判定が連続した
			//cout << "keep touching..." << endl;
			noTouch = 0;
			WaitForSingleObject( managerMutex, INFINITE );

			( *( save_events.end() - 1 ) ).end_frame = frame + 10;		// 保存範囲を現フレーム＋10に更新

			ReleaseMutex( managerMutex );

			//WaitForSingleObject(rsvMutex, INFINITE);
			//saveRsv.push_back( std::make_pair( lastEveID - 1, std::make_pair( frame + 10, frame + 10 ) ) );		// 10フレーム後の1枚を保存する予約
			//ReleaseMutex(rsvMutex);
			//WaitForSingleObject(stateMutex, INFINITE);
			//monitorEvFin[( ( lastEveID - 1 ) - monitorEvFin[0].first )].second.first ++;
			//ReleaseMutex( stateMutex );

			//物体位置と人物位置情報をGET
				touch_object_size = input.size_t_object_id();
				touched_object_infos.first = lastEveID - 1;
				for( int t_objID = 0; t_objID < touch_object_size; t_objID++ ){
					if( input.get_t_object_id(t_objID).t_frame == frame ){
						temp_t_object_id.x1 = input.get_t_object_id( t_objID ).x1;						// 現フレームと同じフレーム番号で登録された接触情報を検索しGET
						temp_t_object_id.x2 = input.get_t_object_id( t_objID ).x2;
						temp_t_object_id.y1 = input.get_t_object_id( t_objID ).y1;
						temp_t_object_id.y2 = input.get_t_object_id( t_objID ).y2;
						temp_t_object_id.xcenter = input.get_t_object_id( t_objID ).xcenter;
						temp_t_object_id.ycenter = input.get_t_object_id( t_objID ).ycenter;
						temp_t_object_id.size = input.get_t_object_id( t_objID ).size;
						temp_t_object_id.touch = input.get_t_object_id( t_objID ).touch;
						temp_t_object_id.t_frame = input.get_t_object_id( t_objID ).t_frame;
						for( int t_humanID = 0; t_humanID < input.size_t_human_id(); t_humanID++ ){
							if( input.get_t_human_id( t_humanID ).id == temp_t_object_id.touch ){
								temp_t_human_id.x1 = input.get_t_human_id( t_humanID ).x1;				// ↑得られた物体接触情報より、接触人物情報を検索しGET
								temp_t_human_id.x2 = input.get_t_human_id( t_humanID ).x2;
								temp_t_human_id.y1 = input.get_t_human_id( t_humanID ).y1;
								temp_t_human_id.y2 = input.get_t_human_id( t_humanID ).y2;
								temp_t_human_id.xcenter = input.get_t_human_id( t_humanID ).xcenter;
								temp_t_human_id.ycenter = input.get_t_human_id( t_humanID ).ycenter;
								touched_object_infos.second.push_back( make_pair( temp_t_object_id, temp_t_human_id ) );
								break;
							}
						}
					}
				}
		}
		else if( touch_flagT1 == 0 && touch_flagT0 == 1 ){ // 接触が終わった
			//cout << "touch end" << endl;
			noTouch++;
			doubtTouch++;
		}
		else{
			noTouch++;
			//cout << "remains no touch(" << noTouch << ")" << endl;
			if( doubtTouch > 3 ){
				doubtTouch = 0;
				WaitForSingleObject( managerMutex, INFINITE );

				( *( save_events.end() - 1 ) ).end_updateRange = true;		// イベントの必要保存範囲の更新終了

				ReleaseMutex( managerMutex );
			}
			else if( doubtTouch > 0 ){
				doubtTouch++;
			}
			else{
				doubtTouch = 0;
				if( !save_events.empty() ){
					WaitForSingleObject( managerMutex, INFINITE );

					( *( save_events.end() - 1 ) ).end_updateRange = true;		// イベントの必要保存範囲の更新終了

					ReleaseMutex( managerMutex );
				}
			}
		}
		if( noTouch > 31 ){	// 31フレーム非接触が続いた
			//cout << "No touch detected between 31 frames..." << endl;
			WaitForSingleObject( managerMutex, INFINITE );

			ev_manager newEvent;
			newEvent.eventID = -1;					// イベントID			：-1
			newEvent.start_frame = frame - 10;		// 保存最初				：現フレーム-10
			newEvent.end_frame = frame - 10;		// 保存最後				：現フレーム-10
			newEvent.end_updateRange = true;		// 保存範囲更新終了？	：true
			newEvent.fin_save = true;				// 画像保存完了？		：true

			save_events.push_back( newEvent );

			ReleaseMutex( managerMutex );
			noTouch = 0;
		}

		touch_flagT0 = touch_flagT1;

		t_output_start = timeGetTime();

		//スケルトンメモリ保存
		memSkelton.push_back( std::make_pair( frame, kinectPack.bodies.jsonBodyInfo.c_str() ) );
		num_memSkelton++;

		//画像メモリ保存
		WaitForSingleObject(saveMutex, INFINITE);																// Mutex取得　画像出力保存スレッド群とmemImgを共有
		ImgSet imgSet;
		imgSet.color = imageColorOriginal;
		imgSet.depth = imageDepthOriginal;
		imgSet.bodyindex = imageBodyIndexOriginal;
		//memImg.push_back( std::make_pair( frame, std::make_pair( imageColorOriginal, imageDepthOriginal ) ) );	// memImg配列に、フレーム番号とMat カラー画像とデプス画像 をプッシュ
		memImg.push_back( std::make_pair( frame, imgSet ) );
		num_memImgs++;
		ReleaseMutex(saveMutex);																				// Mutex解除

		int sizeRsv = saveRsv.size();

		//接触イベントIDを元に保存先の指定
		char dirPath[256];
		char touch_infoDirPath[256];
		char touch_infoFileName[256];
		if( addEvent != -1 ){
			sprintf_s( dirPath, "%s/%d", filePath, touched_object_infos.first );						//2019/05/23現在は /img/yyyymmdd_hhmmss/touch/の下に
			CreateDirectory( dirPath, NULL );															//接触イベントごとに0, 1, 2, ...(eventID)という名前のフォルダが生成され、
			sprintf_s( touch_infoDirPath, "%s/%d/touch_info", filePath, touched_object_infos.first );	//その下に、touch_infoフォルダが
			CreateDirectory( touch_infoDirPath, NULL );													//生成される
		}
		int touch_nums = touched_object_infos.second.size();
		for( int t_num = 0; t_num < touch_nums; t_num++ ){
			sprintf_s( touch_infoFileName, "%s/%08lu_%d.txt", touch_infoDirPath, frame, t_num );
			ofstream outputfile( touch_infoFileName );
			outputfile<< touched_object_infos.second[t_num].first.x1 << " "
					  << touched_object_infos.second[t_num].first.x2 << " "
					  << touched_object_infos.second[t_num].first.y1 << " "
					  << touched_object_infos.second[t_num].first.y2 << " "
					  << touched_object_infos.second[t_num].first.xcenter << " "
					  << touched_object_infos.second[t_num].first.ycenter << " "
					  << touched_object_infos.second[t_num].first.size << " "
					  << touched_object_infos.second[t_num].second.x1 << " "
					  << touched_object_infos.second[t_num].second.x2 << " "
					  << touched_object_infos.second[t_num].second.y1 << " "
					  << touched_object_infos.second[t_num].second.y2 << " "
					  << touched_object_infos.second[t_num].second.xcenter << " "
					  << touched_object_infos.second[t_num].second.ycenter << endl;
			outputfile.close();
		}
		
		//画像ファイル保存////////////////////////////////////////////////////////////////
		if( !save_events.empty() && addEvent != -1 ){		// イベント管理構造体配列が空でない、かつ、「新IDイベントが追加された」判定がtrue

			//画像保存スレッド生成
			WaitForSingleObject( managerMutex, INFINITE );
			for( auto itr = save_events.begin(); itr != save_events.end(); itr++ ){
				if( (*itr).eventID == addEvent ){
					saveThreads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)imageSaver2, (LPVOID)&( (*itr).start_frame ), 0, NULL));
					break;
				}
			}
			ReleaseMutex( managerMutex );
			int numThreads = saveThreads.size();
			addEvent = -1;									// スレッドが立つので、「新IDイベントが追加された」判定はfalseにしておく
		}

		//スケルトン保存//////////////////////////////////////////////////////////////////未完成使わない方がいい
		/*WaitForSingleObject( managerMutex, INFINITE );
		for( auto itr = save_events.begin(); itr != save_events.end(); itr++ ){
			if( (*itr).end_frame >= frame ){
				for( long long i = (*itr).start_frame; i <= (*itr).end_frame; i++ ){
			
					for( auto itr = save_events.begin(); itr != save_events.end(); itr++){	//内部値確認用
						cout << "save_events[" << itr - save_events.begin() << "]" << endl;
						cout << "eventID: " << (*itr).eventID << endl;
						cout << "start_frame: " << (*itr).start_frame << endl;
						cout << "end_frame: " << (*itr).end_frame << endl;
						cout << "end_updateRange: " << (*itr).end_updateRange << endl;
						cout << "fin_save: " << (*itr).fin_save << endl;
					}
					//cout << ( *( save_events.end() - 1 ) ).start_frame << "(start_frame)" << endl;
					char skeltonFileName[256];
					sprintf_s( skeltonFileName, "%s/%08lu.txt", filePath, i );
					ofstream outputfile( skeltonFileName );
					outputfile << memSkelton[i - memSkelton[0].first].second;
					outputfile.close();
				}
			}
		}
		ReleaseMutex( managerMutex );
		//cout << i << "(saveFrame)" << endl;
		*/

		//if( !saveRsv.empty() && frame == saveRsv[ threadState.size() ].second.second ){	// 保存予約が空でない　かつ　現在フレームが予約行列1番の終了フレーム番号と一致　なら保存スレッド生成
		//	//画像保存スレッド生成
		//	saveThreads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)imageSaver, (LPVOID)&(saveRsv[ threadState.size() ].second.first), 0, NULL));
		//	int numThreads = saveThreads.size();
		//	//スケルトン保存
		//	for( int i = 0; i < saveRsv[ threadState.size()].second.second - saveRsv[ threadState.size() ].second.first + 1; i++ ){
		//		long long saveFrame = (long long)i + saveRsv[ threadState.size() ].second.first;
		//		char skeltonFileName[256];
		//		sprintf_s( skeltonFileName, "%s/%08lu.txt", filePath, saveFrame );
		//		ofstream outputfile( skeltonFileName );
		//		outputfile << memSkelton[saveFrame - memSkelton[0].first].second;
		//		outputfile.close();
		//	}
		//	threadState.push_back(std::make_pair( saveRsv[ threadState.size() ].second.first, std::make_pair( saveThreads[numThreads - 1], false ) ) );	// スレッド生成と共に追加され各スレッドの状態を監視する
		//}

		//画像出力
		cv::Mat cap, imgY;
		msgpack::sbuffer sbuf;
		pParent->imagepack["frame"] = MultiData( Util::toString( frame ), "string", "フレーム数" );
		pParent->imagepack["frame"].layout = "now:Frame";


		timeEnd = currentTime.GetCurrentTime();
		CFileTimeSpan timeSpan;
		timeSpan = timeEnd - timeStart;
		double diffTime = timeSpan.GetTimeSpan() / 10000000.0;
		if ( diffTime > 1.f )
		{
			fps = diffFrame / diffTime;
			timeStart = timeEnd;
			diffFrame = 0;
		}
		char fpsString[128];
		sprintf_s( fpsString, "%.3lf", fps );
		pParent->imagepack["fps"] = MultiData( string( fpsString ), "string", "fps" );
		pParent->imagepack["fps"].layout = "fps:";

		t_connectDB_start = timeGetTime();

		/*//指差しの座標から物体IDを取得-------------------------------------------------------------------------2015/06/10
		if ( request.size() > 1 )
		{
		int yubisashiX, yubisashiY, yubisashiX_bottom, yubisashiX_top, yubisashiY_bottom, yubisashiY_top;
		yubisashiX = atoi( request["x"].c_str() );
		yubisashiY = atoi( request["y"].c_str() );
		cout << yubisashiX - 80 << "," << yubisashiY - 15 << endl;
		yubisashiX = 179;
		yubisashiY = 140;
		int margin = 10;//指差し位置と物体位置の誤差許容範囲
		yubisashiX_bottom = ( yubisashiX - 80 ) - margin;
		yubisashiX_top = ( yubisashiX - 80 ) + margin;
		yubisashiY_bottom = ( yubisashiY - 15 ) - margin;
		yubisashiY_top = ( yubisashiY - 15 ) + margin;
		//SQLに入れる用の文字列
		string StrYubisashiX_bottom, StrYubisashiX_top, StrYubisashiY_bottom, StrYubisashiY_top;
		StrYubisashiX_bottom = Util::toString( yubisashiX_bottom );
		StrYubisashiX_top = Util::toString( yubisashiX_top );
		StrYubisashiY_bottom = Util::toString( yubisashiY_bottom );
		StrYubisashiY_top = Util::toString( yubisashiY_top );

		string DBFrameCount, DBKinectID, DBObjectID;
		vector<string> ObjectResult,ObjectFrameCount;
		vector<string> bringInformation, takeInformation, moveInformation, drinkInformation, hideInformation, haveInformation;
		string selectObjectBring, selectObjectTake, selectObjectMove, selectObjectDrink, selectObjectHide, selectObjectHave;//selectObjectTouch
		{
		bool mysqlbool = true;

		string yubisashiSQL =
		"SELECT `FrameCount`,`ObjectIDbringin1`,`ObjectIDbringin2`,`ObjectIDbringin3`,`ObjectIDbringin4`,`ObjectIDbringin5`,`ObjectIDbringin6` FROM `maintable`"
		"WHERE (BringX1 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY1 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
		"(BringX2 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY2 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
		"(BringX3 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY3 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
		"(BringX4 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY4 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
		"(BringX5 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY5 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
		"(BringX6 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY6 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ");"
		;

		if ( mysql_query( mysql, yubisashiSQL.c_str() ) != 0 )
		{//データベース一覧表示SQL
		fprintf( stderr, "%s\n", mysql_error( mysql ) );
		mysqlbool = false;
		}
		if ( mysqlbool )
		{
		MYSQL_RES *result = mysql_store_result( mysql );

		int num_fields = mysql_num_fields( result );

		MYSQL_ROW row;
		while ( ( row = mysql_fetch_row( result ) ) )
		{
		for ( int i = 0; i < num_fields; i++ )
		{
		if ( row[i] != nullptr )
		{
		ObjectResult.push_back( string( row[i] ) );
		//printf( "%s", row[i]);
		//cout << "ObjectID" << str[0] <<":"<< i << endl;
		DBFrameCount = ObjectResult[0];
		DBObjectID = ObjectResult[i];
		}
		}
		}
		cout << "(bringFrameCount:" << DBFrameCount << ")" << ",(ObjectID:" << DBObjectID << ")" << endl;
		mysql_free_result( result );
		}
		}
		{
		bool mysqlbool = true;
		for ( int count = 1; count <= 6; count++ )
		{
		if ( count == 1 )
		{
		selectObjectBring = "`ObjectIDbringin" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectTake = "`ObjectIDtake" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectMove = "`ObjectIDmove" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectDrink = "`ObjectIDdrink" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectHide = "`ObjectIDhide" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectHave = "`ObjectIDhave" + Util::toString( count ) + "`=" + DBObjectID;
		}
		else
		{
		selectObjectBring += ",`ObjectIDbringin" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectTake += ",`ObjectIDtake" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectMove += ",`ObjectIDmove" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectDrink += ",`ObjectIDdrink" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectHide += ",`ObjectIDhide" + Util::toString( count ) + "`=" + DBObjectID;
		selectObjectHave += ",`ObjectIDhave" + Util::toString( count ) + "`=" + DBObjectID;
		}
		}
		string ObjectIDSQL =
		"SELECT FrameCount FROM MainTable WHERE "
		+ selectObjectBring + "," + selectObjectTake + "," + selectObjectMove + "," + selectObjectDrink + "," + selectObjectHide + "," + selectObjectHave +
		"FROM MainTable WHERE FrameCount = " + DBFrameCount + ";"
		;
		if ( mysql_query( mysql, ObjectIDSQL.c_str() ) != 0 )
		{//データベース一覧表示SQL
		fprintf( stderr, "%s\n", mysql_error( mysql ) );
		mysqlbool = false;
		//exit(1);
		}
		if ( mysqlbool )
		{
		MYSQL_RES *result2 = mysql_store_result( mysql );
		int num_fields2 = mysql_num_fields( result2 );

		MYSQL_ROW row2;
		while ( ( row2 = mysql_fetch_row( result2 ) ) )
		{
		for ( int i = 0; i < num_fields2; i++ )
		{
		if ( row2[i] != nullptr )
		{
		ObjectFrameCount.push_back( string( row2[i] ) );
		cout << DBFrameCount << string( row2[i] ) << endl;
		}
		}
		}

		mysql_free_result( result2 );
		}
		}
		{
		bool mysqlbool = true;

		for ( int count = 1; count <= 6; count++ )
		{

		}
		string SelectEvent;

		string EventInfoSQL =
		"SELECT *"

		" FROM MainTable WHERE "
		+ selectObjectBring + "," + selectObjectTake + "," + selectObjectMove + "," + selectObjectDrink + "," + selectObjectHide + "," + selectObjectHave +
		+ ";"
		;
		if ( mysql_query( mysql, EventInfoSQL.c_str( ) ) != 0 )
		{//データベース一覧表示SQL
		fprintf( stderr, "%s\n", mysql_error( mysql ) );
		mysqlbool = false;
		//exit(1);
		}
		if ( mysqlbool )
		{
		MYSQL_RES *result3 = mysql_store_result( mysql );
		int num_fields3 = mysql_num_fields( result3 );

		MYSQL_ROW row3;
		while ( ( row3 = mysql_fetch_row( result3 ) ) )
		{
		for ( int i = 0; i < num_fields3; i++ )
		{
		if ( row3[i] != nullptr )
		{
		ObjectFrameCount.push_back( string( row3[i] ) );
		cout << "(Event:" << string( row3[i] )<< "," << endl;
		}
		}
		cout << ")" << endl;
		}

		mysql_free_result( result3 );
		}
		}
		/*--結果をimagepackで送信
		char keyname[256] = "BringinColorImage";
		sprintf( keyname, ("../img/" + Util::CString2String(strPathDir) + "%08lld.png").c_str(), DBFrameCount );
		cv::Mat ColorImage = cv::imread( keyname, 1 );
		cv::Rect RectHuman( cv::Point( atoi( ObjectInformation[6].c_str( ) ), atoi( ObjectInformation[8].c_str( ) ) ), cv::Point( atoi( ObjectInformation[7].c_str( ) ), atoi( ObjectInformation[9].c_str( ) ) ) );
		cv::Mat ImageHuman = ColorImage( RectHuman );
		char keyname2[256] = "BringinObjectImage";
		sprintf( keyname2, "../DB/output-img/roi/roi-%05d.png", ObjectInformation[0] );
		cv::Mat ObjectImage = cv::imread( keyname2, 1 );
		imshow( "A", ColorImage );
		imshow( "B", ImageHuman );
		imshow( "C", ObjectImage );

		resultPack["senderName"] = MultiData( "Monitonig-DB", "string", "senderName" );

		resultPack["searchResult0ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image","ImageScene") ;
		resultPack["searchResult0ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
		resultPack["searchResult0ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );

		cout << DBFrameCount << ":" << ObjectInformation[10] << endl;
		resultPack["searchResult0Frame"] = MultiData( DBFrameCount, "string", "Frame" );
		resultPack["searchResult0EventType"] = MultiData( ObjectInformation[10], "string", "EventType" );
		resultPack["searchResult0SearchHit"] = MultiData( Util::toString(1), "string", "SearchHit" );
		//*/
		/*
		msgpack::sbuffer sbuf;
		msgpack::pack( sbuf, resultPack );

		zmq::message_t message_searchResult( sbuf.size() );
		memcpy( message_searchResult.data(), sbuf.data(), sbuf.size() );
		socketSearchResultSender.send( message_searchResult );
		}
		//--------------------------------------------------------------------------------------------------------*/

		//指差し位置座標受信----------------------------------------------------------------------------------2015/06/09
		zmq::message_t messageRequest;
		socketReqester.recv( &messageRequest, ZMQ_DONTWAIT );
		string strRequest = std::string( static_cast< char* >( messageRequest.data() ), messageRequest.size() );;
		map<string, string> request;
		try
		{
			msgpack::sbuffer sbuf;
			msgpack::unpacked msg;
			msgpack::unpack( &msg, reinterpret_cast< const char* >( strRequest.data() ), strRequest.size() );
			msg.get().convert( &request );
		}
		catch ( const std::exception &e )
		{
			//std::cout << "exception:" << e.what( ) << std::endl;
		}
		//	for ( map<string, string>::iterator itr = request.begin(); itr != request.end(); itr++ )
		//	{
		//cout << itr->first << ":" << itr->second << endl;
		//	}
		//-----------------------------------------------------------------------------------------------------

		//SelectSQLTest
		if ( request.size() > 1 )
		{
			cv::Mat yubisashiColor;
			int yubisashiX, yubisashiY, yubisashiX_bottom, yubisashiX_top, yubisashiY_bottom, yubisashiY_top;
			yubisashiX = atoi( request["x"].c_str() );
			yubisashiY = atoi( request["y"].c_str() );
			//yubisashiColor = Util::convertString2Mat(request["imageColor"]);
			//cv::imshow("yubisashiColor",yubisashiColor);
			cout << yubisashiX - 80 << ":" << yubisashiY - 15 << endl;
			int margin = 7;//指差し位置と物体位置の誤差許容範囲
			yubisashiX_bottom = ( yubisashiX - 80 ) - margin;
			yubisashiX_top = ( yubisashiX - 80 ) + margin;
			yubisashiY_bottom = ( yubisashiY - 15 ) - margin;
			yubisashiY_top = ( yubisashiY - 15 ) + margin;
			//SQLに入れる用の文字列
			string StrYubisashiX_bottom, StrYubisashiX_top, StrYubisashiY_bottom, StrYubisashiY_top;
			StrYubisashiX_bottom = Util::toString( yubisashiX_bottom );
			StrYubisashiX_top = Util::toString( yubisashiX_top );
			StrYubisashiY_bottom = Util::toString( yubisashiY_bottom );
			StrYubisashiY_top = Util::toString( yubisashiY_top );

			string DBFrameCount, DBKinectID, DBObjectID;
			string selectObjectBring, selectObjectTake, selectObjectMove, selectObjectDrink, selectObjectHide, selectObjectHave;//selectObjectTouch
			vector<string> ObjectResult;
			vector<string> TIME, FrameCountStr;
			vector<string> BringTime, BringFrame, BringObjectID, BringHumanID, BringHumanX, BringHumanXX, BringHumanY, BringHumanYY, BringCenterX, BringCenterY, BringX, BringXX, BringY, BringYY;
			vector<string> TakeTime, TakeFrame, TakeObjectID, TakeHumanID, TakeHumanX, TakeHumanXX, TakeHumanY, TakeHumanYY;
			vector<string> HaveTime, HaveFrame, HaveObjectID, HaveHumanID, HaveHumanX, HaveHumanXX, HaveHumanY, HaveHumanYY;
			vector<string> DrinkTime, DrinkFrame, DrinkObjectID, DrinkHumanID, DrinkHumanX, DrinkHumanXX, DrinkHumanY, DrinkHumanYY;
			vector<string> HideTime, HideFrame, HideObjectID, HideObjectIDed, HideHumanID, HideHumanX, HideHumanXX, HideHumanY, HideHumanYY;
			vector<string> MoveTime, MoveFrame, MoveObjectID, MoveHumanID, MoveHumanX, MoveHumanXX, MoveHumanY, MoveHumanYY, MoveCenterX, MoveCenterY, MoveX, MoveXX, MoveY, MoveYY;

			{
				bool mysqlbool = true;

				string yubisashiSQL =
					"SELECT "
					"`FrameCount`,`ObjectIDbringin1`,`ObjectIDbringin2`,`ObjectIDbringin3`,`ObjectIDbringin4`,`ObjectIDbringin5`,`ObjectIDbringin6` "
					"FROM `maintable`"
					"WHERE "
					"(BringX1 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY1 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
					"(BringX2 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY2 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
					"(BringX3 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY3 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
					"(BringX4 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY4 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
					"(BringX5 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY5 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ") OR"
					"(BringX6 BETWEEN " + StrYubisashiX_bottom + " AND " + StrYubisashiX_top + " AND BringCenterY6 BETWEEN " + StrYubisashiY_bottom + " AND " + StrYubisashiY_top + ");"
					;

				if ( mysql_query( mysql, yubisashiSQL.c_str() ) != 0 )
				{//データベース一覧表示SQL
					//fprintf( stderr, "%s\n", mysql_error( mysql ) );
					mysqlbool = false;
				}
				if ( mysqlbool )
				{
					MYSQL_RES *result = mysql_store_result( mysql );

					int num_fields = mysql_num_fields( result );

					MYSQL_ROW row;
					while ( ( row = mysql_fetch_row( result ) ) )
					{
						for ( int i = 0; i < num_fields; i++ )
						{
							if ( row[i] != nullptr )
							{
								ObjectResult.push_back( string( row[i] ) );
								//printf( "%s", row[i]);
								//cout << "ObjectID" << str[0] <<":"<< i << endl;
								DBFrameCount = ObjectResult[0];
								DBObjectID = string( row[i] );
							}
						}
					}
					cout << "(bringFrameCount:" << DBFrameCount << ")" << ",(ObjectID:" << DBObjectID << ")" << endl;
					mysql_free_result( result );
				}
			}

			if ( DBObjectID.size() != 0 )
			{
				{
					bool mysqlbool = true;
					for ( int count = 1; count <= 6; count++ )
					{
						if ( count == 1 )
						{
							selectObjectBring = "ObjectIDbringin" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectTake = "ObjectIDleave" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectMove = "ObjectIDmove" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectDrink = "ObjectIDdrink" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectHide = "ObjectIDhide" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectHave = "ObjectIDhave" + Util::toString( count ) + "=" + DBObjectID;
						}
						else
						{
							selectObjectBring += " OR ObjectIDbringin" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectTake += " OR ObjectIDleave" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectMove += " OR ObjectIDmove" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectDrink += " OR ObjectIDdrink" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectHide += " OR ObjectIDhide" + Util::toString( count ) + "=" + DBObjectID;
							selectObjectHave += " OR ObjectIDhave" + Util::toString( count ) + "=" + DBObjectID;
						}
					}

					for ( int count = 1; count <= 6; count++ )
					{

					}
					string SelectEvent;
					//Bring
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDbringin" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HumanIDbringin" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringHumanX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringHumanXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringHumanY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringHumanYY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringCenterX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringCenterY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",BringYY" + Util::toString( count );
					}
					//Take
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDleave" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HumanIDleave" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",TakeHumanX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",TakeHumanXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",TakeHumanY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",TakeHumanYY" + Util::toString( count );
					}
					//Have
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDhave" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HumanIDhave" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HaveHumanX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HaveHumanXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HaveHumanY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HaveHumanYY" + Util::toString( count );
					}
					//Drink
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDdrink" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HumanIDdrink" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",DrinkHumanX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",DrinkHumanXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",DrinkHumanY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",DrinkHumanYY" + Util::toString( count );
					}
					//Hide
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDhide" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDhidden" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HumanIDhide" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HideHumanX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HideHumanXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HideHumanY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HideHumanYY" + Util::toString( count );
					}
					//Move
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",ObjectIDmove" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",HumanIDmove" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveHumanX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveHumanXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveHumanY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveHumanYY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveCenterX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveCenterY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveXX" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveY" + Util::toString( count );
					}
					for ( int count = 1; count <= 6; count++ )
					{
						SelectEvent += ",MoveYY" + Util::toString( count );
					}

					

					string EventInfoSQL =
						"SELECT TIME,FrameCount"
						+ SelectEvent +
						" FROM MainTable WHERE "
						+ selectObjectBring + " OR " + selectObjectTake + " OR " + selectObjectMove + " OR " + selectObjectDrink + " OR " + selectObjectHide + " OR " + selectObjectHave +
						+";"
						;
					if ( mysql_query( mysql, EventInfoSQL.c_str() ) != 0 )
					{//データベース一覧表示SQL
						//fprintf( stderr, "%s\n", mysql_error( mysql ) );
						mysqlbool = false;
						//exit(1);
					}
					if ( mysqlbool )
					{
						MYSQL_RES *result2 = mysql_store_result( mysql );
						int num_fields2 = mysql_num_fields( result2 );
						int eventcount = 0;
						int Frame = -1;
						int HumanError = -10;
						MYSQL_ROW row2;
						while ( ( row2 = mysql_fetch_row( result2 ) ) )
						{
							for ( int i = 0; i < num_fields2; i++ )
							{
								int BringEventFlag = 1;
								int TakeEventFlag = 1;
								int HaveEventFlag = 1;
								if ( row2[i] != nullptr )
								{
									string detectTime;
									//ObjectFrameCount.push_back( string( row2[i] ) );
									//cout << "(Event:" << string( row2[i] ) << "," << i << ")" << endl;
									int cnt = 1;
									int cnt2 = 6;
									int top = 7;
									int bottom = 2;
									int EventCountRemember = eventcount;
									//TIME
									if ( i == 0 )
									{
										detectTime = string( row2[i] );
										TIME.push_back( string( row2[i] ) );
										eventcount--;
									}
									if ( i == 1 )
									{
										FrameCountStr.push_back( string( row2[i] ) );
										Frame++;
									}
									//Bring
									if ( i >= bottom && i <= top )
									{
										BringTime.push_back( detectTime );
										BringObjectID.push_back( string( row2[i] ) );
										BringFrame.push_back( FrameCountStr[Frame] );
										eventcount++;
										BringEventFlag = -1;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringHumanID.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringHumanX.push_back( string( row2[i] ) );
										BringEventFlag = 1;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringHumanXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringHumanY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringHumanYY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringCenterX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringCenterY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										BringYY.push_back( string( row2[i] ) );
									}
									//Take
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										TakeTime.push_back( detectTime );
										TakeObjectID.push_back( string( row2[i] ) );
										TakeFrame.push_back( FrameCountStr[Frame] );
										eventcount++;
										TakeEventFlag = -1;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										TakeHumanID.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										TakeHumanX.push_back( string( row2[i] ) );
										TakeEventFlag = 1;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										TakeHumanXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										TakeHumanY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										TakeHumanYY.push_back( string( row2[i] ) );
									}
									//Have
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HaveTime.push_back( detectTime );
										HaveObjectID.push_back( string( row2[i] ) );
										HaveFrame.push_back( FrameCountStr[Frame] );
										eventcount++;
										HaveEventFlag = 1;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HaveHumanID.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HaveHumanX.push_back( string( row2[i] ) );
										HaveEventFlag = 1;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HaveHumanXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HaveHumanY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HaveHumanYY.push_back( string( row2[i] ) );
									}
									//Drink
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										DrinkTime.push_back( detectTime );
										DrinkObjectID.push_back( string( row2[i] ) );
										DrinkFrame.push_back( FrameCountStr[Frame] );
										eventcount++;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										DrinkHumanID.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										DrinkHumanX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										DrinkHumanXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										DrinkHumanY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										DrinkHumanYY.push_back( string( row2[i] ) );
									}
									//Hide
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideTime.push_back( detectTime );
										HideFrame.push_back( FrameCountStr[Frame] );
										HideObjectID.push_back( string( row2[i] ) );
										eventcount++;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideObjectIDed.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideHumanID.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideHumanX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideHumanXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideHumanY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										HideHumanYY.push_back( string( row2[i] ) );
									}
									//Move
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveTime.push_back( detectTime );
										MoveObjectID.push_back( string( row2[i] ) );
										MoveFrame.push_back( FrameCountStr[Frame] );
										eventcount++;
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveHumanID.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveHumanX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveHumanXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveHumanY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveHumanYY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveCenterX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveCenterY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveXX.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveY.push_back( string( row2[i] ) );
									}
									cnt++;
									bottom = top + 1;
									top = cnt*cnt2 + 1;
									if ( i >= bottom && i <= top )
									{
										MoveYY.push_back( string( row2[i] ) );
									}
									//FrameCount
									if ( BringEventFlag == -1 )
									{
										BringHumanX.push_back( Util::toString( HumanError ) );
										BringHumanXX.push_back( Util::toString( HumanError ) );
										BringHumanY.push_back( Util::toString( HumanError ) );
										BringHumanYY.push_back( Util::toString( HumanError ) );
									}
									if ( TakeEventFlag == -1 )
									{
										TakeHumanX.push_back( Util::toString( HumanError ) );
										TakeHumanXX.push_back( Util::toString( HumanError ) );
										TakeHumanY.push_back( Util::toString( HumanError ) );
										TakeHumanYY.push_back( Util::toString( HumanError ) );
									}
									if ( HaveEventFlag == -1 )
									{
										HaveHumanX.push_back( Util::toString( HumanError ) );
										HaveHumanXX.push_back( Util::toString( HumanError ) );
										HaveHumanY.push_back( Util::toString( HumanError ) );
										HaveHumanYY.push_back( Util::toString( HumanError ) );
									}
								}
							}
						}
						mysql_free_result( result2 );
					}
				}

				int hitcount = 0;

				for ( int timecount = 0; timecount < BringTime.size(); timecount++ )
				{

					//TIME[timecount];
					//FrameCountStr[timecount];

					cout << "Bring:" << BringObjectID[timecount].size() << endl;
					cv::Mat ImageHuman;
					/*
					BringObjectID[timecount];
					BringHumanID[timecount];
					BringHumanX[timecount];
					BringHumanXX[timecount];
					BringHumanY[timecount];
					BringHumanYY[timecount];
					BringCenterX[timecount];
					BringCenterY[timecount];
					BringX[timecount];
					BringXX[timecount];
					BringY[timecount];
					BringYY[timecount];
					*/
					char keyname[256] = "BringInColorImage";
					sprintf( keyname, ( "../img/" + Util::CString2String( strPathDir ) + "/RGB/" + "%08lls.png" ).c_str(), BringFrame[timecount].c_str() );
					cv::Mat ColorImage = cv::imread( keyname, 1 );
					if ( BringHumanX.size() != 0 && BringHumanX[timecount] != Util::toString( -10 ) )
					{
						cv::Rect RectHuman( cv::Point( atoi( BringHumanX[timecount].c_str() ), atoi( BringHumanY[timecount].c_str() ) ), cv::Point( atoi( BringHumanXX[timecount].c_str() ), atoi( BringHumanYY[timecount].c_str() ) ) );
						ImageHuman = ColorImage( RectHuman );
						resultPack["searchResult0ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
						imshow( "BringB", ImageHuman );
					}
					char keyname2[256] = "BringInObjectImage";
					sprintf( keyname2, "../DB/output-img/roi/roi-%05s.png", BringObjectID[timecount].c_str() );
					cv::Mat ObjectImage = cv::imread( keyname2, 1 );

					imshow( "BringA", ColorImage );
					imshow( "BringC", ObjectImage );


					resultPack["senderName"] = MultiData( "Monitoring-DB", "string", "senderName" );
					resultPack["searchResult" + Util::toString( hitcount ) + "Time"] = MultiData( BringTime[timecount], "string", "Time" );
					resultPack["searchResult" + Util::toString( hitcount ) + "ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image", "ImageScene" );
					resultPack["searchResult" + Util::toString( hitcount ) + "ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );
					resultPack["searchResult" + Util::toString( hitcount ) + "Frame"] = MultiData( BringFrame[timecount], "string", "Frame" );
					resultPack["searchResult" + Util::toString( hitcount ) + "EventType"] = MultiData( "Bring", "string", "EventType" );
					//search hitどうしよう
					//持ち込み検知あり
					hitcount++;

				}

				for ( int timecount = 0; timecount < TakeTime.size(); timecount++ )
				{
					if ( TakeFrame.size() != 0 && TakeFrame.size() >= ( timecount + 1 ) )
					{
						cout << "Take:" << TakeObjectID[timecount].size() << endl;
						cv::Mat ImageHuman;
						/*
						TakeObjectID[count];
						TakeHumanID[count];
						TakeHumanX[count];
						TakeHumanXX[count];
						TakeHumanY[count];
						TakeHumanYY[count];
						*/

						char keyname[256] = "TakeOutColorImage";
						sprintf( keyname, ( "../img/" + Util::CString2String( strPathDir ) + "/RGB/" + "%08lls.png" ).c_str(), TakeFrame[timecount].c_str() );
						cv::Mat ColorImage = cv::imread( keyname, 1 );
						if ( TakeHumanX.size() != 0 && TakeHumanX[timecount] != Util::toString( -10 ) )
						{
							cv::Rect RectHuman( cv::Point( atoi( TakeHumanX[timecount].c_str() ), atoi( TakeHumanY[timecount].c_str() ) ), cv::Point( atoi( TakeHumanXX[timecount].c_str() ), atoi( TakeHumanYY[timecount].c_str() ) ) );
							ImageHuman = ColorImage( RectHuman );
							resultPack["searchResult0ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
						}
						char keyname2[256] = "TakeOutObjectImage";
						sprintf( keyname2, "../DB/output-img/roi/roi-%05s.png", TakeObjectID[timecount].c_str() );
						cv::Mat ObjectImage = cv::imread( keyname2, 1 );
						imshow( "TakeOutA", ColorImage );
						//imshow( "B", ImageHuman );
						imshow( "TakeOutC", ObjectImage );


						//resultPack["senderName"] = MultiData( "Monitonig-DB", "string", "senderName" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Time"] = MultiData( TakeTime[timecount], "string", "Time" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image", "ImageScene" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Frame"] = MultiData( TakeFrame[timecount], "string", "Frame" );
						resultPack["searchResult" + Util::toString( hitcount ) + "EventType"] = MultiData( "Take", "string", "EventType" );


						//持ち去り検知あり
						hitcount++;
					}
				}
				for ( int timecount = 0; timecount < HaveTime.size(); timecount++ )
				{
					if ( HaveFrame.size() != 0 && HaveFrame.size() >= ( timecount + 1 ) )
					{
						cout << "Have" << HaveObjectID[timecount].size() << endl;
						cv::Mat ImageHuman;
						/*
						HaveObjectID[count];
						HaveHumanID[count];
						HaveHumanX[count];
						HaveHumanXX[count];
						HaveHumanY[count];
						HaveHumanYY[count];
						*/
						char keyname[256] = "HaveColorImage";
						sprintf( keyname, ( "../img/" + Util::CString2String( strPathDir ) + "/RGB/" + "%08lls.png" ).c_str(), HaveFrame[timecount].c_str() );
						cv::Mat ColorImage = cv::imread( keyname, 1 );
						if ( HaveHumanX.size() != 0 && HaveHumanX[timecount] != Util::toString( -10 ) )
						{
							cv::Rect RectHuman( cv::Point( atoi( HaveHumanX[timecount].c_str() ), atoi( HaveHumanY[timecount].c_str() ) ), cv::Point( atoi( HaveHumanXX[timecount].c_str() ), atoi( HaveHumanYY[timecount].c_str() ) ) );
							ImageHuman = ColorImage( RectHuman );
							resultPack["searchResult" + Util::toString( hitcount ) + "ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
						}
						char keyname2[256] = "HaveObjectImage";
						sprintf( keyname2, "../DB/output-img/roi/roi-%05s.png", HaveObjectID[timecount].c_str() );
						cv::Mat ObjectImage = cv::imread( keyname2, 1 );
						imshow( "HaveA", ColorImage );
						//imshow( "HaveB", ImageHuman );
						imshow( "HaveC", ObjectImage );


						//resultPack["senderName"] = MultiData( "Monitonig-DB", "string", "senderName" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Time"] = MultiData( HaveTime[timecount], "string", "Time" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image", "ImageScene" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Frame"] = MultiData( HaveFrame[timecount], "string", "Frame" );
						resultPack["searchResult" + Util::toString( hitcount ) + "EventType"] = MultiData( "Have", "string", "EventType" );
						//search hitどうしよう
						//resultPack["searchResult0SearchHit"] = MultiData( Util::toString( hitcount ), "string", "SearchHit" );

						//保持検知あり
						hitcount++;
					}
				}
				for ( int timecount = 0; timecount < DrinkTime.size(); timecount++ )
				{
					if ( DrinkFrame.size() != 0 && DrinkFrame.size() >= ( timecount + 1 ) )
					{
						cv::Mat ImageHuman;
						/*
						DrinkObjectID[count];
						DrinkHumanID[count];
						DrinkHumanX[count];
						DrinkHumanXX[count];
						DrinkHumanY[count];
						DrinkHumanYY[count];
						*/
						char keyname[256] = "DrinkColorImage";
						sprintf( keyname, ( "../img/" + Util::CString2String( strPathDir ) + "/RGB/" + "%08lls.png" ).c_str(), DrinkFrame[timecount].c_str() );
						cv::Mat ColorImage = cv::imread( keyname, 1 );
						if ( DrinkHumanX.size() != 0 && DrinkHumanX.size() >= ( timecount + 1 ) )
						{
							cv::Rect RectHuman( cv::Point( atoi( DrinkHumanX[timecount].c_str() ), atoi( DrinkHumanY[timecount].c_str() ) ), cv::Point( atoi( DrinkHumanXX[timecount].c_str() ), atoi( DrinkHumanYY[timecount].c_str() ) ) );
							ImageHuman = ColorImage( RectHuman );
							resultPack["searchResult0ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
						}
						char keyname2[256] = "DrinkObjectImage";
						sprintf( keyname2, "../DB/output-img/roi/roi-%05s.png", DrinkObjectID[timecount].c_str() );
						cv::Mat ObjectImage = cv::imread( keyname2, 1 );
						imshow( "DrinkA", ColorImage );
						//imshow( "DrinkB", ImageHuman );
						imshow( "DrinkC", ObjectImage );


						//resultPack["senderName"] = MultiData( "Monitonig-DB", "string", "senderName" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Time"] = MultiData( DrinkTime[timecount], "string", "Time" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image", "ImageScene" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Frame"] = MultiData( DrinkFrame[timecount], "string", "Frame" );
						resultPack["searchResult" + Util::toString( hitcount ) + "EventType"] = MultiData( "Drink", "string", "EventType" );
						//search hitどうしよう
						//resultPack["searchResult0SearchHit"] = MultiData( Util::toString( 1 ), "string", "SearchHit" );

						//飲む検知あり
						hitcount++;
					}
				}
				for ( int timecount = 0; timecount < HideTime.size(); timecount++ )
				{
					if ( HideFrame.size() != 0 && HideFrame.size() >= ( timecount + 1 ) )
					{
						cv::Mat ImageHuman;
						/*
						HideObjectID[count];
						HideObjectIDed[count];
						HideHumanID[count];
						HideHumanX[count];
						HideHumanXX[count];
						HideHumanY[count];
						HideHumanYY[count];
						*/
						char keyname[256] = "HideColorImage";
						sprintf( keyname, ( "../img/" + Util::CString2String( strPathDir ) + "/RGB/" + "%08lls.png" ).c_str(), HideFrame[timecount].c_str() );
						cv::Mat ColorImage = cv::imread( keyname, 1 );
						if ( HideHumanX.size() != 0 && HideHumanX.size() >= ( timecount + 1 ) )
						{
							cv::Rect RectHuman( cv::Point( atoi( HideHumanX[timecount].c_str() ), atoi( HideHumanY[timecount].c_str() ) ), cv::Point( atoi( HideHumanXX[timecount].c_str() ), atoi( HideHumanYY[timecount].c_str() ) ) );
							ImageHuman = ColorImage( RectHuman );
							resultPack["searchResult" + Util::toString( hitcount ) + "ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
						}
						char keyname2[256] = "HideObjectImage";
						sprintf( keyname2, "../DB/output-img/roi/roi-%05s.png", HideObjectID[timecount].c_str() );
						cv::Mat ObjectImage = cv::imread( keyname2, 1 );
						imshow( "HideA", ColorImage );
						//imshow( "HideB", ImageHuman );
						imshow( "HideC", ObjectImage );


						//resultPack["senderName"] = MultiData( "Monitonig-DB", "string", "senderName" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Time"] = MultiData( HideTime[timecount], "string", "Time" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image", "ImageScene" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );
						//resultPack["searchResult0ImageObjectHide"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObjectHide" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Frame"] = MultiData( HideFrame[timecount], "string", "Frame" );
						resultPack["searchResult" + Util::toString( hitcount ) + "EventType"] = MultiData( "Hide", "string", "EventType" );
						//search hitどうしよう
						//resultPack["searchResult0SearchHit"] = MultiData( Util::toString( 1 ), "string", "SearchHit" );

						//隠す検知あり
						hitcount++;
					}
				}
				for ( int timecount = 0; timecount < MoveTime.size(); timecount++ )
				{
					if ( MoveFrame.size() != 0 && MoveFrame.size() >= ( timecount + 1 ) )
					{
						cv::Mat ImageHuman;
						/*
						MoveObjectID[count];
						MoveHumanID[count];
						MoveHumanX[count];
						MoveHumanXX[count];
						MoveHumanY[count];
						MoveHumanYY[count];
						MoveCenterX[count];
						MoveCenterY[count];
						MoveX[count];
						MoveXX[count];
						MoveY[count];
						MoveYY[count];
						*/
						char keyname[256] = "MoveColorImage";
						sprintf( keyname, ( "../img/" + Util::CString2String( strPathDir ) + "/RGB/" + "%08lls.png" ).c_str(), MoveFrame[timecount].c_str() );
						cv::Mat ColorImage = cv::imread( keyname, 1 );
						if ( MoveHumanX.size() != 0 && MoveHumanX.size() >= ( timecount + 1 ) )
						{
							cv::Rect RectHuman( cv::Point( atoi( MoveHumanX[timecount].c_str() ), atoi( MoveHumanY[timecount].c_str() ) ), cv::Point( atoi( MoveHumanXX[timecount].c_str() ), atoi( MoveHumanYY[timecount].c_str() ) ) );
							ImageHuman = ColorImage( RectHuman );
							resultPack["searchResult" + Util::toString( hitcount ) + "ImageHuman"] = MultiData( Util::convertMat2String( ImageHuman ), "image", "ImageHuman" );
						}
						char keyname2[256] = "MoveObjectImage";
						sprintf( keyname2, "../DB/output-img/roi/roi-%05s.png", MoveObjectID[timecount].c_str() );
						cv::Mat ObjectImage = cv::imread( keyname2, 1 );

						imshow( "MoveA", ColorImage );
						//imshow( "MoveB", ImageHuman );
						imshow( "MoveC", ObjectImage );


						//resultPack["senderName"] = MultiData( "Monitonig-DB", "string", "senderName" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Time"] = MultiData( MoveTime[timecount], "string", "Time" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageScene"] = MultiData( Util::convertMat2String( ColorImage ), "image", "ImageScene" );
						resultPack["searchResult" + Util::toString( hitcount ) + "ImageObject"] = MultiData( Util::convertMat2String( ObjectImage ), "image", "ImageObject" );
						resultPack["searchResult" + Util::toString( hitcount ) + "Frame"] = MultiData( MoveFrame[timecount], "string", "Frame" );
						resultPack["searchResult" + Util::toString( hitcount ) + "EventType"] = MultiData( "Move", "string", "EventType" );

						//移動検知あり
						hitcount++;
					}
				}

				resultPack["searchResultSearchHit"] = MultiData( Util::toString( hitcount ), "string", "SearchHit" );
			}
			msgpack::sbuffer sbuf;
			msgpack::pack( sbuf, resultPack );

			zmq::message_t message_searchResult( sbuf.size() );
			memcpy( message_searchResult.data(), sbuf.data(), sbuf.size() );
			socketSearchResultSender.send( message_searchResult );
		}

		//-------------------------------------------------------------------------------------*/

		msgpack::pack( sbuf, pParent->imagepack );
		zmq::message_t message2( sbuf.size() );
		memcpy( message2.data(), sbuf.data(), sbuf.size() );
		//send message

		/********以下の命令を実行すると1000フレーム目で処理が停止するため(強制終了ではない)一時的にコメントアウト**********/
		socket.send(message2);

		if( pParent->getMode() == MODE_ONLINE ){ cout << "MODE_ONLINE" << endl; }
		else if( pParent->getMode() == MODE_OFFLINE ){ cout << "MODE_OFFLINE" << endl; }
		else if( pParent->getMode() == MODE_NONE ){ cout << "MODE_NONE" << endl; break;}

		const char *fileName = "last_frame.txt";
 
	    std::ofstream ofs( fileName );
	    if ( !ofs )
	    {
	        cout << "file [" << fileName << "] cannot read!" << std::endl;
	        std::cin.get();
	    }
		else
		{
			ofs << to_string( frame ) << endl;
		}

		frame++;
		++diffFrame;

		//////Tlabel9 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
		//////RunningTime = Tlabel9 - Tlabel8;
		//////spanTime = ( long long int )RunningTime.GetTimeSpan();
		//////ofs_Time << spanTime << ",";
		//////ofs_Time << fps << endl;

		//古いメモリ保存画像の削除
		//if( frame > numMemHRMax){ rmvStart = true; }
		//if( rmvStart == true ){
		//	rmvFrame = memImg[0].first;
		//	memImg.erase( memImg.begin() );
		//}

		int oldestTouch = -1;
		int memCnt = 0;
		int confirmedID = -1;

		WaitForSingleObject( managerMutex, INFINITE );
		
		for( auto evX = save_events.begin(); evX != save_events.end(); evX++ ){
			if( (*evX).fin_save && (*evX).end_updateRange ){					// イベント(ID:X)の『接触検知が完全終了』かつ『スレッド画像保存完了』なら
				confirmedID = evX - save_events.begin();						// 現時点でこのイベント(ID:X)が、終了している中で最も最近のものである
				continue;														// 次のイベントをチェック
			}
			else{
				break;															// 未終了のイベントが出てきた時点で走査終了
			}
		}

		ReleaseMutex( managerMutex );

		//WaitForSingleObject(stateMutex, INFINITE);
		//for( auto m = threadState.begin(); m != threadState.end(); m++ ){				// threadの状態を走査
		//	if( (*m).second.second == true ){											// threadが終了している
		//		if( saveRsv[memCnt].second.second - saveRsv[memCnt].second.first > 3 ){	// 終了フレーム - 開始フレーム > 0 なら接触発生
		//			oldestTouch = memCnt;												// 今見ているthreadが終了している中で最も最近の接触発生を担当していた
		//			memCnt++;															// 次のthreadの状態を見に行く
		//			continue;
		//		}
		//		else{																	// 終了フレーム - 開始フレーム = 0 なら接触連続
		//			memCnt++;															// 次のthreadの状態を見に行く
		//			continue;
		//		}
		//	}
		//	else{
		//		break;																	// 未終了のthreadが出てきた時点で走査終了
		//	}
		//}
		//ReleaseMutex(stateMutex);

		//confirmedID番目のイベントの最初フレーム以前までは保存が終了しているのでそこまでメモリ画像を削除(-1のままの場合は何もしない)
		if( confirmedID != -1){
			WaitForSingleObject( saveMutex, INFINITE );

			if( ( save_events[confirmedID].start_frame - memImg[0].first ) - 1 != -1 ){		// すでに、要削除範囲の画像が存在しない場合は無視
				num_memImgs -= (int)( save_events[confirmedID].start_frame - ( memImg[1].first - (long long)1 ) );
				//cout << "num_memImgs: " << num_memImgs << "(after erase)" << endl;

				memImg.erase( memImg.begin(), memImg.begin() + ( save_events[confirmedID].start_frame - memImg[0].first ) - 1 );
			}

			ReleaseMutex(saveMutex);

			if( save_events[confirmedID].start_frame - ( memSkelton[0].first ) != 0 ){
				num_memSkelton -= (int)( save_events[confirmedID].start_frame - ( memSkelton[1].first - (long long)1 ) );

				memSkelton.erase( memSkelton.begin(), memSkelton.begin() + ( save_events[confirmedID].start_frame - memSkelton[0].first ) - 1 );
			}
		}

		//終わったイベントの管理構造体配列要素を解放
		if( confirmedID != -1 ){
			WaitForSingleObject( managerMutex, INFINITE );
			save_events.erase( save_events.begin(), save_events.begin() + confirmedID );
			ReleaseMutex( managerMutex );
		}

		////oldestTouch番目の接触発生threadの担当開始フレーム以前までは保存が終了しているのでそこまでメモリ画像を削除(-1のままの場合は何もしない)
		//if( oldestTouch != -1 ){
		//	WaitForSingleObject(saveMutex, INFINITE);
		//	/*if( memImg[0].first != memImg[ memImg.size() - 1 ].first -  (long long)( memImg.size() - 1 ) ){		//メモリ画像配列[0]番のフレーム番号が途中でおかしくなるので
		//		cout << memImg[0].first << "(memory frame 0)" << endl;												//強制的に直そうとしたが意味はなかった。
		//		memImg[0].first = memImg[ memImg.size() - 1 ].first -  (long long)( memImg.size() - 1 );
		//	}*/
		//	//cout << memImg[0].first << "(1)" << endl;
		//	
		//	if( ( threadState[oldestTouch].first - memImg[0].first ) - 1 == -1 ){
		//		//cout << "num_memImgs: " << num_memImgs << endl;
		//		//cout << "memImg.size: " << memImg.size() << endl;
		//		//cout << "oldestTouch: " << oldestTouch << endl;
		//		for( int i = 0; i < threadState.size(); i++ ){
		//			cout << "threadState[" << i << "].first: " << threadState[i].first << endl;
		//			cout << "threadState[" << i << "].second.second: " << threadState[i].second.second << endl;
		//		}
		//		for( int i = 0; i < saveRsv.size(); i++ ){
		//			cout << "saveRsv[" << i << "].second.first: " << saveRsv[i].second.first << endl;
		//			cout << "saveRsv[" << i << "].second.second: " << saveRsv[i].second.second << endl;
		//		}
		//		cout << "memImg.[begin]-[end].first: " << memImg[0].first << " - " << memImg[ memImg.size() - 1 ].first << endl;
		//	}
		//	else{
		//		cout << "◎no error" << endl;
		//		memImg.erase(memImg.begin(), memImg.begin() + ( threadState[oldestTouch].first - memImg[0].first ) - 1 );
		//		cout << "memImg.[begin]-[end].first: " << memImg[0].first << " - " << memImg[ memImg.size() - 1 ].first << endl;
		//	}



		//	num_memImgs -= (int)( threadState[oldestTouch].first - ( memImg[1].first - 1 ) );
		//	cout << "num_memImgs: " << num_memImgs << "(after erase)" << endl;
		//	ReleaseMutex(saveMutex);
		//																							//cout << memImg[0].first << "(2)" << endl;
		//																							//memImg[0].first = memImg[1].first - 1;
		//																							//cout << memImg[0].first << "(3)" << endl;
		//	if( threadState[oldestTouch].first - ( memSkelton[0].first ) == 0 ){
		//	}
		//	else{
		//		memSkelton.erase(memSkelton.begin(), memSkelton.begin() + ( threadState[oldestTouch].first - memSkelton[0].first ) - 1 );
		//	}
		//	num_memSkelton -= (int)( threadState[oldestTouch].first - ( memSkelton[1].first - 1 ) );
		//}

		////予約行列とスレッドハンドルとスレッド状態行列の解放
		//if( oldestTouch != -1 ){
		//	WaitForSingleObject(rsvMutex, INFINITE);
		//	saveRsv.erase( saveRsv.begin(), saveRsv.begin() + oldestTouch );
		//	ReleaseMutex(rsvMutex);
		//	for( memCnt = 0; memCnt <= oldestTouch; memCnt++ ){
		//		//WaitForSingleObject(stateMutex, INFINITE);
		//		//CloseHandle(threadState[memCnt].second.first);
		//		//ReleaseMutex(stateMutex);
		//	}
		//	threadState.erase( threadState.begin(), threadState.begin() + oldestTouch );
		//}

		////もし接触検知がしばらくなければ毎ループ1枚ずつ削除し、残った保存予約とスレッド状態行列も一応終わっているかを確認してから解放する
		//memCnt = -1;
		//if( noTouch > 21 ){
		//	WaitForSingleObject( saveMutex, INFINITE );
		//	/*if( memImg.size() > 11 && threadState.size() == 0 ){
		//		memImg.erase( memImg.begin(), memImg.end() - 10 );
		//		num_memImgs = 10;
		//	}
		//	else{*/
		//		memImg.erase( memImg.begin() );
		//		num_memImgs--;
		//	/*}*/
		//	
		//	/*if( memSkelton.size() > 11 && threadState.size() == 0 ){
		//		memSkelton.erase( memSkelton.begin(), memSkelton.end() - 10 );
		//		num_memSkelton = 10;
		//	}
		//	else{*/
		//		memSkelton.erase(memSkelton.begin());
		//		num_memSkelton--;
		//	/*}*/
		//	ReleaseMutex(saveMutex);
		//	if( threadState.size() > 0 && saveRsv.size() > 0 ){
		//		WaitForSingleObject( stateMutex, INFINITE );
		//		for( auto m = threadState.begin(); m != threadState.end(); m++ ){
		//			if( (*m).second.second == true ){
		//				memCnt++;
		//				continue;
		//			}
		//			else{
		//				break;
		//			}
		//		}
		//		if( memCnt != -1 ){
		//			saveRsv.erase( saveRsv.begin(), saveRsv.begin() + memCnt );
		//		}
		//		for( int i = 0; i <= memCnt; i++ ){
		//			//CloseHandle( threadState[i].second.first );

		//		}
		//		threadState.erase( threadState.begin(), threadState.begin() + memCnt );
		//		ReleaseMutex(stateMutex);
		//		cout << "complete all saving" << endl;
		//		if( noTouch > 10000 ){
		//			cout << "push any key..." << endl;
		//			char ok[10];
		//			cin >> ok;
		//		}
		//	}
		//}
		pastFrame = frame;
		
		//if( !monitorEvFin.empty() ){
		//	for( auto itr = monitorEvFin.begin(); itr != monitorEvFin.end(); itr++ ){
		//		cout << "eventID:" << (*itr).first << " pop:" << (*itr).second.first << " / fin:" << (*itr).second.second << endl;
		//	}
		//}

		////セーブが完全に終了しているイベントのフォルダにfin_save.txtを生成
		//cout << lastEveID << "(lastEveID)" << endl;
		//if( lastEveID > 2 ){
		//	for( int i = 0; i < lastEveID - 1; i++ ){
		//		if( monitorEvFin[i].second.first == monitorEvFin[i].second.second ){
		//			char finishedTextFileName[256];
		//			sprintf( finishedTextFileName, "%s/%d/fin_save.txt", filePath, i );
		//			cout << finishedTextFileName << endl;
		//			FILE *fp;
		//			fp = fopen( finishedTextFileName, "w" );
		//			fclose( fp );
		//		}
		//		else break;
		//	}
		//}
//-------------------------------------------------2018/12/14

		BYTE KeyState[256];
		GetKeyboardState( KeyState );
 
		//ここからキー判定(解像度＆モード変更)2019/11/06更新
		if( KeyState[VK_CONTROL] & 0x80 && KeyState[82] & 0x80 ){		// [Ctrl + R]キー押下時
			cout << "change resolution" << endl;
			string inputLine;
			while(1){
				cout << "input resolution ( 1 ～ 120 )" << endl;
				cin >> inputLine;
				resoInput = stoi( inputLine );
				if( resoInput <= 0 || resoInput > 120 ){
					cout << "file format error" << endl;
				}
				else{
					resoMagnification = resoInput/120.; //resoInputを120で割ったもの、解像度縮小倍率
					resoX = resoInput*16;				//画像横サイズ(基本縦横比16：9、120倍すればkinectのフルサイズ1920：1080)
					resoY = resoInput*9;				//画像縦サイズ
					cout << "accepted resolution ( " << resoX << ", " << resoY << " )" << endl;
					cv::destroyAllWindows();
					break;
				}
			}

		}
		else if( KeyState[VK_CONTROL] & 0x80 && KeyState[75] & 0x80 ){	// [Ctrl + K]キー押下時
			cout << " change receive mode " << endl;
			cout << "input number to select mode( number(int)→[X X X X](binary)→[BodyInfo BodyIndex Depth Color] )" << endl;
			cin >> receiveMode;
			cv::destroyAllWindows();
		}
		if( receiveMode % 2 ) recvColor = true;
		else recvColor = false;
		if( ( receiveMode / 2) % 2 ) recvDepth = true;
		else recvDepth = false;
		if( ( ( receiveMode / 2) / 2 ) % 2 ) recvBodyIndex = true;
		else recvBodyIndex = false;
		if( ( ( ( receiveMode / 2) / 2 ) / 2 ) % 2) recvBodyInfo = true;
		else recvBodyInfo = false;
		recvSTime = true;
		recvCReactiveTime = true;
		recvDReactiveTime = true;
		
		
		////キー受付け
		////accept request for changing resolution(press CONTROL key to change resolution)
		//	SHORT changeReso = GetKeyState(VK_CONTROL);
		//	if( changeReso & 0x80 ){
		//		

		//	}

		//	//accept request for changing mode(press SHIFT key to change mode)
		//	SHORT changeMode = GetKeyState(VK_SHIFT);
		//	if ( changeMode & 0x80 ){
		//		//change mode
		//		
		//	}
			
		//cout << "mode " << receiveMode << "( BInfo: " << recvBodyInfo << " BIndex: " << recvBodyIndex << " Depth: " << recvDepth << " Color: " << recvColor << " )" << endl;
//-------------------------------------------------

		t_end = timeGetTime();

		//cout << "recvColor time: " << (double)( t_recvDepth - t_recvColor ) << "msec" << endl;
		//cout << "recvDepth time: " << (double)( t_recvBodyIndex - t_recvDepth ) << "msec" << endl;
		//cout << "recvBodyIndex time: " << (double)( t_recvBodyInfo - t_recvBodyIndex ) << "msec" << endl;
		//cout << "recvBodyInfo time: " << (double)( t_recvSTime - t_recvBodyInfo ) << "msec" << endl;
		//cout << "recvSTime time: " << (double)( t_recvCReactiveTime - t_recvSTime ) << "msec" << endl;
		//cout << "recvCReactiveTime time: " << (double)( t_recvDReactiveTime - t_recvCReactiveTime ) << "msec" << endl;
		//cout << "recvDReactiveTime time: " << (double)( t_imageSet_start - t_recvDReactiveTime ) << "msec" << endl;
		//cout << "recv time: " << (double)( t_imageSet_start - t_recv_start ) << "msec" << endl;
		//cout << "set image time: " << (double)( t_save_start - t_imageSet_start ) << "msec" << endl;
		////cout << "saveColor time: " << (double)( t_saveDepth - t_saveColor ) << "msec" << endl;
		////cout << "saveDepth time: " << (double)( t_saveSkelton - t_saveDepth ) << "msec" << endl;
		////cout << "saveSkelton time: " << (double)( t_saveColorHR - t_saveSkelton ) << "msec" << endl;
		////cout << "saveColorHR time: " << (double)( t_detection_start - t_saveColorHR ) << "msec" << endl;
		//cout << "save time: " << (double)( t_detection_start - t_save_start ) << "msec" << endl;
		//cout << "detection time: " << (double)( t_output_start - t_detection_start ) << "msec" << endl;
		//cout << "output time: " << (double)( t_connectDB_start - t_output_start ) << "msec" << endl;
		//cout << "connectDBt time: " << (double)( t_end - t_connectDB_start ) << "msec" << endl;

	}//for

	std::cout << "recv end" << std::endl;
	pParent->GetDlgItem( IDC_OFFLINE )->EnableWindow( TRUE );

	preservation = 0;  //保存フラグの初期化

	return 0;
}

string extentionImg[3] = { "jpg", "bmp", "png" };
//--------------------------------------------------
// Name     : OffLine()
// Author   : Kazuhiro MAKI (CV-lab.)
// Updata   : 2006.07.26
// Function : BMPファイルを読み込んで処理
// Argument : なし
// return   : int型の変数
//--------------------------------------------------1
int CCamera::OffLine()
{
	initDir();
	//initImagePack();

	CFileTime programTime;
	CFileTime programStart, programProgress;
	programStart = programTime.GetCurrentTime();

	clock_t tm_start = clock();
	input.init();

	//KinectSensor kinectSensor;
	frame = 0;
	CvFont font;
	cvInitFont( &font, CV_FONT_HERSHEY_DUPLEX, 0.4, 0.4 );

	std::cout << "recv input" << std::endl;
	clock_t tm_mid_pre = clock();

	input.kinect_on = true;
	input.set_depth_size( 320, 240 );
	CFileTime currentTime;
	CFileTime timeStart, timeEnd;
	double fps = 0.0;

	zmq::context_t context( 1 );
	zmq::socket_t socket( context, ZMQ_PUSH );

	string VisualServerIP = "";
	{
		ifstream ifs( "configVisualServer.txt" );
		string line;
		for ( int i = 0; getline( ifs, line ); ++i )
		{
			if ( i >= 1 )
			{
				cout << "invaild file : configVisualServer.txt" << endl;
				return 1;
			}
			VisualServerIP = line;
		}
	}

	try
	{
		// VisualServerのIPアドレス
		//socket.connect( "tcp://10.40.1.105:8081" );
		//socket.connect( "tcp://133.19.23.114:8081" );　　決め打ちの名残
		socket.connect( VisualServerIP.c_str() );

	}
	catch ( zmq::error_t e )
	{
		cout << "conection error:" << ( string ) e.what() << endl;
	}


	string senderName = "";
	{
		ifstream ifs( "configSenderName.txt" );
		string line;
		for ( int i = 0; getline( ifs, line ); ++i )
		{
			if ( i >= 1 )
			{
				cout << "invaild file : configSenderName.txt" << endl;
				return 1;
			}
			senderName = line;
		}
	}
	assert( senderName != "" );
	pParent->imagepack["senderName"] = MultiData( senderName, "string", "" );
	cout << "path:" << pParent->getStrDirPathOfflineInput() << endl;
	string filepath = pParent->getStrDirPathOfflineInput();
	cout << "filepath:" << filepath << endl;

	long long diffFrame = 0;
	timeStart = currentTime.GetCurrentTime();

	bool isKinectFlip = true;

	while(1){
		std::cout << "kinect horizontal flip? : on(slower)[1]/off(faster)[0]" << endl;
		int flip_or_noflip;
		cin >> flip_or_noflip;
		if( flip_or_noflip == 1 ){ isKinectFlip = true; break; }
		else if( flip_or_noflip == 0 ){ isKinectFlip = false; break; }
		else{ cout << "invalid input(0 or 1): " << flip_or_noflip << endl; }
	}

	if( isKinectFlip ){
		input.copy_obj_mask_area( cv::imread( "./mask_object_area_flip.png" ) );
	}else{
		input.copy_obj_mask_area( cv::imread( "./mask_object_area.png" ) );
	}

	while ( 1 )
	{
		pParent->imagepack.clear();
		pParent->imagepack["senderName"] = MultiData( senderName, "string", "" );
		programProgress = programTime.GetCurrentTime();
		CFileTimeSpan programTimeSpan;
		programTimeSpan = programProgress - programStart;
		long long int programProgressTime;
		char operatingTime[128];

		pParent->imagepack["Event1"] = MultiData( "none", "string", "イベント履歴1ラベル" );
		programProgressTime = ( long long int )programTimeSpan.GetTimeSpan() / 10000000; //秒に直す
		sprintf_s( operatingTime, "%d", programProgressTime % 60 );
		pParent->imagepack["programTime_s"] = MultiData( string( operatingTime ), "string", "稼働時間_秒" );
		programProgressTime /= 60; //分
		sprintf_s( operatingTime, "%d", programProgressTime % 60 );
		pParent->imagepack["programTime_m"] = MultiData( string( operatingTime ), "string", "稼働時間_分" );
		programProgressTime /= 60; //時
		sprintf_s( operatingTime, "%d", programProgressTime % 24 );
		pParent->imagepack["programTime_h"] = MultiData( string( operatingTime ), "string", "稼働時間_時" );
		programProgressTime /= 24; //日
		sprintf_s( operatingTime, "%d", programProgressTime );
		pParent->imagepack["programTime_d"] = MultiData( string( operatingTime ), "string", "稼働時間_日" );


		char filenameRGB[256];
		sprintf_s( filenameRGB, "%s/RGB/%08lu.png", filepath.c_str(), frame );
		cv::Mat imageColor = cv::imread( filenameRGB );
		if ( imageColor.data == NULL )
		{
			cout << "not open file:" << filenameRGB << endl;
			break;
		}

		input.sizeWidth = ( double ) imageColor.cols / 320;
		input.sizeHeigth = ( double ) imageColor.rows / 240;



		char filenameDepth[256];
		sprintf_s( filenameDepth, "%s/Depth/%08lu.txt", filepath.c_str(), frame );
		cv::Mat imageDepth;

		ifstream ifs( filenameDepth );
		{
			if ( !ifs.is_open() )
			{
				cout << "not open file" << filenameDepth << endl;
				break;
			}
			int rows, cols;
			ifs >> cols >> rows;
			imageDepth = cv::Mat( rows, cols, CV_16UC1 );
			for ( int row = 0; row < rows; ++row )
			{
				for ( int col = 0; col < cols; ++col )
				{
					ifs >> imageDepth.at<USHORT>( row, col );
				}
			}

		}

		cv::Mat dst;
		imageColor.copyTo( dst );
		std::vector<std::vector<PII>> skeletons;
		{
			char filenameSkeleton[256];
			sprintf_s( filenameSkeleton, "%s/Skeleton/%08lu.txt", filepath.c_str(), frame );
			ifstream ifs( filenameSkeleton );
			if ( !ifs.is_open() )
			{
				cout << "not open file" << filenameSkeleton << endl;
				break;
			}
			int numSkeletons;
			ifs >> numSkeletons;
			for ( int indexSkeletons = 0; indexSkeletons < numSkeletons; ++indexSkeletons )
			{
				int indexSkeleton;
				ifs >> indexSkeleton;
				vector<PII> skeleton;

				// Kinect v1とKinect v2でスケルトン数が違うので条件分岐させる必要がある
				// 一時的にNUI_SKELETON_COUNTがコメントアウトされているが条件分岐ができたら消す
				for ( int indexSkeleton = 0; indexSkeleton < 25; ++indexSkeleton )
				{
					int x, y, z;
					ifs >> x >> y >> z;

					if ( skeleton.size() == 22 || skeleton.size() == 24 )
					{
						cv::circle( dst, cv::Point( x, y ), 1, cv::Scalar( 255, 0, 255 ), 5, 4 );
					}
					else
					{
						cv::circle( dst, cv::Point( x, y ), 3, cv::Scalar( 0, 200, 200 ), 2 );
					}

					skeleton.push_back( PII( ( int ) x, ( int ) y ) );
				}
				skeletons.push_back( skeleton );
			}
		}

		input.set_skeltons() = skeletons;
		input.depth_skelton_copyTo( dst );
		input.resize_depth();
		//kinectSensor.born_set(input);


		//TODO: image→Matv2
		imgInput = Matv2( cv::Size( 320, 240 ), CV_8UC3 );
		input.set( 320, 240 );
		//input.init();
		//input.img_original=imageColor;
		//input.img_depth=imageDepth;
		imageColor.copyTo( input.img_original );
		imageDepth.copyTo( input.img_depth );

		cv::resize( input.img_original, input.img_resize, cv::Size( 320, 240 ), cv::INTER_CUBIC );
		cv::cvtColor( imageColor, input.img_HLS, CV_BGR2HLS );

		cv::resize( imageDepth, imageDepth, cv::Size( 320, 240 ) );
		cv::resize( input.img_depth, input.img_depth, cv::Size( 320, 240 ) );


		input.img_HLS.convertTo( input.img_HLS, CV_32FC3 );
		input.img_resize.convertTo( input.img_resize, CV_8UC3 );


		//kinectSensor.get_human_id(input);//人物IDを取得

		int r, g, b;
		float L;

		for ( int y = 0; y < input.Height(); y++ )
		{
			for ( int x = 0; x<input.Width(); x++ )
			{
				USHORT id = 0;
				r = input.img_resize.data[y * input.img_resize.step + x * input.img_resize.elemSize() + 2];
				g = input.img_resize.data[y * input.img_resize.step + x * input.img_resize.elemSize() + 1];
				b = input.img_resize.data[y * input.img_resize.step + x * input.img_resize.elemSize() + 0];
				USHORT distance_16U = imageDepth.at<USHORT>( y, x ) >> 3;
				id = imageDepth.at<USHORT>( y, x )&( ( 1 << 3 ) - 1 );
				//L=input.img_HLS.data[y*input.img_HLS.step+x*input.img_HLS.elemSize()+1];
				//L=( 0.298912 * r + 0.586611 * g + 0.114478 * b );
				distance_16U = distance_16U > 4000 ? 4000 : distance_16U;
				L = ( ( cv::Point3f* )( input.img_HLS.data + input.img_HLS.step.p[0] * y ) )[x].y;
				input.set_input_B( x, y, b );

				input.set_Out_B( x, y, b );
				input.set_input_G( x, y, g );
				input.set_Out_G( x, y, g );
				input.set_input_R( x, y, r );
				input.set_Out_R( x, y, r );
				input.set_input_L( x, y, L );
				input.set_depth( x, y, distance_16U );
				input.set_human_id( x, y, id );
			}
		}
		input.output_img_copyTo( dst );

		//画像処理

		DetectionOffline( frame );
		//画像出力

		cv::Mat cap, imgY;
		msgpack::sbuffer sbuf;
		pParent->imagepack["frame"] = MultiData( Util::toString( frame ), "string", "総フレーム数" );
		pParent->imagepack["frame"].layout = "now:Frame";


		timeEnd = currentTime.GetCurrentTime();
		CFileTimeSpan timeSpan;
		timeSpan = timeEnd - timeStart;
		double diffTime = timeSpan.GetTimeSpan() / 10000000.0;
		if ( diffTime > 1.f )
		{
			fps = diffFrame / diffTime;
			timeStart = timeEnd;
			diffFrame = 0;
		}
		char fpsString[128];
		sprintf_s( fpsString, "%.3lf", fps );
		pParent->imagepack["fps"] = MultiData( string( fpsString ), "string", "fps" );
		pParent->imagepack["fps"].layout = "fps:";
		msgpack::pack( sbuf, pParent->imagepack );
		zmq::message_t message( sbuf.size() );
		memcpy( message.data(), sbuf.data(), sbuf.size() );
		// send message
		socket.send( message );


		frame++;
		++diffFrame;

	}//for
	std::cout << "recv end" << std::endl;

	preservation = 0;  //保存フラグの初期化

	pParent->GetDlgItem( IDC_OFFLINE )->EnableWindow( TRUE );
	pParent->GetDlgItem( IDC_ONLINE )->EnableWindow( TRUE );

	return 0;
}
//--------------------------------------------------
// Name     : Detection()
// Author   : Kazuhiro MAKI (CV-lab.)
// Updater	: Takayuki Ikegami (CV-lab.)
// Updata   : 2012.06.29
// Function : イベント検知(片山さんのプログラム)
// Argument : unsigned long	: frame			:現在のフレーム
// return   : int型の変数
//--------------------------------------------------
int CCamera::Detection(unsigned long frame, Connection connection, bool* touch_flag )
{
	CFileTime Tlabel1, Tlabel2, Tlabel3, Tlabel4, Tlabel5, Tlabel6, Tlabel7, Tlabel8; //TODO: FPSのためのラベル
	CFileTime programTime;
	ofstream ofs_det( "DetectionRunningTime_log.csv", ios::app );
	if ( !ofs_det.is_open() )
	{
		cout << "detection---_log.csv not open" << endl;
		return 1;
	}
	CFileTimeSpan RunningTime;
	long long int spanTime;

	//////Tlabel1 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル

	//検知範囲設定
	input.setArea( 0, 0, 320, 240 );
	//各フラッグ初期化
	Resetflag();

	//画像編集（背景差分、ラベリング、影除去、Depth情報抽出）返し値：ラベルがあれば（true）
	image_is_diff = edit.imageEdit( frame, input ); //TODO: かかった時間　イベント無い時0.05秒　あるとき?

	//////Tlabel2 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel2 - Tlabel1;
	//////spanTime = ( long long int )RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	//人物検知
	human_detection = Human.HumanSearch( frame, input );

	//////Tlabel3 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel3 - Tlabel2;
	//////spanTime = ( long long int )RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//物体候補検知(一定値のラベルを物体候補領域として確保。複数確保可能。)返し値：20フレーム同じ場所にいれば物体領域として(true)
	object_detection = object.ObjectSearch( frame, input );

	//////Tlabel4 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel4 - Tlabel3;
	//////spanTime = ( long long int )RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//イベントの数を返す。
	//std::cout << "event_flag"  << endl;
	event_flag = _event.EventSearch( frame, input, object_detection, human_detection, touch_flag );

	//////Tlabel5 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel5 - Tlabel4;
	//////spanTime = ( long long int )RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	if ( human_detection != false || event_flag )//人物候補領域の保存
	{
		//人物情報保存
		saveDATA( frame, input, human_detection/*Y,N*/, object_detection/*Y,N*/, event_flag/*イベント数*/ ); //TODO: かかった時間　たまに長時間(場合によっては0.5秒かかることも)
	}

	//////Tlabel6 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel6 - Tlabel5;
	//////spanTime = ( long long int )RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	//flag_detection(input);
	//std::cout << "OutputImage"<< endl;
	//画面に出力させる
	OutputImage2(frame, input, image_is_diff, human_detection, event_flag); //TODO: かかった時間0.05秒ぐらい
	// OutputImageOffline(frame, input, image_is_diff, human_detection, event_flag); //TODO: かかった時間0.05秒ぐらい
	input.Eventreset( 0 );
	input.Event_releace();//kawa
	input.erase_n_event_state();

	//////Tlabel7 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel7 - Tlabel6;
	//////spanTime = ( long long int )RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << endl;

	return 0;
}

int CCamera::Detection2(unsigned long frame, Connection connection, vector<map<string, string>> &detectedObj)
{
	CFileTime Tlabel1, Tlabel2, Tlabel3, Tlabel4, Tlabel5, Tlabel6, Tlabel7, Tlabel8; //TODO: FPSのためのラベル
	CFileTime programTime;

	bool touch_flag = false;	// 仮置き2019/1/10(有本)

	ofstream ofs_det("DetectionRunningTime_log.csv", ios::app);
	if (!ofs_det.is_open())
	{
		cout << "detection---_log.csv not open" << endl;
		return 1;
	}
	CFileTimeSpan RunningTime;
	long long int spanTime;

	//////Tlabel1 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル

	//検知範囲設定
	input.setArea(0, 0, 320, 240);
	//各フラッグ初期化
	Resetflag();

	//画像編集（背景差分、ラベリング、影除去、Depth情報抽出）返し値：ラベルがあれば（true）
	image_is_diff = edit.imageEdit(frame, input); //TODO: かかった時間　イベント無い時0.05秒　あるとき?

	//////Tlabel2 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel2 - Tlabel1;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	human_detection = Human.HumanSearch(frame, input);

	//////Tlabel3 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel3 - Tlabel2;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//物体候補検知(一定値のラベルを物体候補領域として確保。複数確保可能。)返し値：20フレーム同じ場所にいれば物体領域として(true)
	object_detection = object.ObjectSearch(frame, input);

	//////Tlabel4 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel4 - Tlabel3;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//イベントの数を返す。
	//std::cout << "event_flag"  << endl;
	event_flag = _event.EventSearch(frame, input, object_detection, human_detection, &touch_flag);

	//////Tlabel5 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel5 - Tlabel4;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	if (human_detection != false || event_flag)//人物候補領域の保存
	{
		//人物情報保存
		saveDATA(frame, input, human_detection/*Y,N*/, object_detection/*Y,N*/, event_flag/*イベント数*/); //TODO: かかった時間　たまに長時間(場合によっては0.5秒かかることも)
	}

	//////Tlabel6 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel6 - Tlabel5;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";
	
	/*
	for (int n = 0; n < input.Event_size(); n++){
		cout << "index: " << n << ", key: " << input.get_Event_key(n) << ", model: " << input.get_Event_model(n) << ", obj: " << input.get_Event_obj(n) << endl;
	}
	*/
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2016-10-29　実験用に追加
	// 物体認識用サーバーにカラー画像(320x240)を送り、物体認識を行わせる
	// 返ってくる値: クラス名, 位置情報(x,y,width,height), 認識を行った時刻
	// cout << "The start of communication with the object recognition server." << endl;
	string strColorImage = Util::convertMat2String2(input.img_resize);
	cv::Mat depthData = input.depth_O().clone();
	string strDepthImage = Util::convertMat2String2(depthData);
	// string strDepthImage = Util::convertMat2String2(input.img_depth);
	connection.send_recognition_server(strColorImage);
	vector<map<string, string>> recoResult;
	connection.recv_recognition_server(recoResult);
	// cout << "The End of communication with the object recognition server." << endl;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 検知されたイベント情報から物体ID及び物体情報を抽出する
	// cout << frame << endl;
	stringstream ss;
	ss << frame;
	string frameID = ss.str();

	vector<map<string, string>> eventObj;
	for (int n = 0; n < input.Event_size(); n++){
		map<string, string> eventObjTemp;
		if (input.get_Event_Eve(n) != 0) eventObjTemp["event"] = "bring";   // 持ち込み
		else                            eventObjTemp["event"] = "take";   // 持ち去り
		stringstream ss;

		ss << "";
		eventObjTemp["class_name"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		float lay = input.get_depth(input.get_Event_cx(n), input.get_Event_cy(n));
		ss << lay;
		eventObjTemp["lay"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_s_w(n);
		eventObjTemp["x"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_s_h(n);
		eventObjTemp["y"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_e_w(n) - input.get_Event_s_w(n);
		eventObjTemp["width"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_e_h(n) - input.get_Event_s_h(n);
		eventObjTemp["height"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_cx(n);
		eventObjTemp["center_x"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_cy(n);
		eventObjTemp["center_y"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_obj(n);
		eventObjTemp["id"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << frame;
		eventObjTemp["frame"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		eventObj.push_back(eventObjTemp);
	}

	bool newObjectFlag = false;
	// これまでに検出された物体の数が0個の場合は、新たに検出された物体情報を全て保存
	if (detectedObj.size() == 0){
		for (int n = 0; n < eventObj.size(); n++){
			detectedObj.push_back(eventObj[n]);
			newObjectFlag = true;
		}
	}
	/*
	// 今回検出された物体が存在しない場合
	else if (eventObj.size() == 0){
		for (int n = 0; n < detectedObj.size(); n++){
			eventObj.push_back(detectedObj[n]);
		}
	}
	*/
	// 過去、現在共に物体情報が存在している場合
	else{
		for (int n = 0; n < detectedObj.size(); n++){
			bool idCheck = false;  // 既に登録されている情報がある場合はtrue
			int index = 0;
			for (int i = 0; i < eventObj.size(); i++){
				if (eventObj[i]["id"] == detectedObj[n]["id"]){
					index = i;
					idCheck = true;
					break;
				}
			}
			if (idCheck == false){
				newObjectFlag = true;
				eventObj.push_back(detectedObj[n]);
			}
			else{
				map<string, string> eventObjTemp;
				eventObjTemp = eventObj[index];
				eventObjTemp["class_name"] = detectedObj[n]["class_name"];
				eventObj[index] = eventObjTemp;
			}
		}
		// 最終結果のコピー
		detectedObj = eventObj;
	}

	// 物体認識用サーバーから送られた情報全てと比較
	for (int i = 0; i < recoResult.size(); i++){

		stringstream ss;
		float lay = input.get_depth(
			atoi(recoResult[i]["x"].c_str()) + (atoi(recoResult[i]["width"].c_str()) / 2),
			atoi(recoResult[i]["y"].c_str()) + (atoi(recoResult[i]["height"].c_str()) / 2));
		ss << lay;
		recoResult[i]["lay"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);
	}

	// 物体認識用サーバーから送られた情報とモニタリングシステムで検出した物体情報の比較を行う
	// 仮にモニタリングシステムで検出した物体が物体認識用サーバーで検出されていなかった場合は
	// 新たな物体としてインターフェースシステムに送ることにする(未定)
	// for (int n = detectedObj.size() - 1; n > 0; n--){
	for (int n = 0; n < detectedObj.size(); n++){
		int eveX = atoi(detectedObj[n]["x"].c_str());
		int eveY = atoi(detectedObj[n]["y"].c_str());
		int eveWidth = atoi(detectedObj[n]["width"].c_str());
		int eveHeight = atoi(detectedObj[n]["height"].c_str());
		int index = -1;  // 対応付いた物体情報をインデックス
		int indexTemp = -1;
		bool correspondingFlag = false;  // 対応付いた物体情報があるかどうか
		bool multipleCheck = false;
		double bestProportion = 0;
		// double bestIntersect = -1;
		// double bestSizeDif = 76800;  // 320*240が最大値のため
		// double bestCenterDif = 400;  // 320x240のサイズの最大距離

		// 物体認識用サーバーから送られた情報全てと比較
		for (int i = 0; i < recoResult.size(); i++){
			// 既に物体IDを保持している情報とは比較を行わない
			if (recoResult[i]["id"] != ""){
				continue;
			}
			cv::Rect rect1 = cv::Rect(eveX, eveY, eveWidth, eveHeight);
			cv::Rect rect2 = cv::Rect(atoi(recoResult[i]["x"].c_str()), atoi(recoResult[i]["y"].c_str()), atoi(recoResult[i]["width"].c_str()), atoi(recoResult[i]["height"].c_str()));
			double src_x = atoi(detectedObj[n]["center_x"].c_str());
			double src_y = atoi(detectedObj[n]["center_y"].c_str());
			double dis_x = atoi(recoResult[i]["x"].c_str()) + (atoi(recoResult[i]["width"].c_str()) / 2);
			double dis_y = atoi(recoResult[i]["y"].c_str()) + (atoi(recoResult[i]["height"].c_str()) / 2);
			bool intersectFlag = false;

			cv::Rect intersect = rect1 & rect2;   // 2つの矩形の重なった領域の抽出
			if (intersect.width != 0 && intersect.height != 0)   intersectFlag = true;
			double intersectSize = intersect.width * intersect.height;
			double srcSize = rect1.width * rect1.height;
			double disSize = rect2.width * rect2.height;
			double srcProportion = intersectSize / srcSize;
			double disProportion = intersectSize / disSize;
			double multiProportion = srcProportion * disProportion;
			if (srcProportion > 0.8 && disProportion < 0.2){
				continue;
			}
			else if (disProportion > 0.8 && srcProportion < 0.2){
				continue;
			}
			if (intersectFlag == true && srcProportion > 0.2 && disProportion > 0.2 && multiProportion > bestProportion){
			// if (intersectFlag == true && srcProportion > 0.2 && disProportion > 0.2 && multiProportion > bestProportion && frameID == detectedObj[n]["frame"]){
				correspondingFlag = true;
				bestProportion = multiProportion;
				index = i;
				/*
				if (index == -1 || multipleCheck == true || detectedObj[n]["class_name"] == ""){
					index = i;
					if (detectedObj[n]["class_name"] != "") multipleCheck = true;
				}

				if (detectedObj[n]["class_name"] == ""){
					indexTemp = i;
				}
				*/
			}
			// if (intersectFlag == true){
				// double SSD = calc_SSD(input.img_resize(rect1), input.img_resize(rect2));
				// cout << SSD << endl;
			// }
		}
		if (indexTemp != -1){
			index = indexTemp;
		}

		// 対応付いた物体が存在した場合は物体認識結果を更新する
		if (correspondingFlag == true){
			map<string, string> recoResultTemp;
			recoResultTemp = recoResult[index];

			stringstream ss;
			float lay = input.get_depth(atoi(detectedObj[n]["center_x"].c_str()), atoi(detectedObj[n]["center_y"].c_str()));
			ss << lay;
			recoResultTemp["lay"] = ss.str();
			ss.str("");
			ss.clear(stringstream::goodbit); 

			recoResultTemp["id"] = detectedObj[n]["id"];
			recoResultTemp["event"] = detectedObj[n]["event"];
			recoResultTemp["x"] = detectedObj[n]["x"];
			recoResultTemp["y"] = detectedObj[n]["y"];
			recoResultTemp["width"] = detectedObj[n]["width"];
			recoResultTemp["height"] = detectedObj[n]["height"];
			recoResult[index] = recoResultTemp;

			/*
			detectedObj[n]["class_name"] = recoResult[index]["class_name"];
			*/
			map<string, string> detectedObjTemp;
			detectedObjTemp = detectedObj[n];
			detectedObjTemp["class_name"] = recoResult[index]["class_name"];
			detectedObj[n] = detectedObjTemp;
		}
		// 対応付いた物体が存在しない場合は物体認識結果に情報を追加する
		else{
			map<string, string> recoResultTemp;
			recoResultTemp["class_name"] = detectedObj[n]["class_name"];

			stringstream ss;
			float lay = input.get_depth(atoi(detectedObj[n]["center_x"].c_str()), atoi(detectedObj[n]["center_y"].c_str()));
			ss << lay;
			recoResultTemp["lay"] = ss.str();
			ss.str("");
			ss.clear(stringstream::goodbit);

			recoResultTemp["id"] = detectedObj[n]["id"];
			recoResultTemp["event"] = detectedObj[n]["event"];
			recoResultTemp["x"] = detectedObj[n]["x"];
			recoResultTemp["y"] = detectedObj[n]["y"];
			recoResultTemp["width"] = detectedObj[n]["width"];
			recoResultTemp["height"] = detectedObj[n]["height"];
			recoResultTemp["datetime"] = recoResult[0]["datetime"];
			recoResult.push_back(recoResultTemp);
		}
	}

	// 持ち去られた物体については情報を削除
	for (int n = 0; n < detectedObj.size(); n++){
		if (detectedObj[n]["event"] == "take"){
			detectedObj.erase(detectedObj.begin() + n);
		}
	}

	//flag_detection(input);
	//std::cout << "OutputImage"<< endl;
	//画面に出力させる
	OutputImage(frame, input, image_is_diff, human_detection, event_flag, recoResult); //TODO: かかった時間0.05秒ぐらい
	// OutputImageOffline(frame, input, image_is_diff, human_detection, event_flag); //TODO: かかった時間0.05秒ぐらい
	input.Eventreset(0);
	input.Event_releace();//kawa
	input.erase_n_event_state();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2016-10-29　実験用に追加
	// インターフェースシステムに視野画像、物体認識結果、物体ID、画像のフレームIDを送信する
	// cout << "The start of communication with the interface system." << endl;
	connection.recv_interface_system();
	connection.send_interface_system2(strColorImage, strDepthImage, recoResult, frame);
	// connection.send_interface_system(strColorImage, recoResult, frame);
	// cout << "The End of communication with the interface system." << endl;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//////Tlabel7 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel7 - Tlabel6;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << endl;

	return 0;
}

int CCamera::Detection3(unsigned long frame, Connection connection, vector<map<string, string>> &detectedObj, std::vector<float> layVec)
{
	CFileTime Tlabel1, Tlabel2, Tlabel3, Tlabel4, Tlabel5, Tlabel6, Tlabel7, Tlabel8; //TODO: FPSのためのラベル
	CFileTime programTime;

	bool touch_flag = false;	// 仮置き2019/1/10(有本)

	ofstream ofs_det("DetectionRunningTime_log.csv", ios::app);
	if (!ofs_det.is_open())
	{
		cout << "detection---_log.csv not open" << endl;
		return 1;
	}
	CFileTimeSpan RunningTime;
	long long int spanTime;

	//////Tlabel1 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル

	//検知範囲設定
	input.setArea(0, 0, 320, 240);
	//各フラッグ初期化
	Resetflag();

	//画像編集（背景差分、ラベリング、影除去、Depth情報抽出）返し値：ラベルがあれば（true）
	image_is_diff = edit.imageEdit(frame, input); //TODO: かかった時間　イベント無い時0.05秒　あるとき?

	//////Tlabel2 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel2 - Tlabel1;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	human_detection = Human.HumanSearch(frame, input);

	//////Tlabel3 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel3 - Tlabel2;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//物体候補検知(一定値のラベルを物体候補領域として確保。複数確保可能。)返し値：20フレーム同じ場所にいれば物体領域として(true)
	object_detection = object.ObjectSearch(frame, input);

	//////Tlabel4 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel4 - Tlabel3;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//イベントの数を返す。
	//std::cout << "event_flag"  << endl;
	event_flag = _event.EventSearch(frame, input, object_detection, human_detection, &touch_flag);

	//////Tlabel5 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel5 - Tlabel4;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	if (human_detection != false || event_flag)//人物候補領域の保存
	{
		//人物情報保存
		saveDATA(frame, input, human_detection/*Y,N*/, object_detection/*Y,N*/, event_flag/*イベント数*/); //TODO: かかった時間　たまに長時間(場合によっては0.5秒かかることも)
	}

	//////Tlabel6 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel6 - Tlabel5;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	/*
	for (int n = 0; n < input.Event_size(); n++){
	cout << "index: " << n << ", key: " << input.get_Event_key(n) << ", model: " << input.get_Event_model(n) << ", obj: " << input.get_Event_obj(n) << endl;
	}
	*/
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2016-10-29　実験用に追加
	// 物体認識用サーバーにカラー画像(320x240)を送り、物体認識を行わせる
	// 返ってくる値: クラス名, 位置情報(x,y,width,height), 認識を行った時刻
	// cout << "The start of communication with the object recognition server." << endl;
	string strColorImage = Util::convertMat2String2(input.img_resize);
	cv::Mat depthData = input.depth_O().clone();
	string strDepthImage = Util::convertMat2String2(depthData);
	// string strDepthImage = Util::convertMat2String2(input.img_depth);
	connection.send_recognition_server(strColorImage);
	vector<map<string, string>> recoResult;
	connection.recv_recognition_server(recoResult);
	// cout << "The End of communication with the object recognition server." << endl;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 検知されたイベント情報から物体ID及び物体情報を抽出する
	// cout << frame << endl;
	stringstream ss;
	ss << frame;
	string frameID = ss.str();

	vector<map<string, string>> eventObj;
	for (int n = 0; n < input.Event_size(); n++){
		map<string, string> eventObjTemp;
		if (input.get_Event_Eve(n) != 0) eventObjTemp["event"] = "bring";   // 持ち込み
		else                            eventObjTemp["event"] = "take";   // 持ち去り
		stringstream ss;

		ss << "";
		eventObjTemp["class_name"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		float lay = input.get_depth(input.get_Event_cx(n), input.get_Event_cy(n));
		ss << lay;
		eventObjTemp["lay"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_s_w(n);
		eventObjTemp["x"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_s_h(n);
		eventObjTemp["y"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_e_w(n) - input.get_Event_s_w(n);
		eventObjTemp["width"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_e_h(n) - input.get_Event_s_h(n);
		eventObjTemp["height"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_cx(n);
		eventObjTemp["center_x"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_cy(n);
		eventObjTemp["center_y"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << input.get_Event_obj(n);
		eventObjTemp["id"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		ss << frame;
		eventObjTemp["frame"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);

		eventObj.push_back(eventObjTemp);
	}

	bool newObjectFlag = false;
	// これまでに検出された物体の数が0個の場合は、新たに検出された物体情報を全て保存
	if (detectedObj.size() == 0){
		for (int n = 0; n < eventObj.size(); n++){
			detectedObj.push_back(eventObj[n]);
			newObjectFlag = true;
		}
	}
	/*
	// 今回検出された物体が存在しない場合
	else if (eventObj.size() == 0){
	for (int n = 0; n < detectedObj.size(); n++){
	eventObj.push_back(detectedObj[n]);
	}
	}
	*/
	// 過去、現在共に物体情報が存在している場合
	else{
		for (int n = 0; n < detectedObj.size(); n++){
			bool idCheck = false;  // 既に登録されている情報がある場合はtrue
			int index = 0;
			for (int i = 0; i < eventObj.size(); i++){
				if (eventObj[i]["id"] == detectedObj[n]["id"]){
					index = i;
					idCheck = true;
					break;
				}
			}
			if (idCheck == false){
				newObjectFlag = true;
				eventObj.push_back(detectedObj[n]);
			}
			else{
				map<string, string> eventObjTemp;
				eventObjTemp = eventObj[index];
				eventObjTemp["class_name"] = detectedObj[n]["class_name"];
				eventObj[index] = eventObjTemp;
			}
		}
		// 最終結果のコピー
		detectedObj = eventObj;
	}

	// 物体認識用サーバーから送られた情報全てと比較
	for (int i = 0; i < recoResult.size(); i++){

		stringstream ss;
		float lay = input.get_depth(
			atoi(recoResult[i]["x"].c_str()) + (atoi(recoResult[i]["width"].c_str()) / 2),
			atoi(recoResult[i]["y"].c_str()) + (atoi(recoResult[i]["height"].c_str()) / 2));
		ss << lay;
		recoResult[i]["lay"] = ss.str();
		ss.str("");
		ss.clear(stringstream::goodbit);
	}

	// 物体認識用サーバーから送られた情報とモニタリングシステムで検出した物体情報の比較を行う
	// 仮にモニタリングシステムで検出した物体が物体認識用サーバーで検出されていなかった場合は
	// 新たな物体としてインターフェースシステムに送ることにする(未定)
	// for (int n = detectedObj.size() - 1; n > 0; n--){
	vector<cv::Rect> rectVec;
	for (int n = 0; n < detectedObj.size(); n++){
		int eveX = atoi(detectedObj[n]["x"].c_str());
		int eveY = atoi(detectedObj[n]["y"].c_str());
		int eveWidth = atoi(detectedObj[n]["width"].c_str());
		int eveHeight = atoi(detectedObj[n]["height"].c_str());
		cv::Rect rect = cv::Rect(eveX, eveY, eveWidth, eveHeight);
		rectVec.push_back(rect);
	}
	for (int n = 0; n < detectedObj.size(); n++){
		// int eveX = atoi(detectedObj[n]["x"].c_str());
		// int eveY = atoi(detectedObj[n]["y"].c_str());
		// int eveWidth = atoi(detectedObj[n]["width"].c_str());
		// int eveHeight = atoi(detectedObj[n]["height"].c_str());
		int index = -1;  // 対応付いた物体情報をインデックス
		bool correspondingFlag = false;  // 対応付いた物体情報があるかどうか
		bool multipleCheck = false;
		double bestProportion = 0;

		// 物体認識用サーバーから送られた情報全てと比較
		for (int i = 0; i < recoResult.size(); i++){
			// 既に物体IDを保持している情報とは比較を行わない
			if (recoResult[i]["id"] != ""){
				continue;
			}
			cv::Rect rect2 = cv::Rect(atoi(recoResult[i]["x"].c_str()), atoi(recoResult[i]["y"].c_str()), atoi(recoResult[i]["width"].c_str()), atoi(recoResult[i]["height"].c_str()));
			for (int n2 = n; n2 < detectedObj.size(); n2++){
				// cv::Rect rect1 = cv::Rect(eveX, eveY, eveWidth, eveHeight);
				cv::Rect rect1 = rectVec[n2];
				bool intersectFlag = false;

				cv::Rect intersect = rect1 & rect2;   // 2つの矩形の重なった領域の抽出
				if (intersect.width != 0 && intersect.height != 0)   intersectFlag = true;
				double intersectSize = intersect.width * intersect.height;
				double srcSize = rect1.width * rect1.height;
				double disSize = rect2.width * rect2.height;
				double srcProportion = intersectSize / srcSize;
				double disProportion = intersectSize / disSize;
				double multiProportion = srcProportion * disProportion;
				if (intersectFlag == true && srcProportion > 0.2 && disProportion > 0.2 && multiProportion > bestProportion){
					if (n == n2)		correspondingFlag = true;
					else				correspondingFlag = false;
					bestProportion = multiProportion;
					index = i;
				}
			}
		}

		// 対応付いた物体が存在した場合は物体認識結果を更新する
		if (correspondingFlag == true){
			map<string, string> recoResultTemp;
			recoResultTemp = recoResult[index];

			stringstream ss;
			float lay = input.get_depth(atoi(detectedObj[n]["center_x"].c_str()), atoi(detectedObj[n]["center_y"].c_str()));
			ss << lay;
			recoResultTemp["lay"] = ss.str();
			ss.str("");
			ss.clear(stringstream::goodbit);

			recoResultTemp["id"] = detectedObj[n]["id"];
			recoResultTemp["event"] = detectedObj[n]["event"];
			recoResultTemp["x"] = detectedObj[n]["x"];
			recoResultTemp["y"] = detectedObj[n]["y"];
			recoResultTemp["width"] = detectedObj[n]["width"];
			recoResultTemp["height"] = detectedObj[n]["height"];
			recoResult[index] = recoResultTemp;

			map<string, string> detectedObjTemp;
			detectedObjTemp = detectedObj[n];
			detectedObjTemp["class_name"] = recoResult[index]["class_name"];
			detectedObj[n] = detectedObjTemp;
		}
		// 対応付いた物体が存在しない場合は物体認識結果に情報を追加する
		else{
			map<string, string> recoResultTemp;
			recoResultTemp["class_name"] = detectedObj[n]["class_name"];

			stringstream ss;
			float lay = input.get_depth(atoi(detectedObj[n]["center_x"].c_str()), atoi(detectedObj[n]["center_y"].c_str()));
			ss << lay;
			recoResultTemp["lay"] = ss.str();
			ss.str("");
			ss.clear(stringstream::goodbit);

			recoResultTemp["id"] = detectedObj[n]["id"];
			recoResultTemp["event"] = detectedObj[n]["event"];
			recoResultTemp["x"] = detectedObj[n]["x"];
			recoResultTemp["y"] = detectedObj[n]["y"];
			recoResultTemp["width"] = detectedObj[n]["width"];
			recoResultTemp["height"] = detectedObj[n]["height"];
			recoResultTemp["datetime"] = recoResult[0]["datetime"];
			recoResult.push_back(recoResultTemp);
		}
	}

	// 持ち去られた物体については情報を削除
	for (int n = 0; n < detectedObj.size(); n++){
		if (detectedObj[n]["event"] == "take"){
			detectedObj.erase(detectedObj.begin() + n);
		}
	}

	//flag_detection(input);
	//std::cout << "OutputImage"<< endl;
	//画面に出力させる
	OutputImage(frame, input, image_is_diff, human_detection, event_flag, recoResult); //TODO: かかった時間0.05秒ぐらい
	// OutputImageOffline(frame, input, image_is_diff, human_detection, event_flag); //TODO: かかった時間0.05秒ぐらい
	input.Eventreset(0);
	input.Event_releace();//kawa
	input.erase_n_event_state();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2016-10-29　実験用に追加
	// インターフェースシステムに視野画像、物体認識結果、物体ID、画像のフレームIDを送信する
	// cout << "The start of communication with the interface system." << endl;
	connection.recv_interface_system();
	connection.send_interface_system3(strColorImage, strDepthImage, recoResult, frame, layVec);
	// connection.send_interface_system2(strColorImage, strDepthImage, recoResult, frame);
	// connection.send_interface_system(strColorImage, recoResult, frame);
	// cout << "The End of communication with the interface system." << endl;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//////Tlabel7 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel7 - Tlabel6;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << endl;

	return 0;
}

double calc_SSD(cv::Mat in, cv::Mat tmp){
	int iHeight = in.rows;
	int iWidth = in.cols;
	int iHeight2 = tmp.rows;
	int iWidth2 = tmp.cols;
	int r, g, b, temp = -1, i_temp, n_temp;
	double SSD = DBL_MAX;

	for (int i = 0; i < iHeight; i++){
		for (int n = 0; n < iWidth; n++){
			//幅もしくは高さが元の画像を超えてしまう場合の処理
			if (i + iHeight2 > iHeight || n + iWidth2 > iWidth)     continue;
			for (int x = 0, SSD = 0; x < iHeight2; x++){
				for (int y = 0; y < iWidth2; y++){
					cv::Vec3b src = in.at<cv::Vec3b>(i, n);
					cv::Vec3b dis = tmp.at<cv::Vec3b>(i, n);
					r = pow((double)(src[2] - dis[2]), 2);
					g = pow((double)(src[1] - dis[1]), 2);
					b = pow((double)(src[0] - dis[0]), 2);
					SSD = SSD + r + g + b;
				}
			}
			//SSDの値が一番小さいものの値と座標を記憶させる
			if (temp < 0 || temp > SSD){
				temp = SSD;
				i_temp = i;
				n_temp = n;
			}
		}
	}
	return SSD;
}

int CCamera::DetectionOffline(unsigned long frame)
{
	CFileTime Tlabel1, Tlabel2, Tlabel3, Tlabel4, Tlabel5, Tlabel6, Tlabel7, Tlabel8; //TODO: FPSのためのラベル
	CFileTime programTime;

	bool touch_flag = false;	// 仮置き2019/1/10(有本)

	ofstream ofs_det("DetectionRunningTime_log.csv", ios::app);
	if (!ofs_det.is_open())
	{
		cout << "detection---_log.csv not open" << endl;
		return 1;
	}
	CFileTimeSpan RunningTime;
	long long int spanTime;

	//////Tlabel1 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル

	//検知範囲設定
	input.setArea(0, 0, 320, 240);
	//各フラッグ初期化
	Resetflag();

	//画像編集（背景差分、ラベリング、影除去、Depth情報抽出）返し値：ラベルがあれば（true）
	image_is_diff = edit.imageEdit(frame, input); //TODO: かかった時間　イベント無い時0.05秒　あるとき?

	//////Tlabel2 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel2 - Tlabel1;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	human_detection = Human.HumanSearch(frame, input);

	//////Tlabel3 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel3 - Tlabel2;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//物体候補検知(一定値のラベルを物体候補領域として確保。複数確保可能。)返し値：20フレーム同じ場所にいれば物体領域として(true)
	object_detection = object.ObjectSearch(frame, input);

	//////Tlabel4 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel4 - Tlabel3;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	//イベントの数を返す。
	//std::cout << "event_flag"  << endl;
	event_flag = _event.EventSearch(frame, input, object_detection, human_detection, &touch_flag);

	//////Tlabel5 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel5 - Tlabel4;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";


	if (human_detection != false || event_flag)//人物候補領域の保存
	{
		//人物情報保存
		saveDATA(frame, input, human_detection/*Y,N*/, object_detection/*Y,N*/, event_flag/*イベント数*/); //TODO: かかった時間　たまに長時間(場合によっては0.5秒かかることも)
	}

	//////Tlabel6 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel6 - Tlabel5;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << ",";

	//flag_detection(input);
	//std::cout << "OutputImage"<< endl;
	//画面に出力させる
	OutputImageOffline(frame, input, image_is_diff, human_detection, event_flag); //TODO: かかった時間0.05秒ぐらい
	input.Eventreset(0);
	input.Event_releace();//kawa
	input.erase_n_event_state();

	//////Tlabel7 = programTime.GetCurrentTime(); //TODO: FPSのためのラベル
	//////RunningTime = Tlabel7 - Tlabel6;
	//////spanTime = (long long int)RunningTime.GetTimeSpan();
	//////ofs_det << spanTime << endl;

	return 0;
}

//シーン状況のセット
//void CCamera::SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene)
//{
//	pParent->SetScene(id,frame,cx,cy,hour,minute,scene);
//}


//void CCamera::create_window(unsigned long frame){
//	char fname[125];
//	CvFont font;
//	cvInitFont (&font, CV_FONT_HERSHEY_DUPLEX, 0.5, 0.5);
//	IplImage* window = cvCreateImage(cvSize(2500, 800), 8, 3);//kawa 1600->2500
//	IplImage* png2 = cvCreateImage(cvSize(800, 600), 8, 3);
//	//IplImage* outputImage = cvCreateImage(cvSize(png->width, png->height), 8, 3);
//	IplImage* compari_j = 0;
//	IplImage* compari = cvCreateImage(cvSize(50, 50), 8, 3);
//	cvResize(png, png2);
//	cvSetImageROI(window, cvRect(0, 0, png2->width, png2->height));
//	cvCopy(png2, window);
//	cvResetImageROI(window);
//
//	//TODO:read output roi image
//	//ここで物体リストのウィンドウを生成している
//	for(int i = 0; i < (int)Obj.size(); i++){
//		char imgname[128];
//		char tmp[10];
//		sprintf_s(imgname,"../DB/output-img");
//		strcat_s(imgname,"/roi");
//		strcat_s(imgname,"/roi-");
//		sprintf_s(tmp,"%05d",i);
//		strcat_s(imgname,tmp);
//		strcat_s(imgname,".png");
//		//		std::cout<<"path:"<<imgname<<std::endl;
//		compari_j = cvLoadImage(imgname, CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH);
//		if(compari_j==NULL)std::cout<<"    No Image"<<std::endl;		
//		cvResize(compari_j, compari);
//
//		int Width = 830+(30*Obj[i])+(Obj[i]*50);
//		int Hight = (10*(Num[i]))+((Num[i]-1)*50);
//
//		cvSetImageROI(window, cvRect(Width, Hight, compari->width, compari->height));
//		cvCopy(compari, window);
//		cvResetImageROI(window);
//	}
//
//
//	sprintf_s(fname,"../input/%08d.png",frame);
//	cvSaveImage(fname,window);
//	cvReleaseImage(& window);
//	cvReleaseImage(& compari_j);
//	cvReleaseImage(& compari);
//	cvReleaseImage(& png2);
//}


//TODO:write Object.txt
void CCamera::saveDATA( unsigned long frame, image &input, int human_flag, int object_flag, int event_flag )
{
	static unsigned long hframe = 0, sframe = 0;
	static bool file_flag = false;
	//bool hflag=false;
	/*int id_num;
	if(idname.size()==0){id_num=0;}
	else if(obj_id<key_id){id_num=idname[obj_id];}
	else {id_num= key_id;}
	// 物体IDファイルへ登録		物体のキーポイント数
	ofs << key_id << "\t" <<id_num  << "\t" << endl;
	*/

	//std::ofstream ofs( "../DB/output-img/object.txt", std::ios::app );

	//for(int num=0;num<input.EventMAX();num++)
	//{
	//ofs << input.Event_key(num) << "\t" <<input.Event_obj(num)  << "\t" << endl;
	//FileOperator path;
	////TODO:write Object image(output image)
	//std::string dir_name = "../Object/" + Util::toString(input.Event_obj(num));
	//path.MakeDir(dir_name.c_str());
	//char obj_fname[128];
	//char extention[10] = "png";
	//sprintf_s(obj_fname,"../Object/%lu/%lu.%s", input.Event_obj(num) , input.Event_key(num), extention);

	//char imgname[128];
	//sprintf_s(imgname,"../DB/output-img/roi/roi-%05d.%s",input.Event_key(num), extention);
	//IplImage* compari = cvLoadImage(imgname, CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH);
	//	if(compari == NULL){
	//		std::cout << "error" << endl;
	//	}else{
	//		cvSaveImage(obj_fname,compari);
	//		cvReleaseImage(&compari);
	//	}

	//	

	//}



	//人物情報の保存

	if ( human_flag != false )
	{
		std::ostringstream dir, frame_name;
		std::string dir_name;
		FileOperator path;

		//for(int i=1;i<=input.HumanData();i++){

		//cout<<input.get_HumanTab_frame(1)<<endl;
		if ( ( input.get_HumanTab_frame( 1 ) - hframe ) > 20 )
		{
			dir << "../DB/Human-DB/Original/" << frame << "/";
			frame_name << frame;

			dir_name = "../DB/Human-DB/Original/" + frame_name.str();
			path.MakeDir( dir_name.c_str() );

			dir << "../DB/Human-DB/Process/" << frame << "/";
			dir_name = "../DB/Human-DB/Process/" + frame_name.str();
			path.MakeDir( dir_name.c_str() );

			FileOperator file;
			FILE *fl;
			fopen_s( &fl, "../DB/Human-DB/human.log", "a" );
			fprintf( fl, "%d\n", frame );
			fclose( fl );
			sframe = frame;
			file_flag = true;

		}

		char file1[126];
		char file2[126];
		if ( file_flag == true )
		{
			sprintf_s( file1, "../DB/Human-DB/Original/%lu/%08lu.%s", sframe, input.get_HumanTab_frame( 1 ), "png" );
			sprintf_s( file2, "../DB/Human-DB/Process/%lu/%08lu.%s", sframe, input.get_HumanTab_frame( 1 ), "png" );
			cv::imwrite( string( file1 ), input.Out() );//Mat型に変更
			cv::imwrite( string( file2 ), input.dOut() );
			std::ofstream ofs( "../DB/Human-DB/human2.log", std::ios::out | std::ios::trunc );

			ofs << input.get_HumanTab_frame( 1 ) << std::endl;
			hframe = frame;
			ofs.close();
		}
	}

	//}
	//create_Sign(frame,input);
	//input.EventMAX()=0;

}

/* create_Sign
* param	unsigned long	frame	:現在フレーム
int				obj_id	:オブジェクトのID
int				key_id	:keypointの数
int				num_id	:
CvScalar		color	:色情報
vector<Pair*>	Maps	:
* return void
*/


void CCamera::create_Sign( unsigned long frame, image &input )
{
	char fname[125];
	CvFont font;
	cvInitFont( &font, CV_FONT_HERSHEY_DUPLEX, 0.5, 0.5 );
	//IplImage* data = cvCreateImage(cvSize(1600, 800), 8, 3);
	int MaxID = 0;
	std::vector<int>Num;//モデルID
	std::vector<int>obj_idname;
	std::map<int, int>model_id;
	// 物体IDと識別を格納したファイルを開く
	std::ifstream ofs( "../DB/output-img/map.txt" );
	// 1行ずつ読み込み、物体ID->物体名のmapを作成
	string line;
	int t = 0;
	while ( getline( ofs, line, '\n' ) )
	{
		// タブで分割した文字列をldataへ格納
		istringstream ss( line );
		string s;
		for ( int j = 0; getline( ss, s, '\t' ); j++ )
		{
			// 物体IDとキーポイント数を抽出して格納
			if ( j == 3 ) { Num.push_back( atoi( s.c_str() ) ); }
			else if ( j == 0 ) obj_idname.push_back( atoi( s.c_str() ) );
			//else if(j == 1) //key_idname.push_back(atoi(s.c_str()));
			if ( MaxID < obj_idname[obj_idname.size() - 1] ) { MaxID = obj_idname[obj_idname.size() - 1]; }
		}
		//int KeypointId = atol(ldata[0].c_str());
		//idname[KeypointId] = ObjId;
		//for(int j = 0; j < keypoint ; j++)labels.push_back(objId);
		//id2name.insert(map<int, int>::value_type(objId, keypoint));
		model_id.insert( map<int, int>::value_type( obj_idname[t], t ) );
		t++;
	}


	cv::Mat window = cv::Mat( cvSize( 2500, 800 ), CV_8UC3 );//kawa 1600->2500
	cv::Mat png2 = cv::Mat( cvSize( 800, 600 ), CV_8UC3 );
	//IplImage* outputImage = cvCreateImage(cvSize(png->width, png->height), 8, 3);
	cv::Mat compari_j;
	cv::Mat compari = cv::Mat( cvSize( 50, 50 ), CV_8UC3 );

	cv::resize( input.img_resize, png2, png2.size() );

	//cvSetImageROI(window, Rect(0, 0, input.Width(), input.Height()));
	//window=input.img_resize(cv::Rect(0, 0, input.Width(), input.Height()));
	cv::Rect roi_rect( 0, 0, input.Width(), input.Height() );

	window = input.img_resize( roi_rect );//Window関数内に画像を出力
	//cv::rectangle(window, input.Width(), input.Height(), input.img_resize);
	//cvCopy(png2, window);
	//cvResetImageROI(window);

	//TODO:read output roi image
	//ここで物体リストのウィンドウを生成している
	for ( int i = 0; i < 10; i++ )
	{
		int outID = obj_idname.size() - i;
		char imgname[128];
		char tmp[10];
		sprintf_s( imgname, "../DB/output-img" );
		strcat_s( imgname, "/roi" );
		strcat_s( imgname, "/roi-" );
		sprintf_s( tmp, "%05d", outID );//
		strcat_s( imgname, tmp );
		strcat_s( imgname, ".png" );

		compari_j = cv::imread( string( imgname ), CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH );

		if ( compari_j.empty() )
		{
			//	std::cout<<"    No Image"<<std::endl;
		}
		cv::resize( compari_j, compari, cv::Size() );

		int Width = 830 + ( 30 * 10 - i ) + ( 10 * 50 );
		int Hight = ( 10 * ( Num[outID] ) ) + ( ( Num[i] - 1 ) * 50 );


		cv::Rect roi( Width, Hight, 50, 50 );
		window = compari( roi );

		//cvCopy(compari, window);
		//cvResetImageROI(window);
	}


	sprintf_s( fname, "../input/%08d.png", frame );
	cv::imwrite( string( fname ), window );


	//char filenameDetectImg[128];//検出した物体の画像ファイル名
	//char tmp2[10];
	////TODO:read output image(<key_id>.bmp)
	//for(;input.EventMAX();){
	//if(key_id >= 0){
	//	sprintf_s(filenameDetectImg,"../DB/output-img");
	//	strcat_s(filenameDetectImg,"/roi");
	//	strcat_s(filenameDetectImg,"/roi-");
	//	sprintf_s(tmp2,"%05d",key_id);
	//	strcat_s(filenameDetectImg,tmp2);
	//	strcat_s(filenameDetectImg,".png");
	//	compari_j = cv::imread(string(filenameDetectImg));
	//	cv::resize(compari_j, compari,cv::Size());
	//}
	//}
	//IplImage* ababa = cvCloneImage(png);
	/*int Width = 10+(10*(id3name[id2name[x]]))+((id3name[id2name[x]]-1)*50);
	int Hight = 10+(10*objid3[id2name[x]])+(objid3[id2name[x]]*100);*/

	//int Width = (data->height)+30+(30*obj_id)+(obj_id*50);

	////	int Width = 830+(30*obj_id)+(obj_id*50);
	//int Hight = (10*(num_id))+((num_id-1)*50);
	//if(Hight < 0)
	//	Hight = 10;

	///*sprintf(str,"%d",objid2[x]);
	//cvPutText (data, str, cvPoint (10+(objid2[x]*30)+(objid2[x]*50), 30), &font, CV_RGB(255,0,0));*/

	//cvShowImage("output", data);
	//cvWaitKey(30);
	//for(int i = 0; i < (int)Maps.size(); i++){
	//	cvRectangle (data, cvPoint (Maps[i]->get_x(), Maps[i]->get_y())
	//		, cvPoint (Maps[i]->get_vx(), Maps[i]->get_vy()), color, 2);
	//}
	////cvShowImage("output", data);
	////cvWaitKey(30);

	//cvRectangle (data, cvPoint (Width, Hight)
	//	, cvPoint (Width+50, Hight+50), CV_RGB(255,0,0), 2);
	////TODO:objIDwindow
	//cvShowImage("output", data);
	//cvWaitKey(1);
	//sprintf_s(filenameDetectImg,"../input/%08d.png", nframe);
	////sprintf(imgname,"./output/notrack/%05d.jpg", x);
	//cvSaveImage(filenameDetectImg,data);

	//for(int j = 0; j < (int)Maps.size() ; j++)
	//	delete Maps[j];

	//Maps.clear();

	//cvReleaseImage(& data);
	//cvReleaseImage(& compari_i);
	//cvReleaseImage(& compari);
	//key_idname.clear();
	obj_idname.clear();
	Num.clear();
	ofs.close();
}
EventInfo getDetectedObjectInfomation()
{
	ifstream ifs( "../DB/Detected-Object/object.log" );

	if ( ifs == NULL )
	{
		//	cout <<"no"<<endl;
	}
	map<int, State> mapEventInfo;
	int latestId;
	BringInObjectList bringInObjList;
	TakingOutObjectList takingOutObjList;

	string line;
	for ( int i = 0; std::getline( ifs, line ); i++ )
	{
		Util::replaceAll( line, ",", " " );
		stringstream ss( line );
		string s;
		int x1, y1, x2, y2, x3, y3, eventId, objId, frame;
		double area;
		string date;
		for ( int j = 0; ( ss >> s ); j++ )
		{
			if ( j == 0 ) frame = atoi( s.c_str() );
			else if ( j == 1 ) x1 = atoi( s.c_str() );
			else if ( j == 2 ) y1 = atoi( s.c_str() );
			else if ( j == 3 ) x2 = atoi( s.c_str() );
			else if ( j == 4 ) y2 = atoi( s.c_str() );
			else if ( j == 5 ) x3 = atoi( s.c_str() );
			else if ( j == 6 ) y3 = atoi( s.c_str() );
			else if ( j == 7 ) area = ( double ) atof( s.c_str() );
			else if ( j == 8 ) date = s;
			//eventId
			else if ( j == 9 )
			{
				eventId = atoi( s.c_str() );//kawa 
			}
			else if ( j == 10 )
			{
				objId = atoi( s.c_str() );
				latestId = objId;
			}
		}
		ObjectInfo objInfo( frame, Geometory::Point2D( x1, y1 ), Geometory::Point2D( x2, y2 ), Geometory::Point2D( x3, y3 ), area, date, eventId, objId );

		//cout << "(" << objInfo.getDetectedFrame() << "frame) " << "c(" << objInfo.getPosCenter().getX() << "," << objInfo.getPosCenter().getY() << ")"
		//	<< "[" << objInfo.getDetectedEventKind() << "]" << objInfo.getID() << endl; 
		//持ち去りなら
		if ( eventId == EVENT_TAKING_OUT )
		{
			takingOutObjList.push_back( objInfo );
			bringInObjList.erase( objId );
		}
		else if ( eventId == EVENT_BRINGING_IN || eventId == EVENT_MOVE )
		{
			bringInObjList.push_back( objInfo );
		}
		/*det.SetPixColor(x,y,255,0,255);*/
	}
	return EventInfo( takingOutObjList, bringInObjList );
}

//--------------------------------------------------
// Name     :OutputImage()
// Author   : Kawamoto (CV-lab.)
// Updata   : 2013.08.19
// Function : 画像出力
// Argument : unsigned long	: frame			:現在のフレーム
//			　image &input
//			  int image_flag
//			  int human_flag,
//			  int event_flag
// return   : int型の変数
//--------------------------------------------------

void CCamera::OutputImage(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag, vector<map<string, string>> recoResult)//アプリケーションに画像出力　
{
	string FrameNum = Util::toString( frame );
	pParent->imagepack["frame"] = MultiData( FrameNum, "string", "frame" );
	pParent->imagepack["frame"].layout = "フレーム数:";
	bool hoji_flag = false; std::vector<int> hoji_data;
	//cout<<"out"<<endl;

	cv::Mat Lv0_depth_img;
	cv::Mat Lv0_color_img;
	cv::Mat Lv1_diff_label_img;
	cv::Mat Lv2_sub_human;
	cv::Mat Lv2_sub_object;
	cv::Mat Lv3_track_human;
	cv::Mat Lv3_sub_touch_object;
	cv::Mat Lv4_style_detect;
	cv::Mat Lv4_style_human;
	cv::Mat Lv4_bring_take_event;
	cv::Mat Lv4_bring_take_event_human;
	cv::Mat Lv4_bring_take_event_object;
	cv::Mat Lv5_have_object;
	cv::Mat Lv5_have_object_human;
	cv::Mat Lv5_have_object_object;
	cv::Mat Lv5_touch_object;
	cv::Mat Lv6_hi_event;
	cv::Mat Lv6_hi_event_Scene1;
	cv::Mat Lv6_hi_event_Scene2;
	cv::Mat Lv6_hi_event_Scene3;

	//DB用変数----------------------------------------------------------
	//vector<string> TouchHumanKinectID, TouchObjectID, TouchHumanID;
	//vector<cv::Mat>TouchHumanImg, TouchObjectImg;
	vector<string> HumanID, HumanIDKinectID;
	int HumanKinectID;
	string humanInStr, humanOutStr, humanONnumStr;
	vector<cv::Mat>TouchImg;
	vector<string> ObjectName;
	vector<string> BringHumanKinectID, BringObjectID, BringHumanID, BringCenterX, BringCenterY, BringX1, BringY1, BringX2, BringY2;
	vector<string> BringHumanX, BringHumanXX, BringHumanY, BringHumanYY;
	vector<cv::Mat>BringHumanImg, BringObjectImg;
	vector<string> TakeHumanKinectID, TakeObjectID, TakeHumanID;
	vector<string> TakeHumanX, TakeHumanXX, TakeHumanY, TakeHumanYY;
	vector<cv::Mat>TakeHumanImg, TakeObjectImg;
	vector<string> HaveHumanKinectID, HaveObjectID, HaveHumanID;
	vector<string>HaveHumanX, HaveHumanXX, HaveHumanY, HaveHumanYY;
	vector<cv::Mat>HaveHumanImg, HaveObjectImg;
	vector<string> DrinkHumanKinectID, DrinkObjectID, DrinkHumanID;
	vector<string> DrinkHumanX, DrinkHumanXX, DrinkHumanY, DrinkHumanYY;
	vector<cv::Mat>DrinkHumanImg, DrinkObjectImg;
	vector<string> HideHumanKinectID, HideObjectAID, HideObjectBID, HideHumanID;
	vector<string> HideHumanX, HideHumanXX, HideHumanY, HideHumanYY;
	vector<cv::Mat>HideHumanImg, HideObjectImg, HideInObjectImg;
	vector<string> MoveHumanKinectID, MoveObjectID, MoveHumanID, MoveCenterX, MoveCenterY, MoveX1, MoveY1, MoveX2, MoveY2;
	vector<string> MoveHumanX, MoveHumanXX, MoveHumanY, MoveHumanYY;
	vector<cv::Mat>MoveHumanImg, MoveObjectBeforeImg, MoveObjectAfterImg;
	vector<string>Event1, Event2, Event3, Event4, Event5, Event6;
	//--------------------------------------------------------------------
	//DB用シーンID、シーンフレーム更新------------------------------------
	humanONnum = input.size_t_human_id();
	if ( input.size_t_human_id() >= 1 )
	{
		DBbool = true;
		sceneFrame++;
	}
	else
	{
		DBbool = false;
	}
	if ( DBbool )
	{
		if ( HumanCount > input.size_t_human_id() )
		{
			sceneID++;
			humanOUT++;
			sceneFrame = 1;
			humanOutStr = Util::toString( humanOUT );
		}
		else if ( HumanCount < input.size_t_human_id() )
		{
			sceneID++;
			humanIN++;
			sceneFrame = 1;
			humanInStr = Util::toString( humanIN );
		}
		else
		{

		}
	}
	//------------------------------------------------------------------

	//LV0viewer-------------------------------------------------骨格情報：深度情報
	cv::Vec3b pixel_value1;
	//cv::Vec3b pixel_value2;
	Lv0_depth_img = input.img_resize.clone();
	Lv0_color_img = input.img_original.clone();
	for ( int j = 0; j < input.Height(); j++ )
	{
		for ( int i = 0; i < input.Width(); i++ )
		{
			pixel_value1.val[0] = 255 * ( max( 0, ( ( int ) input.get_depth( i, j ) - DEPTH_RANGE_MIN ) ) ) / DEPTH_RANGE_MAX;
			pixel_value1.val[1] = pixel_value1.val[0];
			pixel_value1.val[2] = pixel_value1.val[0];
			/*pixel_value2.val[0]=255*(max(0,((int)input.get_depth_is_diff(i,j)-DEPTH_RANGE_MIN)))/DEPTH_RANGE_MAX;
			pixel_value2.val[1]=pixel_value2.val[0];
			pixel_value2.val[2]=pixel_value2.val[0]; */
			//depth_in.at<cv::Vec3b>(j,i)=pixel_value1;
			Lv0_depth_img.at<cv::Vec3b>( j, i ) = pixel_value1;
		}
	}


	string name_data0 = Util::toString( input.set_skeltons().size() );
	pParent->imagepack["Lv0-SkeletonNum"] = MultiData( name_data0, "string", "skeleton" );
	pParent->imagepack["Lv0-SkeletonNum"].layout = "人物骨格数:";
	pParent->imagepack["Lv0-DepthImg"] = MultiData( Util::convertMat2String( Lv0_depth_img ), "image", "深度情報" );




	//Lv1viewer-------------------------------------------------背景差分情報
	input.dOut().convertTo( Lv1_diff_label_img, CV_8UC3 );

	string name_data1 = Util::toString( input.LabelData() );
	pParent->imagepack["Lv1-DiffLabelNum"] = MultiData( name_data1, "string", "label" );
	pParent->imagepack["Lv1-DiffLabelNum"].layout = "ラベル数:";
	if ( input.LabelData() > 0 )
	{
		pParent->imagepack["Lv1-DiffLabelImg"] = MultiData( Util::convertMat2String( Lv1_diff_label_img ), "image", "ラベル情報" );
	}

	//Lv2viewer-------------------------------------------------人物・物体候補情報
	input.Out().convertTo( Lv2_sub_human, CV_8UC3 );
	int human_max = input.HumanData();
	for ( int i = 1; i <= human_max; i++ )
	{
		if ( input.get_HumanTab_skelton( i ) != -1 )
		{
			int skeleton_num = input.get_HumanTab_skelton( i );
			for ( int indexJoint = 0; indexJoint < input.set_skeltons()[skeleton_num].size(); indexJoint++ )
			{
				int x = input.set_skeltons()[skeleton_num][indexJoint].first;
				int y = input.set_skeltons()[skeleton_num][indexJoint].second;
				cv::circle( Lv2_sub_human, cv::Point( x, y ), 1, cv::Scalar( 122, 122, 122 ), 2, 4 );
			}
		}
		pParent->imagepack["Lv2-SubHumanImg"] = MultiData( Util::convertMat2String( Lv2_sub_human ), "image", "人物候補情報" );
	}
	string name_data2_hum = Util::toString( human_max );
	pParent->imagepack["Lv2-SubHumanNum"] = MultiData( name_data2_hum, "string", "Lv2:SubHumanNum" );
	pParent->imagepack["Lv2-SubHumanNum"].layout = "人物候補数:";

	Lv2_sub_object = input.img_resize.clone();
	for ( int t = 1; t <= input.ObjectData(); t++ )
	{
		stringstream ss;
		ss << "num:" << t;
		//cv::putText(inputO, ss.str(),cv::Point(input.get_ObjectTab_s_w(t)-10,input.get_ObjectTab_s_h(t)) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 2, CV_AA); 
		cv::rectangle( Lv2_sub_object, cv::Rect( input.get_ObjectTab_s_w( t ), input.get_ObjectTab_s_h( t ), input.get_ObjectTab_e_w( t ) - input.get_ObjectTab_s_w( t ), input.get_ObjectTab_e_h( t ) - input.get_ObjectTab_s_h( t ) ), cv::Scalar( 0, 0, 255 ) );
		stringstream aa;
		aa << "cnt:" << input.get_ObjectTab_count( t );
		int red = input.get_ObjectTab_count( t ) == 20 ? 100 : input.get_ObjectTab_count( t ) * 155 / 20 + 100;
		int green = input.get_ObjectTab_count( t ) == 20 ? 255 : 100 + input.get_ObjectTab_count( t ) * 100 / 20;
		int blue = input.get_ObjectTab_count( t ) == 20 ? 255 : 100 + input.get_ObjectTab_count( t ) * 100 / 20;
		cv::putText( Lv2_sub_object, aa.str(), cv::Point( input.get_ObjectTab_s_w( t ), input.get_ObjectTab_e_h( t ) + 10 ) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar( red, green, blue ), 2, CV_AA );

		pParent->imagepack["Lv2-SubObjectImg"] = MultiData( Util::convertMat2String( Lv2_sub_object ), "image", "物体候補情報" );
	}
	string name_data2_obj = Util::toString( human_max );

	pParent->imagepack["Lv2-SubObjectNum"] = MultiData( name_data2_obj, "string", "Lv2:SubObjectNum" );
	pParent->imagepack["Lv2-SubObjectNum"].layout = "物体候補数:";


	//Lv3viewer-------------------------------------------------人物追跡・接触候補検知
	human_max = input.size_t_human_id();
	Lv3_track_human = input.img_resize.clone();
	for ( int i = 0; i < 10; i++ )
	{
		if ( i < input.size_t_human_id() )
		{

			int x1 = input.get_t_human_id( i ).x1;
			int x2 = input.get_t_human_id( i ).x2;
			int y1 = input.get_t_human_id( i ).y1;
			int y2 = input.get_t_human_id( i ).y2;
			cv::rectangle( Lv3_track_human, cv::Point( x1, y1 ), cv::Point( x2, y2 ), cv::Scalar( 0, 152, 243 ), 1, 4 );
			stringstream aa;
			aa << "ID:" << input.get_t_human_id( i ).id;//<<":"<<input.get_t_human_id(i).skeletons.size();
			cv::putText( Lv3_track_human, aa.str(), cv::Point( x1, y1 + 10 ) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar( 255, 0, 0 ), 2, CV_AA );

			string key = "Lv3-TrackHuman_" + Util::toString( i + 1 );
			string name_data = Util::toString( input.get_t_human_id( i ).id );
			pParent->imagepack[key] = MultiData( name_data, "string", key );
			pParent->imagepack[key].layout = "人物ID:";
			//DBHumanID,
			HumanID.push_back( Util::toString( input.get_t_human_id( i ).id ) );
			HumanIDKinectID.push_back( Util::toString( input.get_t_human_id( i ).kinect_id ) );
			HumanKinectID = input.get_t_human_id( i ).kinect_id;

			key.clear();
			pParent->imagepack["Lv3-TrackHumanImg"] = MultiData( Util::convertMat2String( Lv3_track_human ), "image", "人物追跡情報" );

		}
		else
		{
			string key = "Lv3-TrackHuman_" + Util::toString( i + 1 );
			pParent->imagepack[key] = MultiData( "NULL", "string", key );
			pParent->imagepack[key].layout = " ";
			key.clear();
		}
	}



	Lv3_sub_touch_object = input.img_resize.clone();
	for ( int i = 0; i < input.Event_size(); i++ )
	{
		int num = -1;
		for ( int t = 0; t < input.size_t_human_id(); t++ ) { if ( input.get_Event_human( i ) == input.get_t_human_id( t ).id ) { num = t; } }
		if ( num != -1 )
		{
			int x1 = input.get_t_human_id( num ).x1;
			int x2 = input.get_t_human_id( num ).x2;
			int y1 = input.get_t_human_id( num ).y1;
			int y2 = input.get_t_human_id( num ).y2;
			cv::rectangle( Lv3_sub_touch_object, cv::Point( x1, y1 ), cv::Point( x2, y2 ), cv::Scalar( 0, 152, 243 ), 2, 4 );
		}
		int x1 = input.get_Event_s_w( i );
		int x2 = input.get_Event_e_w( i );
		int y1 = input.get_Event_s_h( i );
		int y2 = input.get_Event_e_h( i );
		cv::rectangle( Lv3_sub_touch_object, cv::Point( x1, y1 ), cv::Point( x2, y2 ), cv::Scalar( 0, 152, 243 ), 2, 4 );
		pParent->imagepack["Lv3-SubTouchObjectImg"] = MultiData( Util::convertMat2String( Lv3_sub_touch_object ), "image", "候補接触情報" );
	}


	//cout<<"LV4"<<endl;
	//Lv4viewer-------------------------------------------------持ち込み持ち去り・姿勢検知検知
	Lv4_bring_take_event = input.img_resize.clone();

	for (int i = 0; i < input.Event_size(); i++)
	{
		Lv4_bring_take_event_human = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));
		Lv4_bring_take_event_object = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));

		//string keyString = "Event"+Util::toString( i+1 );
		//string keyString_label = "イベント履歴"+Util::toString( i+1 )+"ラベル";
		//人物IDが物体IDを
		int num = -1;
		int HumanKinectID = 1;
		human_max = input.size_t_human_id();
		for (int t = 0; t < human_max; t++)
		{
			if (input.get_Event_human(i) == input.get_t_human_id(t).id)
			{
				//DB
				if (input.get_t_human_id(t).kinect_id == -1)
				{
					HumanKinectID = 1;
				}
				else
				{
					HumanKinectID = input.get_t_human_id(t).kinect_id;
				}
				num = t;
			}
		}
		if (num != -1)
		{
			int x1 = input.get_t_human_id(num).x1;
			int x2 = input.get_t_human_id(num).x2;
			int y1 = input.get_t_human_id(num).y1;
			int y2 = input.get_t_human_id(num).y2;
			int cx = input.get_t_human_id(num).xcenter;
			int cy = input.get_t_human_id(num).ycenter;
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);
			cv::Mat a(input.img_resize, cv::Rect(x1, y1, x2 - x1, cy - y1));
			Lv4_bring_take_event_human = a.clone();
			BringHumanX.push_back(Util::toString(x1));
			BringHumanXX.push_back(Util::toString(x2));
			BringHumanY.push_back(Util::toString(y1));
			BringHumanYY.push_back(Util::toString(y2));
			TakeHumanX.push_back(Util::toString(x1));
			TakeHumanXX.push_back(Util::toString(x2));
			TakeHumanY.push_back(Util::toString(y1));
			TakeHumanYY.push_back(Util::toString(y2));
		}


		int x1 = input.get_Event_s_w(i);
		int x2 = input.get_Event_e_w(i);
		int y1 = input.get_Event_s_h(i);
		int y2 = input.get_Event_e_h(i);
		int cx = input.get_Event_cx(i);
		int cy = input.get_Event_cy(i);
		cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);



		stringstream aa;
		// aa << "ID:" << input.get_Event_key(i);//<<":"<<input.get_t_human_id(i).skeletons.size();
		aa << "ID:" << input.get_Event_obj(i);//<<":"<<input.get_t_human_id(i).skeletons.size();
		// aa << "ID:" << input.get_Event_human( i );//<<":"<<input.get_t_human_id(i).skeletons.size();
		cv::putText(Lv4_bring_take_event, aa.str(), cv::Point(x1, y1 + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2, CV_AA);

		string key = "Lv4-BringTakeEvent_" + Util::toString(i + 1);
		string event_key = key;//+"_Event";
		string object_key = key + "_Object";
		string human_key = key + "_Human";
		string human_keyimg = human_key + "Img";
		string object_keyimg = object_key + "Img";
		if (input.get_Event_Eve(i) != 0)
		{
			x1 = input.get_Event_s_w(i);
			x2 = input.get_Event_e_w(i);
			y1 = input.get_Event_s_h(i);
			y2 = input.get_Event_e_h(i);
			cx = input.get_Event_cx(i);
			cy = input.get_Event_cy(i);
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 2, 4);

			char keyname[128];
			sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_Event_model(i));
			Lv4_bring_take_event_object = cv::imread(keyname);

			string name_data = "bring";
			pParent->imagepack[event_key] = MultiData(name_data, "string", "bring");
			name_data = Util::toString(input.get_Event_human(i));
			pParent->imagepack[human_key] = MultiData(name_data, "string", "bring");
			name_data = Util::toString(input.get_Event_key(i));
			pParent->imagepack[object_key] = MultiData(name_data, "string", "bring");

			pParent->imagepack[human_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_human), "image", "bring");
			pParent->imagepack[object_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_object), "image", "bring");
			//string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を持ち込み";
			//pParent->imagepack[ keyString ] = MultiData( name_data, "string", "持ち込み" );


			//if(input.get_Event_Eve(i)==2 )//移動検知
			//{
			//	//検知画像出力領域
			//    string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を移動";
			//	pParent->imagepack[ keyString ] = MultiData(name_data, "string",  "移動"  );
			//}
			/*else
			{
			string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を持ち込み";
			pParent->imagepack[ keyString ] = MultiData( name_data, "string",  "持ち込み"  );
			}*/

			//DB-Lv4-----持ち込み物体画像、持ち込み人物ID、持ち込み物体ID、持ち込み物体位置
			if (HumanKinectID == 1)
			{
				Event1.push_back("Bring");
			}
			if (HumanKinectID == 2)
			{
				Event2.push_back("Bring");
			}
			if (HumanKinectID == 3)
			{
				Event3.push_back("Bring");
			}
			if (HumanKinectID == 4)
			{
				Event4.push_back("Bring");
			}
			if (HumanKinectID == 5)
			{
				Event5.push_back("Bring");
			}
			if (HumanKinectID == 6)
			{
				Event6.push_back("Bring");
			}
			BringHumanKinectID.push_back(Util::toString(HumanKinectID));
			// BringObjectID.push_back(Util::toString(input.get_Event_key(i)));
			BringObjectID.push_back(Util::toString(input.get_Event_obj(i)));

			bool checkFlag = false;
			string objID = Util::toString(input.get_Event_obj(i));
			for (int n = 0; n < recoResult.size(); n++){
				if (objID == recoResult[n]["id"]){
					checkFlag = true;
					ObjectName.push_back(recoResult[n]["class_name"]);
					break;
				}
			}
			if (checkFlag == false)   ObjectName.push_back("unknown");

			BringHumanID.push_back(Util::toString(input.get_Event_human(i)));
			BringCenterX.push_back(Util::toString(cx));
			BringCenterY.push_back(Util::toString(cy));
			BringX1.push_back(Util::toString(x1));
			BringX2.push_back(Util::toString(x2));
			BringY1.push_back(Util::toString(y1));
			BringY2.push_back(Util::toString(y2));

			BringHumanImg.push_back(Lv4_bring_take_event_human);
			BringObjectImg.push_back(Lv4_bring_take_event_object);

		}
		else
		{
			x1 = input.get_Event_s_w(i);
			x2 = input.get_Event_e_w(i);
			y1 = input.get_Event_s_h(i);
			y2 = input.get_Event_e_h(i);
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);

			char keyname[128];
			sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_Event_model(i));
			Lv4_bring_take_event_object = cv::imread(keyname, 1);

			string name_data = "take";
			pParent->imagepack[event_key] = MultiData(name_data, "string", "take");
			name_data = Util::toString(input.get_Event_human(i));
			pParent->imagepack[human_key] = MultiData(name_data, "string", "take");
			name_data = Util::toString(input.get_Event_key(i));
			pParent->imagepack[object_key] = MultiData(name_data, "string", "take");


			pParent->imagepack[human_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_human), "image", "take");
			pParent->imagepack[object_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_object), "image", "take");

			hoji_data.push_back(input.get_Event_human(i));
			hoji_flag = true;

			//DB-Lv4-----(持ち去り物体画像)、持ち去り人物ID、持ち去り物体ID
			if (HumanKinectID == 1)
			{
				Event1.push_back("Take");
			}
			if (HumanKinectID == 2)
			{
				Event2.push_back("Take");
			}
			if (HumanKinectID == 3)
			{
				Event3.push_back("Take");
			}
			if (HumanKinectID == 4)
			{
				Event4.push_back("Take");
			}
			if (HumanKinectID == 5)
			{
				Event5.push_back("Take");
			}
			if (HumanKinectID == 6)
			{
				Event6.push_back("Take");
			}
			TakeHumanKinectID.push_back(Util::toString(HumanKinectID));
			TakeObjectID.push_back(Util::toString(input.get_Event_obj(i)));
			// TakeObjectID.push_back(Util::toString(input.get_Event_key(i)));
			TakeHumanID.push_back(Util::toString(input.get_Event_human(i)));

			TakeHumanImg.push_back(Lv4_bring_take_event_human);
			TakeObjectImg.push_back(Lv4_bring_take_event_object);
		}
		// cv::imshow("test", Lv4_bring_take_event);
		pParent->imagepack["Lv4-BringTakeEventImg"] = MultiData(Util::convertMat2String(Lv4_bring_take_event), "image", "Lv4-BringTakeEvent");
	}



	if (input.get_n_event_state().L4.size())
	{
		Lv4_style_detect = input.img_resize.clone();
		Lv4_style_human = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));
		for (int index = 0; index < 6; index++)
		{
			if (index < input.get_n_event_state().L4.size())
			{//，(人物ID,姿勢検知(イベント隠す1,飲む2))
				int i;
				for (i = 0; input.get_t_human_id(i).id != input.get_n_event_state().L4[index].first; i++)
				{
					if (i == input.size_t_human_id())break;
				}
				if (input.get_t_human_id(i).skeletons.size()>0)
				{
					//int indexSkeleton=input.get_t_human_id(i).skelton;
					//cout<<indexSkeleton<<endl;
					for (int indexJoint = 0; indexJoint < input.get_t_human_id(i).skeletons.size(); indexJoint++)
					{
						int x = input.get_t_human_id(i).skeletons[indexJoint].first;
						int y = input.get_t_human_id(i).skeletons[indexJoint].second;
						if (indexJoint == 21 || indexJoint == 23)
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(160, 160, 255), 6, 4);
						}
						else if (indexJoint == 3)
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(0, 255, 255), 6, 4);
						}
						else
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(122, 122, 122), 2, 4);
						}

					}
				}
				char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
				string key = "Lv4-StyleDetect_" + Util::toString(index + 1);
				string name_data = Util::toString(input.get_n_event_state().L4[index].first);
				int x1 = input.get_t_human_id(i).x1;
				int x2 = input.get_t_human_id(i).x2;
				int y1 = input.get_t_human_id(i).y1;
				int cy = input.get_t_human_id(i).ycenter;
				cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, cy - y1));
				Lv4_style_human = src.clone();

				if (input.get_n_event_state().L4[index].second == 1)
				{
					//name_data+="が隠す姿勢中";
					/*	pParent->imagepack[ key ] = MultiData( name_data, "string","隠す動作" );
					string human_key=key+"_HumanImg";
					pParent->imagepack[ human_key ] = MultiData( Util::convertMat2String(Lv4_style_human), "image","隠す動作" );*/
				}
				else
				{
					//name_data+="が飲む姿勢中";
					pParent->imagepack[key] = MultiData(name_data, "string", "drink");
					pParent->imagepack[key].layout = "人物ID：";
					string human_key = key + "_HumanImg";
					pParent->imagepack[human_key] = MultiData(Util::convertMat2String(Lv4_style_human), "image", "drink");
					pParent->imagepack["Lv4-StyleDetectImg"] = MultiData(Util::convertMat2String(Lv4_style_detect), "image", " Lv4-StyleDetect");
				}

				key.clear();
				name_data.clear();
			}
			else
			{
				string key = "Lv4-StyleDetect_" + Util::toString(index + 1);
				pParent->imagepack[key] = MultiData("NULL", "string", ""); key.clear();
				key.clear();
			}
		}

	}
	//cout<<"LV5"<<endl;
	//Lv5viewer-------------------------------------------------物体保持イベント,物体接触イベント
	human_max = input.size_t_human_id();
	Lv5_have_object = input.img_resize.clone();
	for (int i = 0; i < human_max; i++)
	{

		if (input.get_t_human_id(i).have_obj.size())
		{
			int x1 = input.get_t_human_id(i).x1;
			int x2 = input.get_t_human_id(i).x2;
			int y1 = input.get_t_human_id(i).y1;
			int y2 = input.get_t_human_id(i).y2;
			cv::rectangle(Lv5_have_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 243), 1, 4);
			pParent->imagepack["Lv5-HaveObjectImg"] = MultiData(Util::convertMat2String(Lv5_have_object), "image", "保持物体情報");
			HaveHumanX.push_back(Util::toString(x1));
			HaveHumanXX.push_back(Util::toString(x2));
			HaveHumanY.push_back(Util::toString(y1));
			HaveHumanYY.push_back(Util::toString(y2));
		}

	}

	int send_hoji_num = 1;
	for (int i = 0; i < human_max; i++)
	{
		for (int hoji_num = 0; hoji_num < hoji_data.size(); hoji_num++)
		{
			int HumanKinectID = 1;
			if (hoji_data[hoji_num] == input.get_t_human_id(i).id)
			{
				//DB
				if (input.get_t_human_id(i).kinect_id == -1)
				{
					HumanKinectID = 1;
				}
				else
				{
					HumanKinectID = input.get_t_human_id(i).kinect_id;
				}

				char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
				string key = "Lv5-HaveObject_" + Util::toString(send_hoji_num++);
				string have_object = key + "_Object";
				string have_human = key + "_Human";

				pParent->imagepack[have_human] = MultiData(Util::toString(input.get_t_human_id(i).id), "string", "保持動作");
				have_human = have_human + "Img";
				int x1 = input.get_t_human_id(i).x1;
				int x2 = input.get_t_human_id(i).x2;
				int y1 = input.get_t_human_id(i).y1;
				int cy = input.get_t_human_id(i).ycenter;
				cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, cy - y1));
				Lv5_have_object_human = src.clone();

				pParent->imagepack[have_human] = MultiData(Util::convertMat2String(Lv5_have_object_human), "image", "保持動作");

				// pParent->imagepack[have_object] = MultiData(Util::toString(input.get_t_human_id(i).have_obj[0].first), "string", "保持動作");
				pParent->imagepack[have_object] = MultiData(Util::toString(input.get_t_human_id(i).object_ID[input.get_t_human_id(i).object_ID.size() - 1]), "string", "保持動作");
				have_object = have_object + "Img";
				char keyname[128];
				sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(i).have_obj[0]);
				Lv5_have_object_object = cv::imread(keyname, 1);
				pParent->imagepack[have_object] = MultiData(Util::convertMat2String(Lv5_have_object_object), "image", "保持動作");
				//pParent->imagepack["Lv5-HaveObjectImg"] = MultiData( Util::convertMat2String(Lv5_have_object),"image", "保持物体情報");

				//DB-Lv5-----保持人物ID、保持物体ID、保持人物画像、保持物体画像
				if (HumanKinectID == 1)
				{
					Event1.push_back("Have");
				}
				if (HumanKinectID == 2)
				{
					Event2.push_back("Have");
				}
				if (HumanKinectID == 3)
				{
					Event3.push_back("Have");
				}
				if (HumanKinectID == 4)
				{
					Event4.push_back("Have");
				}
				if (HumanKinectID == 5)
				{
					Event5.push_back("Have");
				}
				if (HumanKinectID == 6)
				{
					Event6.push_back("Have");
				}
				HaveHumanKinectID.push_back(Util::toString(HumanKinectID));
				// HaveObjectID.push_back(Util::toString(input.get_t_human_id(i).have_obj[0].first));
				HaveObjectID.push_back(Util::toString(input.get_t_human_id(i).object_ID[input.get_t_human_id(i).object_ID.size() - 1]));
				HaveHumanID.push_back(Util::toString(input.get_t_human_id(i).id));

				HaveHumanImg.push_back(Lv5_have_object_human);
				HaveObjectImg.push_back(Lv5_have_object_object);
			}
		}
	}

	int object_max = input.size_t_object_id();
	Lv5_touch_object = input.img_resize.clone();
	for (int i = 0; i < object_max; i++)
	{
		int x1 = input.get_t_object_id(i).x1;
		int x2 = input.get_t_object_id(i).x2;
		int y1 = input.get_t_object_id(i).y1;
		int y2 = input.get_t_object_id(i).y2;
		cv::rectangle(Lv5_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2, 4);// -1, CV_AA

		for (int index = 0; index<input.get_n_event_state().L5_t.size(); index++)
		{
			if (input.get_n_event_state().L5_t[index].second == input.get_t_object_id(i).model_id)
			{
				cv::rectangle(Lv5_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 3, CV_AA);
			}
			pParent->imagepack["Lv5-TouchObjectImg"] = MultiData(Util::convertMat2String(Lv5_touch_object), "image", "人物と物体の接触情報");
			//DB-Lv5-----接触人物ID、接触物体ID、接触人物画像
			TouchImg.push_back(Lv5_touch_object);
		}
		if (input.EventMAX()>0)
		{
			pParent->imagepack["Lv5-TouchObjectImg"] = MultiData(Util::convertMat2String(Lv5_touch_object), "image", "人物と物体の接触情報");
		}
	}


	//cout<<"LV6"<<endl;
	//Lv6viewer-------------------------------------------------


	if (input.get_n_event_state().L6.size())
	{
		Lv6_hi_event = input.img_resize.clone();
		for (int index = 0; index < 10; index++)
		{
			if (index < input.get_n_event_state().L6.size())
			{//移動隠す飲む動作検知(隠す1飲む2移動3、人物ID)

				string key = "Lv6-HiEvent_" + Util::toString(index + 1);

				if (input.get_n_event_state().L6[index].first == 1)//隠す
				{
					string event_key = key;//+"_Event";
					//string name_data="人物:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体"+Util::toString(input.get_n_event_state().L6_o[index].second)+"を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first)+"に隠す";
					string name_data = "hide";
					pParent->imagepack[event_key] = MultiData(name_data, "string", "hide");


					string event_humanNum = key + "_HumanNum";
					name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "hide");

					string event_ObjectNum = key + "_Object1_Num";


					//name_data="物体:"+Util::toString(input.get_n_event_state().L6_o[index].first)+"←物体:" + Util::toString(input.get_n_event_state().L6_o[index].second);
					pParent->imagepack[event_ObjectNum] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].first), "string", "hide");

					event_ObjectNum = key + "_Object2_Num";
					pParent->imagepack[event_ObjectNum] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].second), "string", "hide");

					string event_Scene = key + "_Scene1";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						HideHumanX.push_back(Util::toString(x1));
						HideHumanXX.push_back(Util::toString(x2));
						HideHumanY.push_back(Util::toString(y1));
						HideHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "hide");

					event_Scene = key + "_Scene2";
					int i = 0;
					for (i = 0; i < input.size_t_object_id(); i++)
					{
						if (input.get_n_event_state().L6_o[index].first == input.get_t_object_id(i).model_id)
						{
							break;
						}
					}
					int x1 = input.get_t_object_id(i).x1;
					int x2 = input.get_t_object_id(i).x2;
					int y1 = input.get_t_object_id(i).y1;
					int y2 = input.get_t_object_id(i).y2;
					cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
					cv::Mat arc(input.img_resize, cv::Rect(x1, y1, x2 - x1, y2 - y1));
					Lv6_hi_event_Scene2 = arc.clone();
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene2), "image", "hide");


					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].second);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					Lv6_hi_event_Scene3 = cv::imread(keyname, 1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "hide");

					//Lv6-隠す-----隠す物体ID、隠される物体ID、人物ID、隠す人物画像、隠す物画像、隠される物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Hide");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Hide");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Hide");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Hide");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Hide");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Hide");
					}
					HideHumanKinectID.push_back(Util::toString(HumanKinectID));
					HideHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					HideObjectAID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));
					HideObjectBID.push_back(Util::toString(input.get_n_event_state().L6_o[index].second));

					HideHumanImg.push_back(Lv6_hi_event_Scene1);
					HideObjectImg.push_back(Lv6_hi_event_Scene2);
					HideInObjectImg.push_back(Lv6_hi_event_Scene3);



				}
				else if (input.get_n_event_state().L6[index].first == 2)//飲む
				{
					string event_humanNum = key + "_HumanNum";
					string name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "drink");
					string event_Object1Num = key + "_Object1_Num";
					pParent->imagepack[event_Object1Num] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].first), "string", "drink");

					string event_key = key;//+"_Event";
					//string name_data="人物:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体"+Util::toString(input.get_n_event_state().L6_o[index].second)+"を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first)+"に隠す";
					//name_data="物体を飲む";
					pParent->imagepack[event_key] = MultiData("飲む", "string", "drink");

					string event_Scene = key + "_Scene2";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						DrinkHumanX.push_back(Util::toString(x1));
						DrinkHumanXX.push_back(Util::toString(x2));
						DrinkHumanY.push_back(Util::toString(y1));
						DrinkHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "drink");

					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					Lv6_hi_event_Scene3 = cv::imread(keyname, 1);
					//cv::imshow("test",Lv6_hi_event_Scene3);
					//cv::waitKey(1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "drink");

					//string event_ObjectNum=key+"_ObjectNum";
					//name_data="物体:"+Util::toString(input.get_n_event_state().L6_o[index].first)+"←人物:" + Util::toString(input.get_n_event_state().L6[index].second);
					//pParent->imagepack[event_ObjectNum] = MultiData( name_data, "string", "飲む");

					//Lv6-飲む-----飲む物体ID、飲む人物ID、飲む人物画像、飲む物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Drink");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Drink");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Drink");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Drink");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Drink");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Drink");
					}
					DrinkHumanKinectID.push_back(Util::toString(HumanKinectID));
					DrinkHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					DrinkObjectID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));

					DrinkHumanImg.push_back(Lv6_hi_event_Scene1);
					DrinkObjectImg.push_back(Lv6_hi_event_Scene3);


				}
				else if (input.get_n_event_state().L6[index].first == 3)
				{
					//string name_data="人物ID:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体ID:"+ Util::toString(input.get_n_event_state().L6_o[index].first)+"の移動を検知";
					//pParent->imagepack[ key ] = MultiData( "移動", "string", "移動動作検知");

					string event_key = key;//+"_Event";
					string name_data = "人物:" + Util::toString(input.get_n_event_state().L6[index].second) + "が物体" + Util::toString(input.get_n_event_state().L6_o[index].second) + "を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first) + "に隠す";
					//name_data="移動";
					pParent->imagepack[event_key] = MultiData("移動", "string", "move");


					string event_humanNum = key + "_HumanNum";
					name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "move");

					string event_ObjectNum = key + "_Object1_Num";
					name_data = Util::toString(input.get_n_event_state().L6_o[index].first);
					pParent->imagepack[event_ObjectNum] = MultiData(name_data, "string", "move");
					event_ObjectNum = key + "_Object2_Num";
					pParent->imagepack[event_ObjectNum] = MultiData(name_data, "string", "move");

					string event_Scene = key + "_Scene1";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						MoveHumanX.push_back(Util::toString(x1));
						MoveHumanXX.push_back(Util::toString(x2));
						MoveHumanY.push_back(Util::toString(y1));
						MoveHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}

					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "move");


					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					// sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					// sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					Lv6_hi_event_Scene2 = cv::imread(keyname, 1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene2), "image", "move");


					event_Scene = key + "_Scene2";
					int i = 0;
					for (i = 0; i < input.size_t_object_id(); i++) { if (input.get_n_event_state().L6_o[index].second == input.get_t_object_id(i).model_id) { break; } }
					int x1 = input.get_t_object_id(i).x1;
					int x2 = input.get_t_object_id(i).x2;
					int y1 = input.get_t_object_id(i).y1;
					int y2 = input.get_t_object_id(i).y2;
					cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
					cv::Mat arc(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
					Lv6_hi_event_Scene3 = arc.clone();
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "move");

					//Lv6-移動-----移動物体ID、移動人物ID、移動人物画像、移動物画像、移動後物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Move");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Move");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Move");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Move");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Move");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Move");
					}
					MoveHumanKinectID.push_back(Util::toString(HumanKinectID));
					MoveHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					MoveObjectID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));
					MoveCenterX.push_back(Util::toString(input.get_t_object_id(i).xcenter));
					MoveCenterY.push_back(Util::toString(input.get_t_object_id(i).ycenter));
					MoveX1.push_back(Util::toString(x1));
					MoveX2.push_back(Util::toString(x2));
					MoveY1.push_back(Util::toString(y1));
					MoveY2.push_back(Util::toString(y2));


					MoveHumanImg.push_back(Lv6_hi_event_Scene1);
					MoveObjectAfterImg.push_back(Lv6_hi_event_Scene2);
					MoveObjectBeforeImg.push_back(Lv6_hi_event_Scene3);

				}
			}
			else
			{
				string key = "Lv6-HiEvent_" + Util::toString(index + 1);
				pParent->imagepack[key] = MultiData("NULL", "string", key);
				key.clear();
			}
		}
		pParent->imagepack["Lv6-HiEventImg"] = MultiData(Util::convertMat2String(Lv6_hi_event), "image", "Lv6:HiEventImg");
	}

	string InsertALL;
	string InsertEvent;
	string InsertColumn;
	string InsertHumanIN, InsertHumanOUT, InsertHumanONnum;
	string InsertBring, InsertTake, InsertHave, InsertDrink, InsertHide, InsertMove, InsertHuman;

	//DB-Lv0-----カラー画像、深度画像、（骨格情報）
	std::vector<uchar> BufColorImg;
	std::vector<int> params( 1 );
	cv::imencode( ".png", Lv0_depth_img, BufColorImg, params );
	string ColorImageStr( BufColorImg.begin(), BufColorImg.end() );
	mysql_real_escape_string( mysql, ColorImgStr, ColorImageStr.c_str(), ColorImageStr.size() );
	std::vector<uchar> BufDepthImg;
	cv::imencode( ".png", Lv0_color_img, BufDepthImg, params );
	string DepthImageStr( BufDepthImg.begin(), BufDepthImg.end() );
	mysql_real_escape_string( mysql, DepthImgStr, DepthImageStr.c_str(), DepthImageStr.size() );


	for ( int count = 0; count < HumanID.size(); count++ )
	{
		if ( HumanKinectID == -1 )
		{
			if ( HumanID.size() >= 1 && HumanID.size()<=6 )
			{
				InsertHuman +=
					",HumanIDKinectNO" + Util::toString( count+1 ) + "=" + HumanID[count]
					;
			}
			
		}
		else
		{
			if ( HumanIDKinectID[count] != "-1" )
			{
				InsertHuman +=
					",HumanIDKinect" + HumanIDKinectID[count] + "=" + HumanID[count]
					;
			}
		}
	}
	if ( humanOutStr.size() != 0 )
	{
		InsertHumanOUT = ",humanOUTID=" + humanOutStr;
	}
	if ( humanInStr.size() != 0 )
	{
		InsertHumanIN = ",humanINID=" + humanInStr;
	}

	for ( int count = 0; count < Event1.size(); count++ )
	{
		InsertEvent += ",Event" + Util::toString( count + 1 ) + "1='" + Event1[count] + "'";
	}
	for ( int count = 0; count < Event2.size(); count++ )
	{
		InsertEvent += ",Event" + Util::toString( count + 1 ) + "2='" + Event2[count] + "'";
	}
	for ( int count = 0; count < Event3.size(); count++ )
	{
		InsertEvent += ",Event" + Util::toString( count + 1 ) + "3='" + Event3[count] + "'";
	}
	for ( int count = 0; count < Event4.size(); count++ )
	{
		InsertEvent += ",Event" + Util::toString( count + 1 ) + "4='" + Event4[count] + "'";
	}
	for ( int count = 0; count < Event5.size(); count++ )
	{
		InsertEvent += ",Event" + Util::toString( count + 1 ) + "5='" + Event5[count] + "'";
	}
	for ( int count = 0; count < Event6.size(); count++ )
	{
		InsertEvent += ",Event" + Util::toString( count + 1 ) + "6='" + Event6[count] + "'";
	}


	for ( int count = 0; count < BringHumanKinectID.size(); count++ )
	{
		InsertBring +=
			//",ObjectNamebringin" + BringHumanKinectID[count] + "='" + ObjectName[count] + "'," +
			",ObjectIDbringin" + BringHumanKinectID[count] + "=" + BringObjectID[count] + "," +
			"HumanIDbringin" + BringHumanKinectID[count] + "=" + BringHumanID[count] + "," +
			"BringCenterX" + BringHumanKinectID[count] + "=" + BringCenterX[count] + "," +
			"BringCenterY" + BringHumanKinectID[count] + "=" + BringCenterY[count] + "," +
			"BringX" + BringHumanKinectID[count] + "=" + BringX1[count] + "," +
			"BringXX" + BringHumanKinectID[count] + "=" + BringX2[count] + "," +
			"BringY" + BringHumanKinectID[count] + "=" + BringY1[count] + "," +
			"BringYY" + BringHumanKinectID[count] + "=" + BringY2[count]
			//"BringHumanImage" + BringHumanKinectID[count] + "='%s'," +
			//"BringObjectImage" + BringHumanKinectID[count] + "='%s'"
			;
		for ( int count = 0; count < BringHumanX.size(); count++ )
		{
			InsertBring +=
				",BringHumanX" + BringHumanKinectID[count] + "=" + BringHumanX[count] + "," +
				"BringHumanXX" + BringHumanKinectID[count] + "=" + BringHumanXX[count] + "," +
				"BringHumanY" + BringHumanKinectID[count] + "=" + BringHumanY[count] + "," +
				"BringHumanYY" + BringHumanKinectID[count] + "=" + BringHumanYY[count]
				;
		}
		cout << "Frame:" << FrameNum << ",CenterX:" << BringCenterX[count] << ",CenterY:" << BringCenterY[count] << endl;
	}

	for ( int count = 0; count < TakeHumanKinectID.size(); count++ )
	{
		InsertTake +=
			",ObjectIDleave" + TakeHumanKinectID[count] + "=" + TakeObjectID[count] + "," +
			"HumanIDleave" + TakeHumanKinectID[count] + "=" + TakeHumanID[count]
			//"LeaveHumanImage" + TakeHumanKinectID[count] + "='%s'," +
			//"LeaveObjectImage" + TakeHumanKinectID[count] + "='%s'"
			;
		for ( int count = 0; count < TakeHumanX.size(); count++ )
		{
			InsertTake +=
				",TakeHumanX" + TakeHumanKinectID[count] + "=" + TakeHumanX[count] + "," +
				"TakeHumanXX" + TakeHumanKinectID[count] + "=" + TakeHumanXX[count] + "," +
				"TakeHumanY" + TakeHumanKinectID[count] + "=" + TakeHumanY[count] + "," +
				"TakeHumanYY" + TakeHumanKinectID[count] + "=" + TakeHumanYY[count]
				;
		}
	}


	for ( int count = 0; count < HaveHumanKinectID.size(); count++ )
	{
		InsertHave +=
			",HaveHumanX" + HaveHumanKinectID[count] + "=" + HaveHumanX[count] + "," +
			"HaveHumanXX" + HaveHumanKinectID[count] + "=" + HaveHumanXX[count] + "," +
			"HaveHumanY" + HaveHumanKinectID[count] + "=" + HaveHumanY[count] + "," +
			"HaveHumanYY" + HaveHumanKinectID[count] + "=" + HaveHumanYY[count] + "," +
			"ObjectIDhave" + HaveHumanKinectID[count] + "=" + HaveObjectID[count] + "," +
			"HumanIDhave" + HaveHumanKinectID[count] + "=" + HaveHumanID[count]
			//"HaveHumanImage" + HaveHumanKinectID[count] + "='%s'," +
			//"HaveObjectImage" + HaveHumanKinectID[count] + "='%s'"
			;
	}

	for ( int count = 0; count < HideHumanKinectID.size(); count++ )
	{
		InsertHide +=
			",HideHumanX" + HideHumanKinectID[count] + "=" + HideHumanX[count] + "," +
			"HideHumanXX" + HideHumanKinectID[count] + "=" + HideHumanXX[count] + "," +
			"HideHumanY" + HideHumanKinectID[count] + "=" + HideHumanY[count] + "," +
			"HideHumanYY" + HideHumanKinectID[count] + "=" + HideHumanYY[count] + "," +
			"ObjectIDhide" + HideHumanKinectID[count] + "=" + HideObjectAID[count] + "," +
			"ObjectIDhidden" + HideHumanKinectID[count] + "=" + HideObjectBID[count] + "," +
			"HumanIDhide" + HideHumanKinectID[count] + "=" + HideHumanID[count]
			//"HideHumanImage" + HideHumanKinectID[count] + "='%s'," +
			//"HideObjectImage" + HideHumanKinectID[count] + "='%s'," +
			//"HideInObjectImage" + HideHumanKinectID[count] + "='%s'"
			;
	}
	for ( int count = 0; count < DrinkHumanKinectID.size(); count++ )
	{
		InsertDrink +=
			",DrinkHumanX" + DrinkHumanKinectID[count] + "=" + DrinkHumanX[count] + "," +
			"DrinkHumanXX" + DrinkHumanKinectID[count] + "=" + DrinkHumanXX[count] + "," +
			"DrinkHumanY" + DrinkHumanKinectID[count] + "=" + DrinkHumanY[count] + "," +
			"DrinkHumanYY" + DrinkHumanKinectID[count] + "=" + DrinkHumanYY[count] + "," +
			"ObjectIDdrink" + DrinkHumanKinectID[count] + "=" + DrinkObjectID[count] + "," +
			"HumanIDdrink" + DrinkHumanKinectID[count] + "=" + DrinkHumanID[count]
			//"DrinkHumanImage" + DrinkHumanKinectID[count] + "='%s'," +
			//"DrinkObjectImage" + DrinkHumanKinectID[count] + "='%s'"
			;
	}
	for ( int count = 0; count < MoveHumanKinectID.size(); count++ )
	{
		InsertMove +=
			",MoveHumanX" + MoveHumanKinectID[count] + "=" + MoveHumanX[count] + "," +
			"MoveHumanXX" + MoveHumanKinectID[count] + "=" + MoveHumanXX[count] + "," +
			"MoveHumanY" + MoveHumanKinectID[count] + "=" + MoveHumanY[count] + "," +
			"MoveHumanYY" + MoveHumanKinectID[count] + "=" + MoveHumanYY[count] + "," +
			"ObjectIDmove" + MoveHumanKinectID[count] + "=" + MoveObjectID[count] + "," +
			"HumanIDmove" + MoveHumanKinectID[count] + "=" + MoveHumanID[count] + "," +
			"MoveCenterX" + MoveHumanKinectID[count] + "=" + MoveCenterX[count] + "," +
			"MoveCenterY" + MoveHumanKinectID[count] + "=" + MoveCenterY[count] + "," +
			"MoveX" + MoveHumanKinectID[count] + "=" + MoveX1[count] + "," +
			"MoveXX" + MoveHumanKinectID[count] + "=" + MoveX2[count] + "," +
			"MoveY" + MoveHumanKinectID[count] + "=" + MoveY1[count] + "," +
			"MoveYY" + MoveHumanKinectID[count] + "=" + MoveY2[count]
			//"MoveHumanImage" + MoveHumanKinectID[count] + "=%s," +
			//"MoveAfterObjectImage" + MoveHumanKinectID[count] + "='%s'," +
			//"MoveBeforeObjectImage" + MoveHumanKinectID[count] + "='%s'"
			;
	}
	string EventFlag;
	if ( InsertBring.size() != 0 || InsertTake.size() != 0 || InsertHave.size() != 0 || InsertDrink.size() != 0 || InsertHide.size() != 0 || InsertMove.size() != 0 )
	{
		EventFlag = "true";
	}
	else
	{
		EventFlag = "false";
	}
	if ( InsertHuman.size() != 0 || InsertBring.size() != 0 || InsertTake.size() != 0 || InsertHave.size() != 0 || InsertDrink.size() != 0 || InsertHide.size() != 0 || InsertMove.size() != 0 )
	{
		InsertColumn = InsertHumanIN + InsertHumanOUT + InsertEvent + InsertHuman + InsertBring + InsertTake + InsertHave + InsertDrink + InsertHide + InsertMove;

		InsertALL =
			"INSERT  MainTable SET "
			"FrameCount=" + FrameNum + ",ID=" + Util::toString( sceneID ) + ",SceneFrame=" + Util::toString( sceneFrame ) +
			",EventFlag='" + EventFlag + "',CameraID='meeting'"
			",humanONnum=" + Util::toString( humanONnum ) +
			",ColorImage='%s',DepthImage='%s'" + InsertColumn + ";"
			;

		//mysql_real_queryを使用するためにqueryの長さが必要
		// cout << strlen(InsertALL.c_str()) << ", " << strlen(ColorImgStr) << ", " << strlen(DepthImgStr) << endl;
		mysqlSize = sprintf_s(query, sizeof(DepthImgStr), InsertALL.c_str(), ColorImgStr, DepthImgStr);
		if ( mysql_real_query( mysql, query, mysqlSize ) != 0 )
		{
			//cout << mysql_error(mysql) << endl;
			//cout << "query_error" << endl;
		}
	}
	HumanCount = input.size_t_human_id();
}


void CCamera::OutputImage2(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag)//アプリケーションに画像出力　
{
	string FrameNum = Util::toString(frame);
	pParent->imagepack["frame"] = MultiData(FrameNum, "string", "frame");
	pParent->imagepack["frame"].layout = "フレーム数:";
	bool hoji_flag = false; std::vector<int> hoji_data;
	//cout<<"out"<<endl;

	cv::Mat Lv0_depth_img;
	cv::Mat Lv0_color_img;
	cv::Mat Lv1_diff_label_img;
	cv::Mat Lv2_sub_human;
	cv::Mat Lv2_sub_object;
	cv::Mat Lv3_track_human;
	cv::Mat Lv3_sub_touch_object;
	cv::Mat Lv4_style_detect;
	cv::Mat Lv4_style_human;
	cv::Mat Lv4_bring_take_event;
	cv::Mat Lv4_bring_take_event_human;
	cv::Mat Lv4_bring_take_event_object;
	cv::Mat Lv5_have_object;
	cv::Mat Lv5_have_object_human;
	cv::Mat Lv5_have_object_object;
	cv::Mat Lv5_touch_object;
	cv::Mat Lv6_hi_event;
	cv::Mat Lv6_hi_event_Scene1;
	cv::Mat Lv6_hi_event_Scene2;
	cv::Mat Lv6_hi_event_Scene3;

	//DB用変数----------------------------------------------------------
	//vector<string> TouchHumanKinectID, TouchObjectID, TouchHumanID;
	//vector<cv::Mat>TouchHumanImg, TouchObjectImg;
	vector<string> HumanID, HumanIDKinectID;
	int HumanKinectID;
	string humanInStr, humanOutStr, humanONnumStr;
	vector<cv::Mat>TouchImg;
	vector<string> ObjectName;
	vector<string> BringHumanKinectID, BringObjectID, BringHumanID, BringCenterX, BringCenterY, BringX1, BringY1, BringX2, BringY2;
	vector<string> BringHumanX, BringHumanXX, BringHumanY, BringHumanYY;
	vector<cv::Mat>BringHumanImg, BringObjectImg;
	vector<string> TakeHumanKinectID, TakeObjectID, TakeHumanID;
	vector<string> TakeHumanX, TakeHumanXX, TakeHumanY, TakeHumanYY;
	vector<cv::Mat>TakeHumanImg, TakeObjectImg;
	vector<string> HaveHumanKinectID, HaveObjectID, HaveHumanID;
	vector<string>HaveHumanX, HaveHumanXX, HaveHumanY, HaveHumanYY;
	vector<cv::Mat>HaveHumanImg, HaveObjectImg;
	vector<string> DrinkHumanKinectID, DrinkObjectID, DrinkHumanID;
	vector<string> DrinkHumanX, DrinkHumanXX, DrinkHumanY, DrinkHumanYY;
	vector<cv::Mat>DrinkHumanImg, DrinkObjectImg;
	vector<string> HideHumanKinectID, HideObjectAID, HideObjectBID, HideHumanID;
	vector<string> HideHumanX, HideHumanXX, HideHumanY, HideHumanYY;
	vector<cv::Mat>HideHumanImg, HideObjectImg, HideInObjectImg;
	vector<string> MoveHumanKinectID, MoveObjectID, MoveHumanID, MoveCenterX, MoveCenterY, MoveX1, MoveY1, MoveX2, MoveY2;
	vector<string> MoveHumanX, MoveHumanXX, MoveHumanY, MoveHumanYY;
	vector<cv::Mat>MoveHumanImg, MoveObjectBeforeImg, MoveObjectAfterImg;
	vector<string>Event1, Event2, Event3, Event4, Event5, Event6;
	//--------------------------------------------------------------------
	//DB用シーンID、シーンフレーム更新------------------------------------
	humanONnum = input.size_t_human_id();
	if (input.size_t_human_id() >= 1)
	{
		DBbool = true;
		sceneFrame++;
	}
	else
	{
		DBbool = false;
	}
	if (DBbool)
	{
		if (HumanCount > input.size_t_human_id())
		{
			sceneID++;
			humanOUT++;
			sceneFrame = 1;
			humanOutStr = Util::toString(humanOUT);
		}
		else if (HumanCount < input.size_t_human_id())
		{
			sceneID++;
			humanIN++;
			sceneFrame = 1;
			humanInStr = Util::toString(humanIN);
		}
		else
		{

		}
	}
	//------------------------------------------------------------------

	//LV0viewer-------------------------------------------------骨格情報：深度情報
	cv::Vec3b pixel_value1;
	//cv::Vec3b pixel_value2;
	Lv0_depth_img = input.img_resize.clone();
	Lv0_color_img = input.img_original.clone();
	for (int j = 0; j < input.Height(); j++)
	{
		for (int i = 0; i < input.Width(); i++)
		{
			pixel_value1.val[0] = 255 * (max(0, ((int)input.get_depth(i, j) - DEPTH_RANGE_MIN))) / DEPTH_RANGE_MAX;
			pixel_value1.val[1] = pixel_value1.val[0];
			pixel_value1.val[2] = pixel_value1.val[0];
			/*pixel_value2.val[0]=255*(max(0,((int)input.get_depth_is_diff(i,j)-DEPTH_RANGE_MIN)))/DEPTH_RANGE_MAX;
			pixel_value2.val[1]=pixel_value2.val[0];
			pixel_value2.val[2]=pixel_value2.val[0]; */
			//depth_in.at<cv::Vec3b>(j,i)=pixel_value1;
			Lv0_depth_img.at<cv::Vec3b>(j, i) = pixel_value1;
		}
	}


	string name_data0 = Util::toString(input.set_skeltons().size());
	pParent->imagepack["Lv0-SkeletonNum"] = MultiData(name_data0, "string", "skeleton");
	pParent->imagepack["Lv0-SkeletonNum"].layout = "人物骨格数:";
	pParent->imagepack["Lv0-DepthImg"] = MultiData(Util::convertMat2String(Lv0_depth_img), "image", "深度情報");




	//Lv1viewer-------------------------------------------------背景差分情報
	input.dOut().convertTo(Lv1_diff_label_img, CV_8UC3);

	string name_data1 = Util::toString(input.LabelData());
	pParent->imagepack["Lv1-DiffLabelNum"] = MultiData(name_data1, "string", "label");
	pParent->imagepack["Lv1-DiffLabelNum"].layout = "ラベル数:";
	if (input.LabelData() > 0)
	{
		pParent->imagepack["Lv1-DiffLabelImg"] = MultiData(Util::convertMat2String(Lv1_diff_label_img), "image", "ラベル情報");
	}

	//Lv2viewer-------------------------------------------------人物・物体候補情報
	input.Out().convertTo(Lv2_sub_human, CV_8UC3);
	int human_max = input.HumanData();
	for (int i = 1; i <= human_max; i++)
	{
		if (input.get_HumanTab_skelton(i) != -1)
		{
			int skeleton_num = input.get_HumanTab_skelton(i);
			for (int indexJoint = 0; indexJoint < input.set_skeltons()[skeleton_num].size(); indexJoint++)
			{
				int x = input.set_skeltons()[skeleton_num][indexJoint].first;
				int y = input.set_skeltons()[skeleton_num][indexJoint].second;
				cv::circle(Lv2_sub_human, cv::Point(x, y), 1, cv::Scalar(122, 122, 122), 2, 4);
			}
		}
		pParent->imagepack["Lv2-SubHumanImg"] = MultiData(Util::convertMat2String(Lv2_sub_human), "image", "人物候補情報");
	}
	string name_data2_hum = Util::toString(human_max);
	pParent->imagepack["Lv2-SubHumanNum"] = MultiData(name_data2_hum, "string", "Lv2:SubHumanNum");
	pParent->imagepack["Lv2-SubHumanNum"].layout = "人物候補数:";

	Lv2_sub_object = input.img_resize.clone();
	for (int t = 1; t <= input.ObjectData(); t++)
	{
		stringstream ss;
		ss << "num:" << t;
		//cv::putText(inputO, ss.str(),cv::Point(input.get_ObjectTab_s_w(t)-10,input.get_ObjectTab_s_h(t)) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 2, CV_AA); 
		cv::rectangle(Lv2_sub_object, cv::Rect(input.get_ObjectTab_s_w(t), input.get_ObjectTab_s_h(t), input.get_ObjectTab_e_w(t) - input.get_ObjectTab_s_w(t), input.get_ObjectTab_e_h(t) - input.get_ObjectTab_s_h(t)), cv::Scalar(0, 0, 255));
		stringstream aa;
		aa << "cnt:" << input.get_ObjectTab_count(t);
		int red = input.get_ObjectTab_count(t) == 20 ? 100 : input.get_ObjectTab_count(t) * 155 / 20 + 100;
		int green = input.get_ObjectTab_count(t) == 20 ? 255 : 100 + input.get_ObjectTab_count(t) * 100 / 20;
		int blue = input.get_ObjectTab_count(t) == 20 ? 255 : 100 + input.get_ObjectTab_count(t) * 100 / 20;
		cv::putText(Lv2_sub_object, aa.str(), cv::Point(input.get_ObjectTab_s_w(t), input.get_ObjectTab_e_h(t) + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(red, green, blue), 2, CV_AA);

		pParent->imagepack["Lv2-SubObjectImg"] = MultiData(Util::convertMat2String(Lv2_sub_object), "image", "物体候補情報");
	}
	string name_data2_obj = Util::toString(human_max);

	pParent->imagepack["Lv2-SubObjectNum"] = MultiData(name_data2_obj, "string", "Lv2:SubObjectNum");
	pParent->imagepack["Lv2-SubObjectNum"].layout = "物体候補数:";


	//Lv3viewer-------------------------------------------------人物追跡・接触候補検知
	human_max = input.size_t_human_id();
	Lv3_track_human = input.img_resize.clone();
	for (int i = 0; i < 10; i++)
	{
		if (i < input.size_t_human_id())
		{

			int x1 = input.get_t_human_id(i).x1;
			int x2 = input.get_t_human_id(i).x2;
			int y1 = input.get_t_human_id(i).y1;
			int y2 = input.get_t_human_id(i).y2;
			cv::rectangle(Lv3_track_human, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);
			stringstream aa;
			aa << "ID:" << input.get_t_human_id(i).id;//<<":"<<input.get_t_human_id(i).skeletons.size();
			cv::putText(Lv3_track_human, aa.str(), cv::Point(x1, y1 + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2, CV_AA);

			string key = "Lv3-TrackHuman_" + Util::toString(i + 1);
			string name_data = Util::toString(input.get_t_human_id(i).id);
			pParent->imagepack[key] = MultiData(name_data, "string", key);
			pParent->imagepack[key].layout = "人物ID:";
			//DBHumanID,
			HumanID.push_back(Util::toString(input.get_t_human_id(i).id));
			HumanIDKinectID.push_back(Util::toString(input.get_t_human_id(i).kinect_id));
			HumanKinectID = input.get_t_human_id(i).kinect_id;

			key.clear();
			pParent->imagepack["Lv3-TrackHumanImg"] = MultiData(Util::convertMat2String(Lv3_track_human), "image", "人物追跡情報");

		}
		else
		{
			string key = "Lv3-TrackHuman_" + Util::toString(i + 1);
			pParent->imagepack[key] = MultiData("NULL", "string", key);
			pParent->imagepack[key].layout = " ";
			key.clear();
		}
	}



	Lv3_sub_touch_object = input.img_resize.clone();
	for (int i = 0; i < input.Event_size(); i++)
	{
		int num = -1;
		for (int t = 0; t < input.size_t_human_id(); t++) { if (input.get_Event_human(i) == input.get_t_human_id(t).id) { num = t; } }
		if (num != -1)
		{
			int x1 = input.get_t_human_id(num).x1;
			int x2 = input.get_t_human_id(num).x2;
			int y1 = input.get_t_human_id(num).y1;
			int y2 = input.get_t_human_id(num).y2;
			cv::rectangle(Lv3_sub_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
		}
		int x1 = input.get_Event_s_w(i);
		int x2 = input.get_Event_e_w(i);
		int y1 = input.get_Event_s_h(i);
		int y2 = input.get_Event_e_h(i);
		cv::rectangle(Lv3_sub_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
		pParent->imagepack["Lv3-SubTouchObjectImg"] = MultiData(Util::convertMat2String(Lv3_sub_touch_object), "image", "候補接触情報");
	}


	//cout<<"LV4"<<endl;
	//Lv4viewer-------------------------------------------------持ち込み持ち去り・姿勢検知検知
	Lv4_bring_take_event = input.img_resize.clone();

	for (int i = 0; i < input.Event_size(); i++)
	{
		Lv4_bring_take_event_human = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));
		Lv4_bring_take_event_object = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));

		//string keyString = "Event"+Util::toString( i+1 );
		//string keyString_label = "イベント履歴"+Util::toString( i+1 )+"ラベル";
		//人物IDが物体IDを
		int num = -1;
		int HumanKinectID = 1;
		human_max = input.size_t_human_id();
		for (int t = 0; t < human_max; t++)
		{
			if (input.get_Event_human(i) == input.get_t_human_id(t).id)
			{
				//DB
				if (input.get_t_human_id(t).kinect_id == -1)
				{
					HumanKinectID = 1;
				}
				else
				{
					HumanKinectID = input.get_t_human_id(t).kinect_id;
				}
				num = t;
			}
		}
		if (num != -1)
		{
			int x1 = input.get_t_human_id(num).x1;
			int x2 = input.get_t_human_id(num).x2;
			int y1 = input.get_t_human_id(num).y1;
			int y2 = input.get_t_human_id(num).y2;
			int cx = input.get_t_human_id(num).xcenter;
			int cy = input.get_t_human_id(num).ycenter;
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);
			cv::Mat a(input.img_resize, cv::Rect(x1, y1, x2 - x1, cy - y1));
			Lv4_bring_take_event_human = a.clone();
			BringHumanX.push_back(Util::toString(x1));
			BringHumanXX.push_back(Util::toString(x2));
			BringHumanY.push_back(Util::toString(y1));
			BringHumanYY.push_back(Util::toString(y2));
			TakeHumanX.push_back(Util::toString(x1));
			TakeHumanXX.push_back(Util::toString(x2));
			TakeHumanY.push_back(Util::toString(y1));
			TakeHumanYY.push_back(Util::toString(y2));
		}


		int x1 = input.get_Event_s_w(i);
		int x2 = input.get_Event_e_w(i);
		int y1 = input.get_Event_s_h(i);
		int y2 = input.get_Event_e_h(i);
		int cx = input.get_Event_cx(i);
		int cy = input.get_Event_cy(i);
		cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);



		stringstream aa;
		// aa << "ID:" << input.get_Event_key(i);//<<":"<<input.get_t_human_id(i).skeletons.size();
		aa << "ID:" << input.get_Event_obj(i);//<<":"<<input.get_t_human_id(i).skeletons.size();
		// aa << "ID:" << input.get_Event_human( i );//<<":"<<input.get_t_human_id(i).skeletons.size();
		cv::putText(Lv4_bring_take_event, aa.str(), cv::Point(x1, y1 + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2, CV_AA);

		string key = "Lv4-BringTakeEvent_" + Util::toString(i + 1);
		string event_key = key;//+"_Event";
		string object_key = key + "_Object";
		string human_key = key + "_Human";
		string human_keyimg = human_key + "Img";
		string object_keyimg = object_key + "Img";
		if (input.get_Event_Eve(i) != 0)
		{
			x1 = input.get_Event_s_w(i);
			x2 = input.get_Event_e_w(i);
			y1 = input.get_Event_s_h(i);
			y2 = input.get_Event_e_h(i);
			cx = input.get_Event_cx(i);
			cy = input.get_Event_cy(i);
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 2, 4);

			char keyname[128];
			sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_Event_model(i));
			Lv4_bring_take_event_object = cv::imread(keyname);

			string name_data = "bring";
			pParent->imagepack[event_key] = MultiData(name_data, "string", "bring");
			name_data = Util::toString(input.get_Event_human(i));
			pParent->imagepack[human_key] = MultiData(name_data, "string", "bring");
			name_data = Util::toString(input.get_Event_key(i));
			pParent->imagepack[object_key] = MultiData(name_data, "string", "bring");

			pParent->imagepack[human_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_human), "image", "bring");
			pParent->imagepack[object_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_object), "image", "bring");
			//string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を持ち込み";
			//pParent->imagepack[ keyString ] = MultiData( name_data, "string", "持ち込み" );


			//if(input.get_Event_Eve(i)==2 )//移動検知
			//{
			//	//検知画像出力領域
			//    string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を移動";
			//	pParent->imagepack[ keyString ] = MultiData(name_data, "string",  "移動"  );
			//}
			/*else
			{
			string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を持ち込み";
			pParent->imagepack[ keyString ] = MultiData( name_data, "string",  "持ち込み"  );
			}*/

			//DB-Lv4-----持ち込み物体画像、持ち込み人物ID、持ち込み物体ID、持ち込み物体位置
			if (HumanKinectID == 1)
			{
				Event1.push_back("Bring");
			}
			if (HumanKinectID == 2)
			{
				Event2.push_back("Bring");
			}
			if (HumanKinectID == 3)
			{
				Event3.push_back("Bring");
			}
			if (HumanKinectID == 4)
			{
				Event4.push_back("Bring");
			}
			if (HumanKinectID == 5)
			{
				Event5.push_back("Bring");
			}
			if (HumanKinectID == 6)
			{
				Event6.push_back("Bring");
			}
			BringHumanKinectID.push_back(Util::toString(HumanKinectID));
			// BringObjectID.push_back(Util::toString(input.get_Event_key(i)));
			BringObjectID.push_back(Util::toString(input.get_Event_obj(i)));
			BringHumanID.push_back(Util::toString(input.get_Event_human(i)));
			BringCenterX.push_back(Util::toString(cx));
			BringCenterY.push_back(Util::toString(cy));
			BringX1.push_back(Util::toString(x1));
			BringX2.push_back(Util::toString(x2));
			BringY1.push_back(Util::toString(y1));
			BringY2.push_back(Util::toString(y2));

			BringHumanImg.push_back(Lv4_bring_take_event_human);
			BringObjectImg.push_back(Lv4_bring_take_event_object);

		}
		else
		{
			x1 = input.get_Event_s_w(i);
			x2 = input.get_Event_e_w(i);
			y1 = input.get_Event_s_h(i);
			y2 = input.get_Event_e_h(i);
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);

			char keyname[128];
			sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_Event_model(i));
			Lv4_bring_take_event_object = cv::imread(keyname, 1);

			string name_data = "take";
			pParent->imagepack[event_key] = MultiData(name_data, "string", "take");
			name_data = Util::toString(input.get_Event_human(i));
			pParent->imagepack[human_key] = MultiData(name_data, "string", "take");
			name_data = Util::toString(input.get_Event_key(i));
			pParent->imagepack[object_key] = MultiData(name_data, "string", "take");


			pParent->imagepack[human_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_human), "image", "take");
			pParent->imagepack[object_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_object), "image", "take");

			hoji_data.push_back(input.get_Event_human(i));
			hoji_flag = true;

			//DB-Lv4-----(持ち去り物体画像)、持ち去り人物ID、持ち去り物体ID
			if (HumanKinectID == 1)
			{
				Event1.push_back("Take");
			}
			if (HumanKinectID == 2)
			{
				Event2.push_back("Take");
			}
			if (HumanKinectID == 3)
			{
				Event3.push_back("Take");
			}
			if (HumanKinectID == 4)
			{
				Event4.push_back("Take");
			}
			if (HumanKinectID == 5)
			{
				Event5.push_back("Take");
			}
			if (HumanKinectID == 6)
			{
				Event6.push_back("Take");
			}
			TakeHumanKinectID.push_back(Util::toString(HumanKinectID));
			TakeObjectID.push_back(Util::toString(input.get_Event_obj(i)));
			// TakeObjectID.push_back(Util::toString(input.get_Event_key(i)));
			TakeHumanID.push_back(Util::toString(input.get_Event_human(i)));

			TakeHumanImg.push_back(Lv4_bring_take_event_human);
			TakeObjectImg.push_back(Lv4_bring_take_event_object);
		}
		// cv::imshow("test", Lv4_bring_take_event);
		pParent->imagepack["Lv4-BringTakeEventImg"] = MultiData(Util::convertMat2String(Lv4_bring_take_event), "image", "Lv4-BringTakeEvent");
	}



	if (input.get_n_event_state().L4.size())
	{
		Lv4_style_detect = input.img_resize.clone();
		Lv4_style_human = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));
		for (int index = 0; index < 6; index++)
		{
			if (index < input.get_n_event_state().L4.size())
			{//，(人物ID,姿勢検知(イベント隠す1,飲む2))
				int i;
				for (i = 0; input.get_t_human_id(i).id != input.get_n_event_state().L4[index].first; i++)
				{
					if (i == input.size_t_human_id())break;
				}
				if (input.get_t_human_id(i).skeletons.size()>0)
				{
					//int indexSkeleton=input.get_t_human_id(i).skelton;
					//cout<<indexSkeleton<<endl;
					for (int indexJoint = 0; indexJoint < input.get_t_human_id(i).skeletons.size(); indexJoint++)
					{
						int x = input.get_t_human_id(i).skeletons[indexJoint].first;
						int y = input.get_t_human_id(i).skeletons[indexJoint].second;
						if (indexJoint == 21 || indexJoint == 23)
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(160, 160, 255), 6, 4);
						}
						else if (indexJoint == 3)
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(0, 255, 255), 6, 4);
						}
						else
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(122, 122, 122), 2, 4);
						}

					}
				}
				char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
				string key = "Lv4-StyleDetect_" + Util::toString(index + 1);
				string name_data = Util::toString(input.get_n_event_state().L4[index].first);
				int x1 = input.get_t_human_id(i).x1;
				int x2 = input.get_t_human_id(i).x2;
				int y1 = input.get_t_human_id(i).y1;
				int cy = input.get_t_human_id(i).ycenter;
				cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, cy - y1));
				Lv4_style_human = src.clone();

				if (input.get_n_event_state().L4[index].second == 1)
				{
					//name_data+="が隠す姿勢中";
					/*	pParent->imagepack[ key ] = MultiData( name_data, "string","隠す動作" );
					string human_key=key+"_HumanImg";
					pParent->imagepack[ human_key ] = MultiData( Util::convertMat2String(Lv4_style_human), "image","隠す動作" );*/
				}
				else
				{
					//name_data+="が飲む姿勢中";
					pParent->imagepack[key] = MultiData(name_data, "string", "drink");
					pParent->imagepack[key].layout = "人物ID：";
					string human_key = key + "_HumanImg";
					pParent->imagepack[human_key] = MultiData(Util::convertMat2String(Lv4_style_human), "image", "drink");
					pParent->imagepack["Lv4-StyleDetectImg"] = MultiData(Util::convertMat2String(Lv4_style_detect), "image", " Lv4-StyleDetect");
				}

				key.clear();
				name_data.clear();
			}
			else
			{
				string key = "Lv4-StyleDetect_" + Util::toString(index + 1);
				pParent->imagepack[key] = MultiData("NULL", "string", ""); key.clear();
				key.clear();
			}
		}

	}
	//cout<<"LV5"<<endl;
	//Lv5viewer-------------------------------------------------物体保持イベント,物体接触イベント
	human_max = input.size_t_human_id();
	Lv5_have_object = input.img_resize.clone();
	for (int i = 0; i < human_max; i++)
	{

		if (input.get_t_human_id(i).have_obj.size())
		{
			int x1 = input.get_t_human_id(i).x1;
			int x2 = input.get_t_human_id(i).x2;
			int y1 = input.get_t_human_id(i).y1;
			int y2 = input.get_t_human_id(i).y2;
			cv::rectangle(Lv5_have_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 243), 1, 4);
			pParent->imagepack["Lv5-HaveObjectImg"] = MultiData(Util::convertMat2String(Lv5_have_object), "image", "保持物体情報");
			HaveHumanX.push_back(Util::toString(x1));
			HaveHumanXX.push_back(Util::toString(x2));
			HaveHumanY.push_back(Util::toString(y1));
			HaveHumanYY.push_back(Util::toString(y2));
		}

	}

	int send_hoji_num = 1;
	for (int i = 0; i < human_max; i++)
	{
		for (int hoji_num = 0; hoji_num < hoji_data.size(); hoji_num++)
		{
			int HumanKinectID = 1;
			if (hoji_data[hoji_num] == input.get_t_human_id(i).id)
			{
				//DB
				if (input.get_t_human_id(i).kinect_id == -1)
				{
					HumanKinectID = 1;
				}
				else
				{
					HumanKinectID = input.get_t_human_id(i).kinect_id;
				}

				char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
				string key = "Lv5-HaveObject_" + Util::toString(send_hoji_num++);
				string have_object = key + "_Object";
				string have_human = key + "_Human";

				pParent->imagepack[have_human] = MultiData(Util::toString(input.get_t_human_id(i).id), "string", "保持動作");
				have_human = have_human + "Img";
				int x1 = input.get_t_human_id(i).x1;
				int x2 = input.get_t_human_id(i).x2;
				int y1 = input.get_t_human_id(i).y1;
				int cy = input.get_t_human_id(i).ycenter;
				cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, cy - y1));
				Lv5_have_object_human = src.clone();

				pParent->imagepack[have_human] = MultiData(Util::convertMat2String(Lv5_have_object_human), "image", "保持動作");

				// pParent->imagepack[have_object] = MultiData(Util::toString(input.get_t_human_id(i).have_obj[0].first), "string", "保持動作");
				pParent->imagepack[have_object] = MultiData(Util::toString(input.get_t_human_id(i).object_ID[input.get_t_human_id(i).object_ID.size() - 1]), "string", "保持動作");
				have_object = have_object + "Img";
				
				char keyfilename[128];
				sprintf(keyfilename, "roi-%05d.png", input.get_t_human_id(i).have_obj[0]);
				string keyFileName = keyfilename;
				string keyname = "../DB/output-img/roi/" + keyFileName;

				Lv5_have_object_object = cv::imread( keyname.c_str(), 1 );
				pParent->imagepack[have_object] = MultiData(Util::convertMat2String(Lv5_have_object_object), "image", "保持動作");
				//pParent->imagepack["Lv5-HaveObjectImg"] = MultiData( Util::convertMat2String(Lv5_have_object),"image", "保持物体情報");

				//DB-Lv5-----保持人物ID、保持物体ID、保持人物画像、保持物体画像
				if (HumanKinectID == 1)
				{
					Event1.push_back("Have");
				}
				if (HumanKinectID == 2)
				{
					Event2.push_back("Have");
				}
				if (HumanKinectID == 3)
				{
					Event3.push_back("Have");
				}
				if (HumanKinectID == 4)
				{
					Event4.push_back("Have");
				}
				if (HumanKinectID == 5)
				{
					Event5.push_back("Have");
				}
				if (HumanKinectID == 6)
				{
					Event6.push_back("Have");
				}
				HaveHumanKinectID.push_back(Util::toString(HumanKinectID));
				// HaveObjectID.push_back(Util::toString(input.get_t_human_id(i).have_obj[0].first));
				HaveObjectID.push_back(Util::toString(input.get_t_human_id(i).object_ID[input.get_t_human_id(i).object_ID.size() - 1]));
				HaveHumanID.push_back(Util::toString(input.get_t_human_id(i).id));

				HaveHumanImg.push_back(Lv5_have_object_human);
				HaveObjectImg.push_back(Lv5_have_object_object);
			}
		}
	}

	int object_max = input.size_t_object_id();
	Lv5_touch_object = input.img_resize.clone();
	for (int i = 0; i < object_max; i++)
	{
		int x1 = input.get_t_object_id(i).x1;
		int x2 = input.get_t_object_id(i).x2;
		int y1 = input.get_t_object_id(i).y1;
		int y2 = input.get_t_object_id(i).y2;
		cv::rectangle(Lv5_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2, 4);// -1, CV_AA

		for (int index = 0; index<input.get_n_event_state().L5_t.size(); index++)
		{
			if (input.get_n_event_state().L5_t[index].second == input.get_t_object_id(i).model_id)
			{
				cv::rectangle(Lv5_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 3, CV_AA);
			}
			pParent->imagepack["Lv5-TouchObjectImg"] = MultiData(Util::convertMat2String(Lv5_touch_object), "image", "人物と物体の接触情報");
			//DB-Lv5-----接触人物ID、接触物体ID、接触人物画像
			TouchImg.push_back(Lv5_touch_object);
		}
		if (input.EventMAX()>0)
		{
			pParent->imagepack["Lv5-TouchObjectImg"] = MultiData(Util::convertMat2String(Lv5_touch_object), "image", "人物と物体の接触情報");
		}
	}


	//cout<<"LV6"<<endl;
	//Lv6viewer-------------------------------------------------


	if (input.get_n_event_state().L6.size())
	{
		Lv6_hi_event = input.img_resize.clone();
		for (int index = 0; index < 10; index++)
		{
			if (index < input.get_n_event_state().L6.size())
			{//移動隠す飲む動作検知(隠す1飲む2移動3、人物ID)

				string key = "Lv6-HiEvent_" + Util::toString(index + 1);

				if (input.get_n_event_state().L6[index].first == 1)//隠す
				{
					string event_key = key;//+"_Event";
					//string name_data="人物:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体"+Util::toString(input.get_n_event_state().L6_o[index].second)+"を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first)+"に隠す";
					string name_data = "hide";
					pParent->imagepack[event_key] = MultiData(name_data, "string", "hide");


					string event_humanNum = key + "_HumanNum";
					name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "hide");

					string event_ObjectNum = key + "_Object1_Num";


					//name_data="物体:"+Util::toString(input.get_n_event_state().L6_o[index].first)+"←物体:" + Util::toString(input.get_n_event_state().L6_o[index].second);
					pParent->imagepack[event_ObjectNum] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].first), "string", "hide");

					event_ObjectNum = key + "_Object2_Num";
					pParent->imagepack[event_ObjectNum] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].second), "string", "hide");

					string event_Scene = key + "_Scene1";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						HideHumanX.push_back(Util::toString(x1));
						HideHumanXX.push_back(Util::toString(x2));
						HideHumanY.push_back(Util::toString(y1));
						HideHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "hide");

					event_Scene = key + "_Scene2";
					int i = 0;
					for (i = 0; i < input.size_t_object_id(); i++)
					{
						if (input.get_n_event_state().L6_o[index].first == input.get_t_object_id(i).model_id)
						{
							break;
						}
					}
					int x1 = input.get_t_object_id(i).x1;
					int x2 = input.get_t_object_id(i).x2;
					int y1 = input.get_t_object_id(i).y1;
					int y2 = input.get_t_object_id(i).y2;
					cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
					cv::Mat arc(input.img_resize, cv::Rect(x1, y1, x2 - x1, y2 - y1));
					Lv6_hi_event_Scene2 = arc.clone();
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene2), "image", "hide");


					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].second);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					Lv6_hi_event_Scene3 = cv::imread(keyname, 1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "hide");

					//Lv6-隠す-----隠す物体ID、隠される物体ID、人物ID、隠す人物画像、隠す物画像、隠される物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Hide");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Hide");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Hide");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Hide");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Hide");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Hide");
					}
					HideHumanKinectID.push_back(Util::toString(HumanKinectID));
					HideHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					HideObjectAID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));
					HideObjectBID.push_back(Util::toString(input.get_n_event_state().L6_o[index].second));

					HideHumanImg.push_back(Lv6_hi_event_Scene1);
					HideObjectImg.push_back(Lv6_hi_event_Scene2);
					HideInObjectImg.push_back(Lv6_hi_event_Scene3);



				}
				else if (input.get_n_event_state().L6[index].first == 2)//飲む
				{
					string event_humanNum = key + "_HumanNum";
					string name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "drink");
					string event_Object1Num = key + "_Object1_Num";
					pParent->imagepack[event_Object1Num] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].first), "string", "drink");

					string event_key = key;//+"_Event";
					//string name_data="人物:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体"+Util::toString(input.get_n_event_state().L6_o[index].second)+"を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first)+"に隠す";
					//name_data="物体を飲む";
					pParent->imagepack[event_key] = MultiData("飲む", "string", "drink");

					string event_Scene = key + "_Scene2";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						DrinkHumanX.push_back(Util::toString(x1));
						DrinkHumanXX.push_back(Util::toString(x2));
						DrinkHumanY.push_back(Util::toString(y1));
						DrinkHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "drink");

					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					Lv6_hi_event_Scene3 = cv::imread(keyname, 1);
					//cv::imshow("test",Lv6_hi_event_Scene3);
					//cv::waitKey(1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "drink");

					//string event_ObjectNum=key+"_ObjectNum";
					//name_data="物体:"+Util::toString(input.get_n_event_state().L6_o[index].first)+"←人物:" + Util::toString(input.get_n_event_state().L6[index].second);
					//pParent->imagepack[event_ObjectNum] = MultiData( name_data, "string", "飲む");

					//Lv6-飲む-----飲む物体ID、飲む人物ID、飲む人物画像、飲む物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Drink");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Drink");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Drink");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Drink");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Drink");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Drink");
					}
					DrinkHumanKinectID.push_back(Util::toString(HumanKinectID));
					DrinkHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					DrinkObjectID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));

					DrinkHumanImg.push_back(Lv6_hi_event_Scene1);
					DrinkObjectImg.push_back(Lv6_hi_event_Scene3);


				}
				else if (input.get_n_event_state().L6[index].first == 3)
				{
					//string name_data="人物ID:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体ID:"+ Util::toString(input.get_n_event_state().L6_o[index].first)+"の移動を検知";
					//pParent->imagepack[ key ] = MultiData( "移動", "string", "移動動作検知");

					string event_key = key;//+"_Event";
					string name_data = "人物:" + Util::toString(input.get_n_event_state().L6[index].second) + "が物体" + Util::toString(input.get_n_event_state().L6_o[index].second) + "を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first) + "に隠す";
					//name_data="移動";
					pParent->imagepack[event_key] = MultiData("移動", "string", "move");


					string event_humanNum = key + "_HumanNum";
					name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "move");

					string event_ObjectNum = key + "_Object1_Num";
					name_data = Util::toString(input.get_n_event_state().L6_o[index].first);
					pParent->imagepack[event_ObjectNum] = MultiData(name_data, "string", "move");
					event_ObjectNum = key + "_Object2_Num";
					pParent->imagepack[event_ObjectNum] = MultiData(name_data, "string", "move");

					string event_Scene = key + "_Scene1";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						MoveHumanX.push_back(Util::toString(x1));
						MoveHumanXX.push_back(Util::toString(x2));
						MoveHumanY.push_back(Util::toString(y1));
						MoveHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}

					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "move");


					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					// sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					// sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					Lv6_hi_event_Scene2 = cv::imread(keyname, 1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene2), "image", "move");


					event_Scene = key + "_Scene2";
					int i = 0;
					for (i = 0; i < input.size_t_object_id(); i++) { if (input.get_n_event_state().L6_o[index].second == input.get_t_object_id(i).model_id) { break; } }
					int x1 = input.get_t_object_id(i).x1;
					int x2 = input.get_t_object_id(i).x2;
					int y1 = input.get_t_object_id(i).y1;
					int y2 = input.get_t_object_id(i).y2;
					cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
					cv::Mat arc(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
					Lv6_hi_event_Scene3 = arc.clone();
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "move");

					//Lv6-移動-----移動物体ID、移動人物ID、移動人物画像、移動物画像、移動後物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Move");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Move");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Move");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Move");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Move");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Move");
					}
					MoveHumanKinectID.push_back(Util::toString(HumanKinectID));
					MoveHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					MoveObjectID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));
					MoveCenterX.push_back(Util::toString(input.get_t_object_id(i).xcenter));
					MoveCenterY.push_back(Util::toString(input.get_t_object_id(i).ycenter));
					MoveX1.push_back(Util::toString(x1));
					MoveX2.push_back(Util::toString(x2));
					MoveY1.push_back(Util::toString(y1));
					MoveY2.push_back(Util::toString(y2));


					MoveHumanImg.push_back(Lv6_hi_event_Scene1);
					MoveObjectAfterImg.push_back(Lv6_hi_event_Scene2);
					MoveObjectBeforeImg.push_back(Lv6_hi_event_Scene3);

				}
			}
			else
			{
				string key = "Lv6-HiEvent_" + Util::toString(index + 1);
				pParent->imagepack[key] = MultiData("NULL", "string", key);
				key.clear();
			}
		}
		pParent->imagepack["Lv6-HiEventImg"] = MultiData(Util::convertMat2String(Lv6_hi_event), "image", "Lv6:HiEventImg");
	}

	string InsertALL;
	string InsertEvent;
	string InsertColumn;
	string InsertHumanIN, InsertHumanOUT, InsertHumanONnum;
	string InsertBring, InsertTake, InsertHave, InsertDrink, InsertHide, InsertMove, InsertHuman;

	//DB-Lv0-----カラー画像、深度画像、（骨格情報）
	std::vector<uchar> BufColorImg;
	std::vector<int> params(1);
	cv::imencode(".png", Lv0_depth_img, BufColorImg, params);
	string ColorImageStr(BufColorImg.begin(), BufColorImg.end());
	mysql_real_escape_string(mysql, ColorImgStr, ColorImageStr.c_str(), ColorImageStr.size());
	std::vector<uchar> BufDepthImg;
	cv::imencode(".png", Lv0_color_img, BufDepthImg, params);
	string DepthImageStr(BufDepthImg.begin(), BufDepthImg.end());
	mysql_real_escape_string(mysql, DepthImgStr, DepthImageStr.c_str(), DepthImageStr.size());


	for (int count = 0; count < HumanID.size(); count++)
	{
		if (HumanKinectID == -1)
		{
			if (HumanID.size() >= 1 && HumanID.size() <= 6)
			{
				InsertHuman +=
					",HumanIDKinectNO" + Util::toString(count + 1) + "=" + HumanID[count]
					;
			}

		}
		else
		{
			if (HumanIDKinectID[count] != "-1")
			{
				InsertHuman +=
					",HumanIDKinect" + HumanIDKinectID[count] + "=" + HumanID[count]
					;
			}
		}
	}
	if (humanOutStr.size() != 0)
	{
		InsertHumanOUT = ",humanOUTID=" + humanOutStr;
	}
	if (humanInStr.size() != 0)
	{
		InsertHumanIN = ",humanINID=" + humanInStr;
	}

	for (int count = 0; count < Event1.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "1='" + Event1[count] + "'";
	}
	for (int count = 0; count < Event2.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "2='" + Event2[count] + "'";
	}
	for (int count = 0; count < Event3.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "3='" + Event3[count] + "'";
	}
	for (int count = 0; count < Event4.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "4='" + Event4[count] + "'";
	}
	for (int count = 0; count < Event5.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "5='" + Event5[count] + "'";
	}
	for (int count = 0; count < Event6.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "6='" + Event6[count] + "'";
	}


	for (int count = 0; count < BringHumanKinectID.size(); count++)
	{
		InsertBring +=
			//",ObjectNamebringin" + BringHumanKinectID[count] + "='" + ObjectName[count] + "'," +
			",ObjectIDbringin" + BringHumanKinectID[count] + "=" + BringObjectID[count] + "," +
			"HumanIDbringin" + BringHumanKinectID[count] + "=" + BringHumanID[count] + "," +
			"BringCenterX" + BringHumanKinectID[count] + "=" + BringCenterX[count] + "," +
			"BringCenterY" + BringHumanKinectID[count] + "=" + BringCenterY[count] + "," +
			"BringX" + BringHumanKinectID[count] + "=" + BringX1[count] + "," +
			"BringXX" + BringHumanKinectID[count] + "=" + BringX2[count] + "," +
			"BringY" + BringHumanKinectID[count] + "=" + BringY1[count] + "," +
			"BringYY" + BringHumanKinectID[count] + "=" + BringY2[count]
			//"BringHumanImage" + BringHumanKinectID[count] + "='%s'," +
			//"BringObjectImage" + BringHumanKinectID[count] + "='%s'"
			;
		for (int count = 0; count < BringHumanX.size(); count++)
		{
			InsertBring +=
				",BringHumanX" + BringHumanKinectID[count] + "=" + BringHumanX[count] + "," +
				"BringHumanXX" + BringHumanKinectID[count] + "=" + BringHumanXX[count] + "," +
				"BringHumanY" + BringHumanKinectID[count] + "=" + BringHumanY[count] + "," +
				"BringHumanYY" + BringHumanKinectID[count] + "=" + BringHumanYY[count]
				;
		}
		cout << "Frame:" << FrameNum << ",CenterX:" << BringCenterX[count] << ",CenterY:" << BringCenterY[count] << endl;
	}

	for (int count = 0; count < TakeHumanKinectID.size(); count++)
	{
		InsertTake +=
			",ObjectIDleave" + TakeHumanKinectID[count] + "=" + TakeObjectID[count] + "," +
			"HumanIDleave" + TakeHumanKinectID[count] + "=" + TakeHumanID[count]
			//"LeaveHumanImage" + TakeHumanKinectID[count] + "='%s'," +
			//"LeaveObjectImage" + TakeHumanKinectID[count] + "='%s'"
			;
		for (int count = 0; count < TakeHumanX.size(); count++)
		{
			InsertTake +=
				",TakeHumanX" + TakeHumanKinectID[count] + "=" + TakeHumanX[count] + "," +
				"TakeHumanXX" + TakeHumanKinectID[count] + "=" + TakeHumanXX[count] + "," +
				"TakeHumanY" + TakeHumanKinectID[count] + "=" + TakeHumanY[count] + "," +
				"TakeHumanYY" + TakeHumanKinectID[count] + "=" + TakeHumanYY[count]
				;
		}
	}


	for (int count = 0; count < HaveHumanKinectID.size(); count++)
	{
		InsertHave +=
			",HaveHumanX" + HaveHumanKinectID[count] + "=" + HaveHumanX[count] + "," +
			"HaveHumanXX" + HaveHumanKinectID[count] + "=" + HaveHumanXX[count] + "," +
			"HaveHumanY" + HaveHumanKinectID[count] + "=" + HaveHumanY[count] + "," +
			"HaveHumanYY" + HaveHumanKinectID[count] + "=" + HaveHumanYY[count] + "," +
			"ObjectIDhave" + HaveHumanKinectID[count] + "=" + HaveObjectID[count] + "," +
			"HumanIDhave" + HaveHumanKinectID[count] + "=" + HaveHumanID[count]
			//"HaveHumanImage" + HaveHumanKinectID[count] + "='%s'," +
			//"HaveObjectImage" + HaveHumanKinectID[count] + "='%s'"
			;
	}

	for (int count = 0; count < HideHumanKinectID.size(); count++)
	{
		InsertHide +=
			",HideHumanX" + HideHumanKinectID[count] + "=" + HideHumanX[count] + "," +
			"HideHumanXX" + HideHumanKinectID[count] + "=" + HideHumanXX[count] + "," +
			"HideHumanY" + HideHumanKinectID[count] + "=" + HideHumanY[count] + "," +
			"HideHumanYY" + HideHumanKinectID[count] + "=" + HideHumanYY[count] + "," +
			"ObjectIDhide" + HideHumanKinectID[count] + "=" + HideObjectAID[count] + "," +
			"ObjectIDhidden" + HideHumanKinectID[count] + "=" + HideObjectBID[count] + "," +
			"HumanIDhide" + HideHumanKinectID[count] + "=" + HideHumanID[count]
			//"HideHumanImage" + HideHumanKinectID[count] + "='%s'," +
			//"HideObjectImage" + HideHumanKinectID[count] + "='%s'," +
			//"HideInObjectImage" + HideHumanKinectID[count] + "='%s'"
			;
	}
	for (int count = 0; count < DrinkHumanKinectID.size(); count++)
	{
		InsertDrink +=
			",DrinkHumanX" + DrinkHumanKinectID[count] + "=" + DrinkHumanX[count] + "," +
			"DrinkHumanXX" + DrinkHumanKinectID[count] + "=" + DrinkHumanXX[count] + "," +
			"DrinkHumanY" + DrinkHumanKinectID[count] + "=" + DrinkHumanY[count] + "," +
			"DrinkHumanYY" + DrinkHumanKinectID[count] + "=" + DrinkHumanYY[count] + "," +
			"ObjectIDdrink" + DrinkHumanKinectID[count] + "=" + DrinkObjectID[count] + "," +
			"HumanIDdrink" + DrinkHumanKinectID[count] + "=" + DrinkHumanID[count]
			//"DrinkHumanImage" + DrinkHumanKinectID[count] + "='%s'," +
			//"DrinkObjectImage" + DrinkHumanKinectID[count] + "='%s'"
			;
	}
	for (int count = 0; count < MoveHumanKinectID.size(); count++)
	{
		InsertMove +=
			",MoveHumanX" + MoveHumanKinectID[count] + "=" + MoveHumanX[count] + "," +
			"MoveHumanXX" + MoveHumanKinectID[count] + "=" + MoveHumanXX[count] + "," +
			"MoveHumanY" + MoveHumanKinectID[count] + "=" + MoveHumanY[count] + "," +
			"MoveHumanYY" + MoveHumanKinectID[count] + "=" + MoveHumanYY[count] + "," +
			"ObjectIDmove" + MoveHumanKinectID[count] + "=" + MoveObjectID[count] + "," +
			"HumanIDmove" + MoveHumanKinectID[count] + "=" + MoveHumanID[count] + "," +
			"MoveCenterX" + MoveHumanKinectID[count] + "=" + MoveCenterX[count] + "," +
			"MoveCenterY" + MoveHumanKinectID[count] + "=" + MoveCenterY[count] + "," +
			"MoveX" + MoveHumanKinectID[count] + "=" + MoveX1[count] + "," +
			"MoveXX" + MoveHumanKinectID[count] + "=" + MoveX2[count] + "," +
			"MoveY" + MoveHumanKinectID[count] + "=" + MoveY1[count] + "," +
			"MoveYY" + MoveHumanKinectID[count] + "=" + MoveY2[count]
			//"MoveHumanImage" + MoveHumanKinectID[count] + "=%s," +
			//"MoveAfterObjectImage" + MoveHumanKinectID[count] + "='%s'," +
			//"MoveBeforeObjectImage" + MoveHumanKinectID[count] + "='%s'"
			;
	}
	string EventFlag;
	if (InsertBring.size() != 0 || InsertTake.size() != 0 || InsertHave.size() != 0 || InsertDrink.size() != 0 || InsertHide.size() != 0 || InsertMove.size() != 0)
	{
		EventFlag = "true";
	}
	else
	{
		EventFlag = "false";
	}
	if (InsertHuman.size() != 0 || InsertBring.size() != 0 || InsertTake.size() != 0 || InsertHave.size() != 0 || InsertDrink.size() != 0 || InsertHide.size() != 0 || InsertMove.size() != 0)
	{
		InsertColumn = InsertHumanIN + InsertHumanOUT + InsertEvent + InsertHuman + InsertBring + InsertTake + InsertHave + InsertDrink + InsertHide + InsertMove;

		InsertALL =
			"INSERT  MainTable SET "
			"FrameCount=" + FrameNum + ",ID=" + Util::toString(sceneID) + ",SceneFrame=" + Util::toString(sceneFrame) +
			",EventFlag='" + EventFlag + "',CameraID='meeting'"
			",humanONnum=" + Util::toString(humanONnum) +
			",ColorImage='%s',DepthImage='%s'" + InsertColumn + ";"
			;

		//mysql_real_queryを使用するためにqueryの長さが必要
		// cout << strlen(InsertALL.c_str()) << ", " << strlen(ColorImgStr) << ", " << strlen(DepthImgStr) << endl;
		mysqlSize = sprintf_s(query, sizeof(DepthImgStr), InsertALL.c_str(), ColorImgStr, DepthImgStr);
		if (mysql_real_query(mysql, query, mysqlSize) != 0)
		{
			//cout << mysql_error(mysql) << endl;
			//cout << "query_error" << endl;
		}
	}
	HumanCount = input.size_t_human_id();
}

void CCamera::OutputImageOffline(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag)//アプリケーションに画像出力　
{
	string FrameNum = Util::toString(frame);
	pParent->imagepack["frame"] = MultiData(FrameNum, "string", "frame");
	pParent->imagepack["frame"].layout = "フレーム数:";
	bool hoji_flag = false; std::vector<int> hoji_data;
	//cout<<"out"<<endl;

	cv::Mat Lv0_depth_img;
	cv::Mat Lv0_color_img;
	cv::Mat Lv1_diff_label_img;
	cv::Mat Lv2_sub_human;
	cv::Mat Lv2_sub_object;
	cv::Mat Lv3_track_human;
	cv::Mat Lv3_sub_touch_object;
	cv::Mat Lv4_style_detect;
	cv::Mat Lv4_style_human;
	cv::Mat Lv4_bring_take_event;
	cv::Mat Lv4_bring_take_event_human;
	cv::Mat Lv4_bring_take_event_object;
	cv::Mat Lv5_have_object;
	cv::Mat Lv5_have_object_human;
	cv::Mat Lv5_have_object_object;
	cv::Mat Lv5_touch_object;
	cv::Mat Lv6_hi_event;
	cv::Mat Lv6_hi_event_Scene1;
	cv::Mat Lv6_hi_event_Scene2;
	cv::Mat Lv6_hi_event_Scene3;

	//DB用変数----------------------------------------------------------
	//vector<string> TouchHumanKinectID, TouchObjectID, TouchHumanID;
	//vector<cv::Mat>TouchHumanImg, TouchObjectImg;
	vector<string> HumanID, HumanIDKinectID;
	int HumanKinectID;
	string humanInStr, humanOutStr, humanONnumStr;
	vector<cv::Mat>TouchImg;
	vector<string> BringHumanKinectID, BringObjectID, BringHumanID, BringCenterX, BringCenterY, BringX1, BringY1, BringX2, BringY2;
	vector<string> BringHumanX, BringHumanXX, BringHumanY, BringHumanYY;
	vector<cv::Mat>BringHumanImg, BringObjectImg;
	vector<string> TakeHumanKinectID, TakeObjectID, TakeHumanID;
	vector<string> TakeHumanX, TakeHumanXX, TakeHumanY, TakeHumanYY;
	vector<cv::Mat>TakeHumanImg, TakeObjectImg;
	vector<string> HaveHumanKinectID, HaveObjectID, HaveHumanID;
	vector<string>HaveHumanX, HaveHumanXX, HaveHumanY, HaveHumanYY;
	vector<cv::Mat>HaveHumanImg, HaveObjectImg;
	vector<string> DrinkHumanKinectID, DrinkObjectID, DrinkHumanID;
	vector<string> DrinkHumanX, DrinkHumanXX, DrinkHumanY, DrinkHumanYY;
	vector<cv::Mat>DrinkHumanImg, DrinkObjectImg;
	vector<string> HideHumanKinectID, HideObjectAID, HideObjectBID, HideHumanID;
	vector<string> HideHumanX, HideHumanXX, HideHumanY, HideHumanYY;
	vector<cv::Mat>HideHumanImg, HideObjectImg, HideInObjectImg;
	vector<string> MoveHumanKinectID, MoveObjectID, MoveHumanID, MoveCenterX, MoveCenterY, MoveX1, MoveY1, MoveX2, MoveY2;
	vector<string> MoveHumanX, MoveHumanXX, MoveHumanY, MoveHumanYY;
	vector<cv::Mat>MoveHumanImg, MoveObjectBeforeImg, MoveObjectAfterImg;
	vector<string>Event1, Event2, Event3, Event4, Event5, Event6;
	//--------------------------------------------------------------------
	//DB用シーンID、シーンフレーム更新------------------------------------
	humanONnum = input.size_t_human_id();
	if (input.size_t_human_id() >= 1)
	{
		DBbool = true;
		sceneFrame++;
	}
	else
	{
		DBbool = false;
	}
	if (DBbool)
	{
		if (HumanCount > input.size_t_human_id())
		{
			sceneID++;
			humanOUT++;
			sceneFrame = 1;
			humanOutStr = Util::toString(humanOUT);
		}
		else if (HumanCount < input.size_t_human_id())
		{
			sceneID++;
			humanIN++;
			sceneFrame = 1;
			humanInStr = Util::toString(humanIN);
		}
		else
		{

		}
	}
	//------------------------------------------------------------------

	//LV0viewer-------------------------------------------------骨格情報：深度情報
	cv::Vec3b pixel_value1;
	//cv::Vec3b pixel_value2;
	Lv0_depth_img = input.img_resize.clone();
	Lv0_color_img = input.img_original.clone();
	for (int j = 0; j < input.Height(); j++)
	{
		for (int i = 0; i < input.Width(); i++)
		{
			pixel_value1.val[0] = 255 * (max(0, ((int)input.get_depth(i, j) - DEPTH_RANGE_MIN))) / DEPTH_RANGE_MAX;
			pixel_value1.val[1] = pixel_value1.val[0];
			pixel_value1.val[2] = pixel_value1.val[0];
			/*pixel_value2.val[0]=255*(max(0,((int)input.get_depth_is_diff(i,j)-DEPTH_RANGE_MIN)))/DEPTH_RANGE_MAX;
			pixel_value2.val[1]=pixel_value2.val[0];
			pixel_value2.val[2]=pixel_value2.val[0]; */
			//depth_in.at<cv::Vec3b>(j,i)=pixel_value1;
			Lv0_depth_img.at<cv::Vec3b>(j, i) = pixel_value1;
		}
	}


	string name_data0 = Util::toString(input.set_skeltons().size());
	pParent->imagepack["Lv0-SkeletonNum"] = MultiData(name_data0, "string", "skeleton");
	pParent->imagepack["Lv0-SkeletonNum"].layout = "人物骨格数:";
	pParent->imagepack["Lv0-DepthImg"] = MultiData(Util::convertMat2String(Lv0_depth_img), "image", "深度情報");




	//Lv1viewer-------------------------------------------------背景差分情報
	input.dOut().convertTo(Lv1_diff_label_img, CV_8UC3);

	string name_data1 = Util::toString(input.LabelData());
	pParent->imagepack["Lv1-DiffLabelNum"] = MultiData(name_data1, "string", "label");
	pParent->imagepack["Lv1-DiffLabelNum"].layout = "ラベル数:";
	if (input.LabelData() > 0)
	{
		pParent->imagepack["Lv1-DiffLabelImg"] = MultiData(Util::convertMat2String(Lv1_diff_label_img), "image", "ラベル情報");
	}

	//Lv2viewer-------------------------------------------------人物・物体候補情報
	input.Out().convertTo(Lv2_sub_human, CV_8UC3);
	int human_max = input.HumanData();
	for (int i = 1; i <= human_max; i++)
	{
		if (input.get_HumanTab_skelton(i) != -1)
		{
			int skeleton_num = input.get_HumanTab_skelton(i);
			for (int indexJoint = 0; indexJoint < input.set_skeltons()[skeleton_num].size(); indexJoint++)
			{
				int x = input.set_skeltons()[skeleton_num][indexJoint].first;
				int y = input.set_skeltons()[skeleton_num][indexJoint].second;
				cv::circle(Lv2_sub_human, cv::Point(x, y), 1, cv::Scalar(122, 122, 122), 2, 4);
			}
		}
		pParent->imagepack["Lv2-SubHumanImg"] = MultiData(Util::convertMat2String(Lv2_sub_human), "image", "人物候補情報");
	}
	string name_data2_hum = Util::toString(human_max);
	pParent->imagepack["Lv2-SubHumanNum"] = MultiData(name_data2_hum, "string", "Lv2:SubHumanNum");
	pParent->imagepack["Lv2-SubHumanNum"].layout = "人物候補数:";

	Lv2_sub_object = input.img_resize.clone();
	for (int t = 1; t <= input.ObjectData(); t++)
	{
		stringstream ss;
		ss << "num:" << t;
		//cv::putText(inputO, ss.str(),cv::Point(input.get_ObjectTab_s_w(t)-10,input.get_ObjectTab_s_h(t)) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 2, CV_AA); 
		cv::rectangle(Lv2_sub_object, cv::Rect(input.get_ObjectTab_s_w(t), input.get_ObjectTab_s_h(t), input.get_ObjectTab_e_w(t) - input.get_ObjectTab_s_w(t), input.get_ObjectTab_e_h(t) - input.get_ObjectTab_s_h(t)), cv::Scalar(0, 0, 255));
		stringstream aa;
		aa << "cnt:" << input.get_ObjectTab_count(t);
		int red = input.get_ObjectTab_count(t) == 20 ? 100 : input.get_ObjectTab_count(t) * 155 / 20 + 100;
		int green = input.get_ObjectTab_count(t) == 20 ? 255 : 100 + input.get_ObjectTab_count(t) * 100 / 20;
		int blue = input.get_ObjectTab_count(t) == 20 ? 255 : 100 + input.get_ObjectTab_count(t) * 100 / 20;
		cv::putText(Lv2_sub_object, aa.str(), cv::Point(input.get_ObjectTab_s_w(t), input.get_ObjectTab_e_h(t) + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(red, green, blue), 2, CV_AA);

		pParent->imagepack["Lv2-SubObjectImg"] = MultiData(Util::convertMat2String(Lv2_sub_object), "image", "物体候補情報");
	}
	string name_data2_obj = Util::toString(human_max);

	pParent->imagepack["Lv2-SubObjectNum"] = MultiData(name_data2_obj, "string", "Lv2:SubObjectNum");
	pParent->imagepack["Lv2-SubObjectNum"].layout = "物体候補数:";


	//Lv3viewer-------------------------------------------------人物追跡・接触候補検知
	human_max = input.size_t_human_id();
	Lv3_track_human = input.img_resize.clone();
	for (int i = 0; i < 10; i++)
	{
		if (i < input.size_t_human_id())
		{

			int x1 = input.get_t_human_id(i).x1;
			int x2 = input.get_t_human_id(i).x2;
			int y1 = input.get_t_human_id(i).y1;
			int y2 = input.get_t_human_id(i).y2;
			cv::rectangle(Lv3_track_human, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);
			stringstream aa;
			aa << "ID:" << input.get_t_human_id(i).id;//<<":"<<input.get_t_human_id(i).skeletons.size();
			cv::putText(Lv3_track_human, aa.str(), cv::Point(x1, y1 + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2, CV_AA);

			string key = "Lv3-TrackHuman_" + Util::toString(i + 1);
			string name_data = Util::toString(input.get_t_human_id(i).id);
			pParent->imagepack[key] = MultiData(name_data, "string", key);
			pParent->imagepack[key].layout = "人物ID:";
			//DBHumanID,
			HumanID.push_back(Util::toString(input.get_t_human_id(i).id));
			HumanIDKinectID.push_back(Util::toString(input.get_t_human_id(i).kinect_id));
			HumanKinectID = input.get_t_human_id(i).kinect_id;

			key.clear();
			pParent->imagepack["Lv3-TrackHumanImg"] = MultiData(Util::convertMat2String(Lv3_track_human), "image", "人物追跡情報");

		}
		else
		{
			string key = "Lv3-TrackHuman_" + Util::toString(i + 1);
			pParent->imagepack[key] = MultiData("NULL", "string", key);
			pParent->imagepack[key].layout = " ";
			key.clear();
		}
	}



	Lv3_sub_touch_object = input.img_resize.clone();
	for (int i = 0; i < input.Event_size(); i++)
	{
		int num = -1;
		for (int t = 0; t < input.size_t_human_id(); t++) { if (input.get_Event_human(i) == input.get_t_human_id(t).id) { num = t; } }
		if (num != -1)
		{
			int x1 = input.get_t_human_id(num).x1;
			int x2 = input.get_t_human_id(num).x2;
			int y1 = input.get_t_human_id(num).y1;
			int y2 = input.get_t_human_id(num).y2;
			cv::rectangle(Lv3_sub_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
		}
		int x1 = input.get_Event_s_w(i);
		int x2 = input.get_Event_e_w(i);
		int y1 = input.get_Event_s_h(i);
		int y2 = input.get_Event_e_h(i);
		cv::rectangle(Lv3_sub_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
		pParent->imagepack["Lv3-SubTouchObjectImg"] = MultiData(Util::convertMat2String(Lv3_sub_touch_object), "image", "候補接触情報");
	}


	//cout<<"LV4"<<endl;
	//Lv4viewer-------------------------------------------------持ち込み持ち去り・姿勢検知検知
	Lv4_bring_take_event = input.img_resize.clone();

	for (int i = 0; i < input.Event_size(); i++)
	{
		Lv4_bring_take_event_human = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));
		Lv4_bring_take_event_object = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));

		//string keyString = "Event"+Util::toString( i+1 );
		//string keyString_label = "イベント履歴"+Util::toString( i+1 )+"ラベル";
		//人物IDが物体IDを
		int num = -1;
		int HumanKinectID = 1;
		human_max = input.size_t_human_id();
		for (int t = 0; t < human_max; t++)
		{
			if (input.get_Event_human(i) == input.get_t_human_id(t).id)
			{
				//DB
				if (input.get_t_human_id(t).kinect_id == -1)
				{
					HumanKinectID = 1;
				}
				else
				{
					HumanKinectID = input.get_t_human_id(t).kinect_id;
				}
				num = t;
			}
		}
		if (num != -1)
		{
			int x1 = input.get_t_human_id(num).x1;
			int x2 = input.get_t_human_id(num).x2;
			int y1 = input.get_t_human_id(num).y1;
			int y2 = input.get_t_human_id(num).y2;
			int cx = input.get_t_human_id(num).xcenter;
			int cy = input.get_t_human_id(num).ycenter;
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);
			cv::Mat a(input.img_resize, cv::Rect(x1, y1, x2 - x1, cy - y1));
			Lv4_bring_take_event_human = a.clone();
			BringHumanX.push_back(Util::toString(x1));
			BringHumanXX.push_back(Util::toString(x2));
			BringHumanY.push_back(Util::toString(y1));
			BringHumanYY.push_back(Util::toString(y2));
			TakeHumanX.push_back(Util::toString(x1));
			TakeHumanXX.push_back(Util::toString(x2));
			TakeHumanY.push_back(Util::toString(y1));
			TakeHumanYY.push_back(Util::toString(y2));
		}


		int x1 = input.get_Event_s_w(i);
		int x2 = input.get_Event_e_w(i);
		int y1 = input.get_Event_s_h(i);
		int y2 = input.get_Event_e_h(i);
		int cx = input.get_Event_cx(i);
		int cy = input.get_Event_cy(i);
		cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 1, 4);



		stringstream aa;
		// aa << "ID:" << input.get_Event_key(i);//<<":"<<input.get_t_human_id(i).skeletons.size();
		aa << "ID:" << input.get_Event_obj(i);//<<":"<<input.get_t_human_id(i).skeletons.size();
		// aa << "ID:" << input.get_Event_human( i );//<<":"<<input.get_t_human_id(i).skeletons.size();
		cv::putText(Lv4_bring_take_event, aa.str(), cv::Point(x1, y1 + 10) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2, CV_AA);

		string key = "Lv4-BringTakeEvent_" + Util::toString(i + 1);
		string event_key = key;//+"_Event";
		string object_key = key + "_Object";
		string human_key = key + "_Human";
		string human_keyimg = human_key + "Img";
		string object_keyimg = object_key + "Img";
		if (input.get_Event_Eve(i) != 0)
		{
			x1 = input.get_Event_s_w(i);
			x2 = input.get_Event_e_w(i);
			y1 = input.get_Event_s_h(i);
			y2 = input.get_Event_e_h(i);
			cx = input.get_Event_cx(i);
			cy = input.get_Event_cy(i);
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 2, 4);

			char keyname[128];
			sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_Event_model(i));
			Lv4_bring_take_event_object = cv::imread(keyname);

			string name_data = "bring";
			pParent->imagepack[event_key] = MultiData(name_data, "string", "bring");
			name_data = Util::toString(input.get_Event_human(i));
			pParent->imagepack[human_key] = MultiData(name_data, "string", "bring");
			name_data = Util::toString(input.get_Event_key(i));
			pParent->imagepack[object_key] = MultiData(name_data, "string", "bring");

			pParent->imagepack[human_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_human), "image", "bring");
			pParent->imagepack[object_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_object), "image", "bring");
			//string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を持ち込み";
			//pParent->imagepack[ keyString ] = MultiData( name_data, "string", "持ち込み" );


			//if(input.get_Event_Eve(i)==2 )//移動検知
			//{
			//	//検知画像出力領域
			//    string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を移動";
			//	pParent->imagepack[ keyString ] = MultiData(name_data, "string",  "移動"  );
			//}
			/*else
			{
			string name_data="人物ID:"+Util::toString(input.get_Event_human(i))+"が物体ID:"+Util::toString(input.get_Event_model(i))+"を持ち込み";
			pParent->imagepack[ keyString ] = MultiData( name_data, "string",  "持ち込み"  );
			}*/

			//DB-Lv4-----持ち込み物体画像、持ち込み人物ID、持ち込み物体ID、持ち込み物体位置
			if (HumanKinectID == 1)
			{
				Event1.push_back("Bring");
			}
			if (HumanKinectID == 2)
			{
				Event2.push_back("Bring");
			}
			if (HumanKinectID == 3)
			{
				Event3.push_back("Bring");
			}
			if (HumanKinectID == 4)
			{
				Event4.push_back("Bring");
			}
			if (HumanKinectID == 5)
			{
				Event5.push_back("Bring");
			}
			if (HumanKinectID == 6)
			{
				Event6.push_back("Bring");
			}
			BringHumanKinectID.push_back(Util::toString(HumanKinectID));
			// BringObjectID.push_back(Util::toString(input.get_Event_key(i)));
			BringObjectID.push_back(Util::toString(input.get_Event_obj(i)));
			BringHumanID.push_back(Util::toString(input.get_Event_human(i)));
			BringCenterX.push_back(Util::toString(cx));
			BringCenterY.push_back(Util::toString(cy));
			BringX1.push_back(Util::toString(x1));
			BringX2.push_back(Util::toString(x2));
			BringY1.push_back(Util::toString(y1));
			BringY2.push_back(Util::toString(y2));

			BringHumanImg.push_back(Lv4_bring_take_event_human);
			BringObjectImg.push_back(Lv4_bring_take_event_object);

		}
		else
		{
			x1 = input.get_Event_s_w(i);
			x2 = input.get_Event_e_w(i);
			y1 = input.get_Event_s_h(i);
			y2 = input.get_Event_e_h(i);
			cv::rectangle(Lv4_bring_take_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);

			char keyname[128];
			sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_Event_model(i));
			Lv4_bring_take_event_object = cv::imread(keyname, 1);

			string name_data = "take";
			pParent->imagepack[event_key] = MultiData(name_data, "string", "take");
			name_data = Util::toString(input.get_Event_human(i));
			pParent->imagepack[human_key] = MultiData(name_data, "string", "take");
			name_data = Util::toString(input.get_Event_key(i));
			pParent->imagepack[object_key] = MultiData(name_data, "string", "take");


			pParent->imagepack[human_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_human), "image", "take");
			pParent->imagepack[object_keyimg] = MultiData(Util::convertMat2String(Lv4_bring_take_event_object), "image", "take");

			hoji_data.push_back(input.get_Event_human(i));
			hoji_flag = true;

			//DB-Lv4-----(持ち去り物体画像)、持ち去り人物ID、持ち去り物体ID
			if (HumanKinectID == 1)
			{
				Event1.push_back("Take");
			}
			if (HumanKinectID == 2)
			{
				Event2.push_back("Take");
			}
			if (HumanKinectID == 3)
			{
				Event3.push_back("Take");
			}
			if (HumanKinectID == 4)
			{
				Event4.push_back("Take");
			}
			if (HumanKinectID == 5)
			{
				Event5.push_back("Take");
			}
			if (HumanKinectID == 6)
			{
				Event6.push_back("Take");
			}
			TakeHumanKinectID.push_back(Util::toString(HumanKinectID));
			TakeObjectID.push_back(Util::toString(input.get_Event_obj(i)));
			// TakeObjectID.push_back(Util::toString(input.get_Event_key(i)));
			TakeHumanID.push_back(Util::toString(input.get_Event_human(i)));

			TakeHumanImg.push_back(Lv4_bring_take_event_human);
			TakeObjectImg.push_back(Lv4_bring_take_event_object);
		}
		cv::imshow("test", Lv4_bring_take_event);
		pParent->imagepack["Lv4-BringTakeEventImg"] = MultiData(Util::convertMat2String(Lv4_bring_take_event), "image", "Lv4-BringTakeEvent");
	}



	if (input.get_n_event_state().L4.size())
	{
		Lv4_style_detect = input.img_resize.clone();
		Lv4_style_human = cv::Mat(cv::Size(50, 50), CV_8UC3, cv::Scalar(0, 0, 0));
		for (int index = 0; index < 6; index++)
		{
			if (index < input.get_n_event_state().L4.size())
			{//，(人物ID,姿勢検知(イベント隠す1,飲む2))
				int i;
				for (i = 0; input.get_t_human_id(i).id != input.get_n_event_state().L4[index].first; i++)
				{
					if (i == input.size_t_human_id())break;
				}
				if (input.get_t_human_id(i).skeletons.size()>0)
				{
					//int indexSkeleton=input.get_t_human_id(i).skelton;
					//cout<<indexSkeleton<<endl;
					for (int indexJoint = 0; indexJoint < input.get_t_human_id(i).skeletons.size(); indexJoint++)
					{
						int x = input.get_t_human_id(i).skeletons[indexJoint].first;
						int y = input.get_t_human_id(i).skeletons[indexJoint].second;
						if (indexJoint == 21 || indexJoint == 23)
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(160, 160, 255), 6, 4);
						}
						else if (indexJoint == 3)
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(0, 255, 255), 6, 4);
						}
						else
						{
							cv::circle(Lv4_style_detect, cv::Point(x, y), 1, cv::Scalar(122, 122, 122), 2, 4);
						}

					}
				}
				char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
				string key = "Lv4-StyleDetect_" + Util::toString(index + 1);
				string name_data = Util::toString(input.get_n_event_state().L4[index].first);
				int x1 = input.get_t_human_id(i).x1;
				int x2 = input.get_t_human_id(i).x2;
				int y1 = input.get_t_human_id(i).y1;
				int cy = input.get_t_human_id(i).ycenter;
				cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, cy - y1));
				Lv4_style_human = src.clone();

				if (input.get_n_event_state().L4[index].second == 1)
				{
					//name_data+="が隠す姿勢中";
					/*	pParent->imagepack[ key ] = MultiData( name_data, "string","隠す動作" );
					string human_key=key+"_HumanImg";
					pParent->imagepack[ human_key ] = MultiData( Util::convertMat2String(Lv4_style_human), "image","隠す動作" );*/
				}
				else
				{
					//name_data+="が飲む姿勢中";
					pParent->imagepack[key] = MultiData(name_data, "string", "drink");
					pParent->imagepack[key].layout = "人物ID：";
					string human_key = key + "_HumanImg";
					pParent->imagepack[human_key] = MultiData(Util::convertMat2String(Lv4_style_human), "image", "drink");
					pParent->imagepack["Lv4-StyleDetectImg"] = MultiData(Util::convertMat2String(Lv4_style_detect), "image", " Lv4-StyleDetect");
				}

				key.clear();
				name_data.clear();
			}
			else
			{
				string key = "Lv4-StyleDetect_" + Util::toString(index + 1);
				pParent->imagepack[key] = MultiData("NULL", "string", ""); key.clear();
				key.clear();
			}
		}

	}
	//cout<<"LV5"<<endl;
	//Lv5viewer-------------------------------------------------物体保持イベント,物体接触イベント
	human_max = input.size_t_human_id();
	Lv5_have_object = input.img_resize.clone();
	for (int i = 0; i < human_max; i++)
	{

		if (input.get_t_human_id(i).have_obj.size())
		{
			int x1 = input.get_t_human_id(i).x1;
			int x2 = input.get_t_human_id(i).x2;
			int y1 = input.get_t_human_id(i).y1;
			int y2 = input.get_t_human_id(i).y2;
			cv::rectangle(Lv5_have_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 243), 1, 4);
			pParent->imagepack["Lv5-HaveObjectImg"] = MultiData(Util::convertMat2String(Lv5_have_object), "image", "保持物体情報");
			HaveHumanX.push_back(Util::toString(x1));
			HaveHumanXX.push_back(Util::toString(x2));
			HaveHumanY.push_back(Util::toString(y1));
			HaveHumanYY.push_back(Util::toString(y2));
		}

	}

	int send_hoji_num = 1;
	for (int i = 0; i < human_max; i++)
	{
		for (int hoji_num = 0; hoji_num < hoji_data.size(); hoji_num++)
		{
			int HumanKinectID = 1;
			if (hoji_data[hoji_num] == input.get_t_human_id(i).id)
			{
				//DB
				if (input.get_t_human_id(i).kinect_id == -1)
				{
					HumanKinectID = 1;
				}
				else
				{
					HumanKinectID = input.get_t_human_id(i).kinect_id;
				}

				char filenameRGB[256];  //TODO: かかった時間　およそ0.08秒　ぶれがひどい
				string key = "Lv5-HaveObject_" + Util::toString(send_hoji_num++);
				string have_object = key + "_Object";
				string have_human = key + "_Human";

				pParent->imagepack[have_human] = MultiData(Util::toString(input.get_t_human_id(i).id), "string", "保持動作");
				have_human = have_human + "Img";
				int x1 = input.get_t_human_id(i).x1;
				int x2 = input.get_t_human_id(i).x2;
				int y1 = input.get_t_human_id(i).y1;
				int cy = input.get_t_human_id(i).ycenter;
				cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, cy - y1));
				Lv5_have_object_human = src.clone();

				pParent->imagepack[have_human] = MultiData(Util::convertMat2String(Lv5_have_object_human), "image", "保持動作");

				// pParent->imagepack[have_object] = MultiData(Util::toString(input.get_t_human_id(i).have_obj[0].first), "string", "保持動作");
				pParent->imagepack[have_object] = MultiData(Util::toString(input.get_t_human_id(i).object_ID[input.get_t_human_id(i).object_ID.size() - 1]), "string", "保持動作");
				have_object = have_object + "Img";
				char keyname[128];
				sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(i).have_obj[0]);
				Lv5_have_object_object = cv::imread(keyname, 1);
				pParent->imagepack[have_object] = MultiData(Util::convertMat2String(Lv5_have_object_object), "image", "保持動作");
				//pParent->imagepack["Lv5-HaveObjectImg"] = MultiData( Util::convertMat2String(Lv5_have_object),"image", "保持物体情報");

				//DB-Lv5-----保持人物ID、保持物体ID、保持人物画像、保持物体画像
				if (HumanKinectID == 1)
				{
					Event1.push_back("Have");
				}
				if (HumanKinectID == 2)
				{
					Event2.push_back("Have");
				}
				if (HumanKinectID == 3)
				{
					Event3.push_back("Have");
				}
				if (HumanKinectID == 4)
				{
					Event4.push_back("Have");
				}
				if (HumanKinectID == 5)
				{
					Event5.push_back("Have");
				}
				if (HumanKinectID == 6)
				{
					Event6.push_back("Have");
				}
				HaveHumanKinectID.push_back(Util::toString(HumanKinectID));
				// HaveObjectID.push_back(Util::toString(input.get_t_human_id(i).have_obj[0].first));
				HaveObjectID.push_back(Util::toString(input.get_t_human_id(i).object_ID[input.get_t_human_id(i).object_ID.size() - 1]));
				HaveHumanID.push_back(Util::toString(input.get_t_human_id(i).id));

				HaveHumanImg.push_back(Lv5_have_object_human);
				HaveObjectImg.push_back(Lv5_have_object_object);
			}
		}
	}

	int object_max = input.size_t_object_id();
	Lv5_touch_object = input.img_resize.clone();
	for (int i = 0; i < object_max; i++)
	{
		int x1 = input.get_t_object_id(i).x1;
		int x2 = input.get_t_object_id(i).x2;
		int y1 = input.get_t_object_id(i).y1;
		int y2 = input.get_t_object_id(i).y2;
		cv::rectangle(Lv5_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2, 4);// -1, CV_AA

		for (int index = 0; index<input.get_n_event_state().L5_t.size(); index++)
		{
			if (input.get_n_event_state().L5_t[index].second == input.get_t_object_id(i).model_id)
			{
				cv::rectangle(Lv5_touch_object, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 3, CV_AA);
			}
			pParent->imagepack["Lv5-TouchObjectImg"] = MultiData(Util::convertMat2String(Lv5_touch_object), "image", "人物と物体の接触情報");
			//DB-Lv5-----接触人物ID、接触物体ID、接触人物画像
			TouchImg.push_back(Lv5_touch_object);
		}
		if (input.EventMAX()>0)
		{
			pParent->imagepack["Lv5-TouchObjectImg"] = MultiData(Util::convertMat2String(Lv5_touch_object), "image", "人物と物体の接触情報");
		}
	}


	//cout<<"LV6"<<endl;
	//Lv6viewer-------------------------------------------------


	if (input.get_n_event_state().L6.size())
	{
		Lv6_hi_event = input.img_resize.clone();
		for (int index = 0; index < 10; index++)
		{
			if (index < input.get_n_event_state().L6.size())
			{//移動隠す飲む動作検知(隠す1飲む2移動3、人物ID)

				string key = "Lv6-HiEvent_" + Util::toString(index + 1);

				if (input.get_n_event_state().L6[index].first == 1)//隠す
				{
					string event_key = key;//+"_Event";
					//string name_data="人物:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体"+Util::toString(input.get_n_event_state().L6_o[index].second)+"を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first)+"に隠す";
					string name_data = "hide";
					pParent->imagepack[event_key] = MultiData(name_data, "string", "hide");


					string event_humanNum = key + "_HumanNum";
					name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "hide");

					string event_ObjectNum = key + "_Object1_Num";


					//name_data="物体:"+Util::toString(input.get_n_event_state().L6_o[index].first)+"←物体:" + Util::toString(input.get_n_event_state().L6_o[index].second);
					pParent->imagepack[event_ObjectNum] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].first), "string", "hide");

					event_ObjectNum = key + "_Object2_Num";
					pParent->imagepack[event_ObjectNum] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].second), "string", "hide");

					string event_Scene = key + "_Scene1";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						HideHumanX.push_back(Util::toString(x1));
						HideHumanXX.push_back(Util::toString(x2));
						HideHumanY.push_back(Util::toString(y1));
						HideHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "hide");

					event_Scene = key + "_Scene2";
					int i = 0;
					for (i = 0; i < input.size_t_object_id(); i++)
					{
						if (input.get_n_event_state().L6_o[index].first == input.get_t_object_id(i).model_id)
						{
							break;
						}
					}
					int x1 = input.get_t_object_id(i).x1;
					int x2 = input.get_t_object_id(i).x2;
					int y1 = input.get_t_object_id(i).y1;
					int y2 = input.get_t_object_id(i).y2;
					cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
					cv::Mat arc(input.img_resize, cv::Rect(x1, y1, x2 - x1, y2 - y1));
					Lv6_hi_event_Scene2 = arc.clone();
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene2), "image", "hide");


					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].second);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					Lv6_hi_event_Scene3 = cv::imread(keyname, 1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "hide");

					//Lv6-隠す-----隠す物体ID、隠される物体ID、人物ID、隠す人物画像、隠す物画像、隠される物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Hide");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Hide");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Hide");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Hide");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Hide");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Hide");
					}
					HideHumanKinectID.push_back(Util::toString(HumanKinectID));
					HideHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					HideObjectAID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));
					HideObjectBID.push_back(Util::toString(input.get_n_event_state().L6_o[index].second));

					HideHumanImg.push_back(Lv6_hi_event_Scene1);
					HideObjectImg.push_back(Lv6_hi_event_Scene2);
					HideInObjectImg.push_back(Lv6_hi_event_Scene3);



				}
				else if (input.get_n_event_state().L6[index].first == 2)//飲む
				{
					string event_humanNum = key + "_HumanNum";
					string name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "drink");
					string event_Object1Num = key + "_Object1_Num";
					pParent->imagepack[event_Object1Num] = MultiData(Util::toString(input.get_n_event_state().L6_o[index].first), "string", "drink");

					string event_key = key;//+"_Event";
					//string name_data="人物:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体"+Util::toString(input.get_n_event_state().L6_o[index].second)+"を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first)+"に隠す";
					//name_data="物体を飲む";
					pParent->imagepack[event_key] = MultiData("飲む", "string", "drink");

					string event_Scene = key + "_Scene2";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						DrinkHumanX.push_back(Util::toString(x1));
						DrinkHumanXX.push_back(Util::toString(x2));
						DrinkHumanY.push_back(Util::toString(y1));
						DrinkHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "drink");

					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					Lv6_hi_event_Scene3 = cv::imread(keyname, 1);
					//cv::imshow("test",Lv6_hi_event_Scene3);
					//cv::waitKey(1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "drink");

					//string event_ObjectNum=key+"_ObjectNum";
					//name_data="物体:"+Util::toString(input.get_n_event_state().L6_o[index].first)+"←人物:" + Util::toString(input.get_n_event_state().L6[index].second);
					//pParent->imagepack[event_ObjectNum] = MultiData( name_data, "string", "飲む");

					//Lv6-飲む-----飲む物体ID、飲む人物ID、飲む人物画像、飲む物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Drink");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Drink");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Drink");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Drink");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Drink");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Drink");
					}
					DrinkHumanKinectID.push_back(Util::toString(HumanKinectID));
					DrinkHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					DrinkObjectID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));

					DrinkHumanImg.push_back(Lv6_hi_event_Scene1);
					DrinkObjectImg.push_back(Lv6_hi_event_Scene3);


				}
				else if (input.get_n_event_state().L6[index].first == 3)
				{
					//string name_data="人物ID:"+Util::toString(input.get_n_event_state().L6[index].second)+"が物体ID:"+ Util::toString(input.get_n_event_state().L6_o[index].first)+"の移動を検知";
					//pParent->imagepack[ key ] = MultiData( "移動", "string", "移動動作検知");

					string event_key = key;//+"_Event";
					string name_data = "人物:" + Util::toString(input.get_n_event_state().L6[index].second) + "が物体" + Util::toString(input.get_n_event_state().L6_o[index].second) + "を物体:" + Util::toString(input.get_n_event_state().L6_o[index].first) + "に隠す";
					//name_data="移動";
					pParent->imagepack[event_key] = MultiData("移動", "string", "move");


					string event_humanNum = key + "_HumanNum";
					name_data = Util::toString(input.get_n_event_state().L6[index].second);
					pParent->imagepack[event_humanNum] = MultiData(name_data, "string", "move");

					string event_ObjectNum = key + "_Object1_Num";
					name_data = Util::toString(input.get_n_event_state().L6_o[index].first);
					pParent->imagepack[event_ObjectNum] = MultiData(name_data, "string", "move");
					event_ObjectNum = key + "_Object2_Num";
					pParent->imagepack[event_ObjectNum] = MultiData(name_data, "string", "move");

					string event_Scene = key + "_Scene1";
					int num = -1;
					int HumanKinectID = 1;
					bool modelFlag = false;
					for (int t = 0; t < input.size_t_human_id(); t++)
					{
						if (input.get_n_event_state().L6[index].second == input.get_t_human_id(t).id)
						{
							//DB
							if (input.get_t_human_id(t).kinect_id == -1)
							{
								HumanKinectID = 1;
							}
							else
							{
								HumanKinectID = input.get_t_human_id(t).kinect_id;
							}
							num = t;
						}
					}
					if (num != -1)
					{
						int x1 = input.get_t_human_id(num).x1;
						int x2 = input.get_t_human_id(num).x2;
						int y1 = input.get_t_human_id(num).y1;
						int y2 = input.get_t_human_id(num).ycenter;
						cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
						cv::Mat src(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
						Lv6_hi_event_Scene1 = src.clone();
						MoveHumanX.push_back(Util::toString(x1));
						MoveHumanXX.push_back(Util::toString(x2));
						MoveHumanY.push_back(Util::toString(y1));
						MoveHumanYY.push_back(Util::toString(y2));

						if (input.get_t_human_id(num).model_ID.size() > 1){
							modelFlag = true;
						}
					}

					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene1), "image", "move");


					event_Scene = key + "_Scene3";
					char keyname[128];
					if (modelFlag == false){
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					}
					else{
						sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_t_human_id(num).model_ID[input.get_t_human_id(num).model_ID.size() - 1]);
					}
					// sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					// sprintf(keyname, "../DB/output-img/roi/roi-%05d.png", input.get_n_event_state().L6_o[index].first);
					Lv6_hi_event_Scene2 = cv::imread(keyname, 1);
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene2), "image", "move");


					event_Scene = key + "_Scene2";
					int i = 0;
					for (i = 0; i < input.size_t_object_id(); i++) { if (input.get_n_event_state().L6_o[index].second == input.get_t_object_id(i).model_id) { break; } }
					int x1 = input.get_t_object_id(i).x1;
					int x2 = input.get_t_object_id(i).x2;
					int y1 = input.get_t_object_id(i).y1;
					int y2 = input.get_t_object_id(i).y2;
					cv::rectangle(Lv6_hi_event, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 152, 243), 2, 4);
					cv::Mat arc(input.img_resize.clone(), cv::Rect(x1, y1, x2 - x1, y2 - y1));
					Lv6_hi_event_Scene3 = arc.clone();
					pParent->imagepack[event_Scene] = MultiData(Util::convertMat2String(Lv6_hi_event_Scene3), "image", "move");

					//Lv6-移動-----移動物体ID、移動人物ID、移動人物画像、移動物画像、移動後物画像
					if (HumanKinectID == 1)
					{
						Event1.push_back("Move");
					}
					if (HumanKinectID == 2)
					{
						Event2.push_back("Move");
					}
					if (HumanKinectID == 3)
					{
						Event3.push_back("Move");
					}
					if (HumanKinectID == 4)
					{
						Event4.push_back("Move");
					}
					if (HumanKinectID == 5)
					{
						Event5.push_back("Move");
					}
					if (HumanKinectID == 6)
					{
						Event6.push_back("Move");
					}
					MoveHumanKinectID.push_back(Util::toString(HumanKinectID));
					MoveHumanID.push_back(Util::toString(input.get_n_event_state().L6[index].second));
					MoveObjectID.push_back(Util::toString(input.get_n_event_state().L6_o[index].first));
					MoveCenterX.push_back(Util::toString(input.get_t_object_id(i).xcenter));
					MoveCenterY.push_back(Util::toString(input.get_t_object_id(i).ycenter));
					MoveX1.push_back(Util::toString(x1));
					MoveX2.push_back(Util::toString(x2));
					MoveY1.push_back(Util::toString(y1));
					MoveY2.push_back(Util::toString(y2));


					MoveHumanImg.push_back(Lv6_hi_event_Scene1);
					MoveObjectAfterImg.push_back(Lv6_hi_event_Scene2);
					MoveObjectBeforeImg.push_back(Lv6_hi_event_Scene3);

				}
			}
			else
			{
				string key = "Lv6-HiEvent_" + Util::toString(index + 1);
				pParent->imagepack[key] = MultiData("NULL", "string", key);
				key.clear();
			}
		}
		pParent->imagepack["Lv6-HiEventImg"] = MultiData(Util::convertMat2String(Lv6_hi_event), "image", "Lv6:HiEventImg");
	}

	string InsertALL;
	string InsertEvent;
	string InsertColumn;
	string InsertHumanIN, InsertHumanOUT, InsertHumanONnum;
	string InsertBring, InsertTake, InsertHave, InsertDrink, InsertHide, InsertMove, InsertHuman;

	//DB-Lv0-----カラー画像、深度画像、（骨格情報）
	std::vector<uchar> BufColorImg;
	std::vector<int> params(1);
	cv::imencode(".png", Lv0_depth_img, BufColorImg, params);
	string ColorImageStr(BufColorImg.begin(), BufColorImg.end());
	mysql_real_escape_string(mysql, ColorImgStr, ColorImageStr.c_str(), ColorImageStr.size());
	std::vector<uchar> BufDepthImg;
	cv::imencode(".png", Lv0_color_img, BufDepthImg, params);
	string DepthImageStr(BufDepthImg.begin(), BufDepthImg.end());
	mysql_real_escape_string(mysql, DepthImgStr, DepthImageStr.c_str(), DepthImageStr.size());


	for (int count = 0; count < HumanID.size(); count++)
	{
		if (HumanKinectID == -1)
		{
			if (HumanID.size() >= 1 && HumanID.size() <= 6)
			{
				InsertHuman +=
					",HumanIDKinectNO" + Util::toString(count + 1) + "=" + HumanID[count]
					;
			}

		}
		else
		{
			if (HumanIDKinectID[count] != "-1")
			{
				InsertHuman +=
					",HumanIDKinect" + HumanIDKinectID[count] + "=" + HumanID[count]
					;
			}
		}
	}
	if (humanOutStr.size() != 0)
	{
		InsertHumanOUT = ",humanOUTID=" + humanOutStr;
	}
	if (humanInStr.size() != 0)
	{
		InsertHumanIN = ",humanINID=" + humanInStr;
	}

	for (int count = 0; count < Event1.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "1='" + Event1[count] + "'";
	}
	for (int count = 0; count < Event2.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "2='" + Event2[count] + "'";
	}
	for (int count = 0; count < Event3.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "3='" + Event3[count] + "'";
	}
	for (int count = 0; count < Event4.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "4='" + Event4[count] + "'";
	}
	for (int count = 0; count < Event5.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "5='" + Event5[count] + "'";
	}
	for (int count = 0; count < Event6.size(); count++)
	{
		InsertEvent += ",Event" + Util::toString(count + 1) + "6='" + Event6[count] + "'";
	}


	for (int count = 0; count < BringHumanKinectID.size(); count++)
	{
		InsertBring +=
			",ObjectIDbringin" + BringHumanKinectID[count] + "=" + BringObjectID[count] + "," +
			"HumanIDbringin" + BringHumanKinectID[count] + "=" + BringHumanID[count] + "," +
			"BringCenterX" + BringHumanKinectID[count] + "=" + BringCenterX[count] + "," +
			"BringCenterY" + BringHumanKinectID[count] + "=" + BringCenterY[count] + "," +
			"BringX" + BringHumanKinectID[count] + "=" + BringX1[count] + "," +
			"BringXX" + BringHumanKinectID[count] + "=" + BringX2[count] + "," +
			"BringY" + BringHumanKinectID[count] + "=" + BringY1[count] + "," +
			"BringYY" + BringHumanKinectID[count] + "=" + BringY2[count]
			//"BringHumanImage" + BringHumanKinectID[count] + "='%s'," +
			//"BringObjectImage" + BringHumanKinectID[count] + "='%s'"
			;
		for (int count = 0; count < BringHumanX.size(); count++)
		{
			InsertBring +=
				",BringHumanX" + BringHumanKinectID[count] + "=" + BringHumanX[count] + "," +
				"BringHumanXX" + BringHumanKinectID[count] + "=" + BringHumanXX[count] + "," +
				"BringHumanY" + BringHumanKinectID[count] + "=" + BringHumanY[count] + "," +
				"BringHumanYY" + BringHumanKinectID[count] + "=" + BringHumanYY[count]
				;
		}
		cout << "Frame:" << FrameNum << ",CenterX:" << BringCenterX[count] << ",CenterY:" << BringCenterY[count] << endl;
	}

	for (int count = 0; count < TakeHumanKinectID.size(); count++)
	{
		InsertTake +=
			",ObjectIDleave" + TakeHumanKinectID[count] + "=" + TakeObjectID[count] + "," +
			"HumanIDleave" + TakeHumanKinectID[count] + "=" + TakeHumanID[count]
			//"LeaveHumanImage" + TakeHumanKinectID[count] + "='%s'," +
			//"LeaveObjectImage" + TakeHumanKinectID[count] + "='%s'"
			;
		for (int count = 0; count < TakeHumanX.size(); count++)
		{
			InsertTake +=
				",TakeHumanX" + TakeHumanKinectID[count] + "=" + TakeHumanX[count] + "," +
				"TakeHumanXX" + TakeHumanKinectID[count] + "=" + TakeHumanXX[count] + "," +
				"TakeHumanY" + TakeHumanKinectID[count] + "=" + TakeHumanY[count] + "," +
				"TakeHumanYY" + TakeHumanKinectID[count] + "=" + TakeHumanYY[count]
				;
		}
	}


	for (int count = 0; count < HaveHumanKinectID.size(); count++)
	{
		InsertHave +=
			",HaveHumanX" + HaveHumanKinectID[count] + "=" + HaveHumanX[count] + "," +
			"HaveHumanXX" + HaveHumanKinectID[count] + "=" + HaveHumanXX[count] + "," +
			"HaveHumanY" + HaveHumanKinectID[count] + "=" + HaveHumanY[count] + "," +
			"HaveHumanYY" + HaveHumanKinectID[count] + "=" + HaveHumanYY[count] + "," +
			"ObjectIDhave" + HaveHumanKinectID[count] + "=" + HaveObjectID[count] + "," +
			"HumanIDhave" + HaveHumanKinectID[count] + "=" + HaveHumanID[count]
			//"HaveHumanImage" + HaveHumanKinectID[count] + "='%s'," +
			//"HaveObjectImage" + HaveHumanKinectID[count] + "='%s'"
			;
	}

	for (int count = 0; count < HideHumanKinectID.size(); count++)
	{
		InsertHide +=
			",HideHumanX" + HideHumanKinectID[count] + "=" + HideHumanX[count] + "," +
			"HideHumanXX" + HideHumanKinectID[count] + "=" + HideHumanXX[count] + "," +
			"HideHumanY" + HideHumanKinectID[count] + "=" + HideHumanY[count] + "," +
			"HideHumanYY" + HideHumanKinectID[count] + "=" + HideHumanYY[count] + "," +
			"ObjectIDhide" + HideHumanKinectID[count] + "=" + HideObjectAID[count] + "," +
			"ObjectIDhidden" + HideHumanKinectID[count] + "=" + HideObjectBID[count] + "," +
			"HumanIDhide" + HideHumanKinectID[count] + "=" + HideHumanID[count]
			//"HideHumanImage" + HideHumanKinectID[count] + "='%s'," +
			//"HideObjectImage" + HideHumanKinectID[count] + "='%s'," +
			//"HideInObjectImage" + HideHumanKinectID[count] + "='%s'"
			;
	}
	for (int count = 0; count < DrinkHumanKinectID.size(); count++)
	{
		InsertDrink +=
			",DrinkHumanX" + DrinkHumanKinectID[count] + "=" + DrinkHumanX[count] + "," +
			"DrinkHumanXX" + DrinkHumanKinectID[count] + "=" + DrinkHumanXX[count] + "," +
			"DrinkHumanY" + DrinkHumanKinectID[count] + "=" + DrinkHumanY[count] + "," +
			"DrinkHumanYY" + DrinkHumanKinectID[count] + "=" + DrinkHumanYY[count] + "," +
			"ObjectIDdrink" + DrinkHumanKinectID[count] + "=" + DrinkObjectID[count] + "," +
			"HumanIDdrink" + DrinkHumanKinectID[count] + "=" + DrinkHumanID[count]
			//"DrinkHumanImage" + DrinkHumanKinectID[count] + "='%s'," +
			//"DrinkObjectImage" + DrinkHumanKinectID[count] + "='%s'"
			;
	}
	for (int count = 0; count < MoveHumanKinectID.size(); count++)
	{
		InsertMove +=
			",MoveHumanX" + MoveHumanKinectID[count] + "=" + MoveHumanX[count] + "," +
			"MoveHumanXX" + MoveHumanKinectID[count] + "=" + MoveHumanXX[count] + "," +
			"MoveHumanY" + MoveHumanKinectID[count] + "=" + MoveHumanY[count] + "," +
			"MoveHumanYY" + MoveHumanKinectID[count] + "=" + MoveHumanYY[count] + "," +
			"ObjectIDmove" + MoveHumanKinectID[count] + "=" + MoveObjectID[count] + "," +
			"HumanIDmove" + MoveHumanKinectID[count] + "=" + MoveHumanID[count] + "," +
			"MoveCenterX" + MoveHumanKinectID[count] + "=" + MoveCenterX[count] + "," +
			"MoveCenterY" + MoveHumanKinectID[count] + "=" + MoveCenterY[count] + "," +
			"MoveX" + MoveHumanKinectID[count] + "=" + MoveX1[count] + "," +
			"MoveXX" + MoveHumanKinectID[count] + "=" + MoveX2[count] + "," +
			"MoveY" + MoveHumanKinectID[count] + "=" + MoveY1[count] + "," +
			"MoveYY" + MoveHumanKinectID[count] + "=" + MoveY2[count]
			//"MoveHumanImage" + MoveHumanKinectID[count] + "=%s," +
			//"MoveAfterObjectImage" + MoveHumanKinectID[count] + "='%s'," +
			//"MoveBeforeObjectImage" + MoveHumanKinectID[count] + "='%s'"
			;
	}
	string EventFlag;
	if (InsertBring.size() != 0 || InsertTake.size() != 0 || InsertHave.size() != 0 || InsertDrink.size() != 0 || InsertHide.size() != 0 || InsertMove.size() != 0)
	{
		EventFlag = "true";
	}
	else
	{
		EventFlag = "false";
	}
	if (InsertHuman.size() != 0 || InsertBring.size() != 0 || InsertTake.size() != 0 || InsertHave.size() != 0 || InsertDrink.size() != 0 || InsertHide.size() != 0 || InsertMove.size() != 0)
	{
		InsertColumn = InsertHumanIN + InsertHumanOUT + InsertEvent + InsertHuman + InsertBring + InsertTake + InsertHave + InsertDrink + InsertHide + InsertMove;

		InsertALL =
			"INSERT  MainTable SET "
			"FrameCount=" + FrameNum + ",ID=" + Util::toString(sceneID) + ",SceneFrame=" + Util::toString(sceneFrame) +
			",EventFlag='" + EventFlag + "',CameraID='meeting'"
			",humanONnum=" + Util::toString(humanONnum) +
			",ColorImage='%s',DepthImage='%s'" + InsertColumn + ";"
			;

		//mysql_real_queryを使用するためにqueryの長さが必要
		mysqlSize = sprintf_s(query, sizeof(DepthImgStr), InsertALL.c_str(), ColorImgStr, DepthImgStr);
		if (mysql_real_query(mysql, query, mysqlSize) != 0)
		{
			//cout << "query_error" << endl;
		}
	}
	HumanCount = input.size_t_human_id();
}

void  CCamera::DBConnect()
{
	//初期化
	mysql = mysql_init( NULL );

	//MySQLに必要なユーザー情報定義(必要に応じて変更する)
	
	//IP old
	/*const char *hostname = "133.19.23.225";
	const char *username = "kanshi";
	const char *password = "kanshi123";
	const char *database = NULL;*/

	//OS dead
	/*const char *hostname = "10.40.1.105";
	const char *username = "kanshi";
	const char *password = "kanshi123";
	const char *database = NULL;*/
	
	//cannot display
	const char *hostname = "10.40.0.53";
	const char *username = "yatsuduka";
	const char *password = "interface123";
	const char *database = NULL;

	/*const char *hostname = "10.40.0.53";
	const char *username = "root";
	const char *password = "interface123";
	const char *database = NULL;*/

	unsigned long portnumber = 3306;

	//mysql_real_connect MySQLに接続
	if ( NULL == mysql_real_connect( mysql, hostname, username, password, database, portnumber, NULL, 0 ) )
	{
		// 接続エラー
		printf( "error1: %s\n", mysql_error( mysql ) );
	}
	else
	{
		cout << "MySQLConnect" << endl;
		//  接続成功

		//データベース名を入力し、データベースを作成
		string dbname;
		if( autoExecute ){
			dbname = DBname;
		}
		else{
			cout << "create database:";
			cin >> dbname;
		}

		string create = "CREATE DATABASE " + dbname;
		//クエリ発行
		if ( mysql_query( mysql, create.c_str() ) != 0 )
		{//データベース作成SQL
			//fprintf( stderr, "%s\n", mysql_error( mysql ) );
			cout << mysql_error( mysql ) << endl;
			//exit(1);
		}

		//データベースの選択
		if ( mysql_select_db( mysql, dbname.c_str() ) != 0 )
		{
			printf( "cannot create database \n" );
			//exit(1);
		}

		//メインテーブル作成
		string maintable;
		maintable =
			"CREATE TABLE MainTable("

			//フレームカウント、シーンID、シーン内のフレーム
			"FrameCount int(15),ID int(5) ,SceneFrame int(15)"

			//人の出入り
			",humanINID varchar(5),humanONID int(5),humanONnum int(5),humanOUTID varchar(5)"

			//時間、場所
			",TIME timestamp,CameraID varchar(20)"

			//カラー画像、深度画像
			",ColorImage MEDIUMBLOB,DepthImage MEDIUMBLOB"

			// 物体名
			",ObjectNamebringin1 varchar(20),ObjectNamebringin2 varchar(20),ObjectNamebringin3 varchar(20),ObjectNamebringin4 varchar(20),ObjectNamebringin5 varchar(20),ObjectNamebringin6 varchar(20)"

			//event
			",Eventflag varchar(5),Event11 varchar(15),Event12 varchar(15),Event13 varchar(15),Event14 varchar(15),Event15 varchar(15),Event16 varchar(15)"
			",Event21 varchar(15),Event22 varchar(15),Event23 varchar(15),Event24 varchar(15),Event25 varchar(15),Event26 varchar(15)"
			",Event31 varchar(15),Event32 varchar(15),Event33 varchar(15),Event34 varchar(15),Event35 varchar(15),Event36 varchar(15)"
			",Event41 varchar(15),Event42 varchar(15),Event43 varchar(15),Event44 varchar(15),Event45 varchar(15),Event46 varchar(15)"
			",Event51 varchar(15),Event52 varchar(15),Event53 varchar(15),Event54 varchar(15),Event55 varchar(15),Event56 varchar(15)"
			",Event61 varchar(15),Event62 varchar(15),Event63 varchar(15),Event64 varchar(15),Event65 varchar(15),Event66 varchar(15)"
			//touch
			",ObjectTouchImage MEDIUMBLOB"

			//have
			",ObjectIDhave1 varchar(5),ObjectIDhave2 varchar(5),ObjectIDhave3 varchar(5),ObjectIDhave4 varchar(5),ObjectIDhave5 varchar(5),ObjectIDhave6 varchar(5)"
			",HumanIDhave1 varchar(5),HumanIDhave2 varchar(5),HumanIDhave3 varchar(5),HumanIDhave4 varchar(5),HumanIDhave5 varchar(5),HumanIDhave6 varchar(5)"
			",HaveHumanX1 varchar(5),HaveHumanX2 varchar(5),HaveHumanX3 varchar(5),HaveHumanX4 varchar(5),HaveHumanX5 varchar(5),HaveHumanX6 varchar(5)"
			",HaveHumanXX1 varchar(5),HaveHumanXX2 varchar(5),HaveHumanXX3 varchar(5),HaveHumanXX4 varchar(5),HaveHumanXX5 varchar(5),HaveHumanXX6 varchar(5)"
			",HaveHumanY1 varchar(5),HaveHumanY2 varchar(5),HaveHumanY3 varchar(5),HaveHumanY4 varchar(5),HaveHumanY5 varchar(5),HaveHumanY6 varchar(5)"
			",HaveHumanYY1 varchar(5),HaveHumanYY2 varchar(5),HaveHumanYY3 varchar(5),HaveHumanYY4 varchar(5),HaveHumanYY5 varchar(5),HaveHumanYY6 varchar(5)"

			",HaveHumanImage1 BLOB,HaveHumanImage2 BLOB,HaveHumanImage3 BLOB,HaveHumanImage4 BLOB,HaveHumanImage5 BLOB,HaveHumanImage6 BLOB"
			",HaveObjectImage1 BLOB,HaveObjectImage2 BLOB,HaveObjectImage3 BLOB,HaveObjectImage4 BLOB,HaveObjectImage5 BLOB,HaveObjectImage6 BLOB"

			//bringin
			",ObjectIDbringin1 varchar(5),ObjectIDbringin2 varchar(5),ObjectIDbringin3 varchar(5),ObjectIDbringin4 varchar(5),ObjectIDbringin5 varchar(5),ObjectIDbringin6 varchar(5)"
			",HumanIDbringin1 varchar(5),HumanIDbringin2 varchar(5),HumanIDbringin3 varchar(5),HumanIDbringin4 varchar(5),HumanIDbringin5 varchar(5),HumanIDbringin6 varchar(5)"
			",BringHumanX1 varchar(5),BringHumanX2 varchar(5),BringHumanX3 varchar(5),BringHumanX4 varchar(5),BringHumanX5 varchar(5),BringHumanX6 varchar(5)"
			",BringHumanXX1 varchar(5),BringHumanXX2 varchar(5),BringHumanXX3 varchar(5),BringHumanXX4 varchar(5),BringHumanXX5 varchar(5),BringHumanXX6 varchar(5)"
			",BringHumanY1 varchar(5),BringHumanY2 varchar(5),BringHumanY3 varchar(5),BringHumanY4 varchar(5),BringHumanY5 varchar(5),BringHumanY6 varchar(5)"
			",BringHumanYY1 varchar(5),BringHumanYY2 varchar(5),BringHumanYY3 varchar(5),BringHumanYY4 varchar(5),BringHumanYY5 varchar(5),BringHumanYY6 varchar(5)"
			",BringCenterX1 varchar(5),BringCenterX2 varchar(5),BringCenterX3 varchar(5),BringCenterX4 varchar(5),BringCenterX5 varchar(5),BringCenterX6 varchar(5)"
			",BringCenterY1 varchar(5),BringCenterY2 varchar(5),BringCenterY3 varchar(5),BringCenterY4 varchar(5),BringCenterY5 varchar(5),BringCenterY6 varchar(5)"
			",BringX1 varchar(5),BringX2 varchar(5),BringX3 varchar(5),BringX4 varchar(5),BringX5 varchar(5),BringX6 varchar(5)"
			",BringXX1 varchar(5),BringXX2 varchar(5),BringXX3 varchar(5),BringXX4 varchar(5),BringXX5 varchar(5),BringXX6 varchar(5)"
			",BringY1 varchar(5),BringY2 varchar(5),BringY3 varchar(5),BringY4 varchar(5),BringY5 varchar(5),BringY6 varchar(5)"
			",BringYY1 varchar(5),BringYY2 varchar(5),BringYY3 varchar(5),BringYY4 varchar(5),BringYY5 varchar(5),BringYY6 varchar(5)"

			",BringHumanImage1 BLOB,BringHumanImage2 BLOB,BringHumanImage3 BLOB,BringHumanImage4 BLOB,BringHumanImage5 BLOB,BringHumanImage6 BLOB"
			",BringObjectImage1 BLOB,BringObjectImage2 BLOB,BringObjectImage3 BLOB,BringObjectImage4 BLOB,BringObjectImage5 BLOB,BringObjectImage6 BLOB"

			//leave
			",ObjectIDleave1 varchar(5),ObjectIDleave2 varchar(5),ObjectIDleave3 varchar(5),ObjectIDleave4 varchar(5),ObjectIDleave5 varchar(5),ObjectIDleave6 varchar(5)"
			",HumanIDleave1 varchar(5),HumanIDleave2 varchar(5),HumanIDleave3 varchar(5),HumanIDleave4 varchar(5),HumanIDleave5 varchar(5),HumanIDleave6 varchar(5)"
			",TakeHumanX1 varchar(5),TakeHumanX2 varchar(5),TakeHumanX3 varchar(5),TakeHumanX4 varchar(5),TakeHumanX5 varchar(5),TakeHumanX6 varchar(5)"
			",TakeHumanXX1 varchar(5),TakeHumanXX2 varchar(5),TakeHumanXX3 varchar(5),TakeHumanXX4 varchar(5),TakeHumanXX5 varchar(5),TakeHumanXX6 varchar(5)"
			",TakeHumanY1 varchar(5),TakeHumanY2 varchar(5),TakeHumanY3 varchar(5),TakeHumanY4 varchar(5),TakeHumanY5 varchar(5),TakeHumanY6 varchar(5)"
			",TakeHumanYY1 varchar(5),TakeHumanYY2 varchar(5),TakeHumanYY3 varchar(5),TakeHumanYY4 varchar(5),TakeHumanYY5 varchar(5),TakeHumanYY6 varchar(5)"

			",LeaveHumanImage1 BLOB,LeaveHumanImage2 BLOB,LeaveHumanImage3 BLOB,LeaveHumanImage4 BLOB,LeaveHumanImage5 BLOB,LeaveHumanImage6 BLOB"
			",LeaveObjectImage1 BLOB,LeaveObjectImage2 BLOB,LeaveObjectImage3 BLOB,LeaveObjectImage4 BLOB,LeaveObjectImage5 BLOB,LeaveObjectImage6 BLOB"

			//drink
			",ObjectIDdrink1 varchar(5),ObjectIDdrink2 varchar(5),ObjectIDdrink3 varchar(5),ObjectIDdrink4 varchar(5),ObjectIDdrink5 varchar(5),ObjectIDdrink6 varchar(5)"
			",HumanIDdrink1 varchar(5),HumanIDdrink2 varchar(5),HumanIDdrink3 varchar(5),HumanIDdrink4 varchar(5),HumanIDdrink5 varchar(5),HumanIDdrink6 varchar(5)"
			",DrinkHumanX1 varchar(5),DrinkHumanX2 varchar(5),DrinkHumanX3 varchar(5),DrinkHumanX4 varchar(5),DrinkHumanX5 varchar(5),DrinkHumanX6 varchar(5)"
			",DrinkHumanXX1 varchar(5),DrinkHumanXX2 varchar(5),DrinkHumanXX3 varchar(5),DrinkHumanXX4 varchar(5),DrinkHumanXX5 varchar(5),DrinkHumanXX6 varchar(5)"
			",DrinkHumanY1 varchar(5),DrinkHumanY2 varchar(5),DrinkHumanY3 varchar(5),DrinkHumanY4 varchar(5),DrinkHumanY5 varchar(5),DrinkHumanY6 varchar(5)"
			",DrinkHumanYY1 varchar(5),DrinkHumanYY2 varchar(5),DrinkHumanYY3 varchar(5),DrinkHumanYY4 varchar(5),DrinkHumanYY5 varchar(5),DrinkHumanYY6 varchar(5)"

			",DrinkHumanImage1 BLOB,DrinkHumanImage2 BLOB,DrinkHumanImage3 BLOB,DrinkHumanImage4 BLOB,DrinkHumanImage5 BLOB,DrinkHumanImage6 BLOB"
			",DrinkObjectImage1 BLOB,DrinkObjectImage2 BLOB,DrinkObjectImage3 BLOB,DrinkObjectImage4 BLOB,DrinkObjectImage5 BLOB,DrinkObjectImage6 BLOB"

			//hide
			",ObjectIDhide1 varchar(5),ObjectIDhide2 varchar(5),ObjectIDhide3 varchar(5),ObjectIDhide4 varchar(5),ObjectIDhide5 varchar(5),ObjectIDhide6 varchar(5)"
			",ObjectIDhidden1 varchar(5),ObjectIDhidden2 varchar(5),ObjectIDhidden3 varchar(5),ObjectIDhidden4 varchar(5),ObjectIDhidden5 varchar(5),ObjectIDhidden6 varchar(5)"
			",HumanIDhide1 varchar(5),HumanIDhide2 varchar(5),HumanIDhide3 varchar(5),HumanIDhide4 varchar(5),HumanIDhide5 varchar(5),HumanIDhide6 varchar(5)"
			",HideHumanX1 varchar(5),HideHumanX2 varchar(5),HideHumanX3 varchar(5),HideHumanX4 varchar(5),HideHumanX5 varchar(5),HideHumanX6 varchar(5)"
			",HideHumanXX1 varchar(5),HideHumanXX2 varchar(5),HideHumanXX3 varchar(5),HideHumanXX4 varchar(5),HideHumanXX5 varchar(5),HideHumanXX6 varchar(5)"
			",HideHumanY1 varchar(5),HideHumanY2 varchar(5),HideHumanY3 varchar(5),HideHumanY4 varchar(5),HideHumanY5 varchar(5),HideHumanY6 varchar(5)"
			",HideHumanYY1 varchar(5),HideHumanYY2 varchar(5),HideHumanYY3 varchar(5),HideHumanYY4 varchar(5),HideHumanYY5 varchar(5),HideHumanYY6 varchar(5)"

			",HideHumanImage1 BLOB,HideHumanImage2 BLOB,HideHumanImage3 BLOB,HideHumanImage4 BLOB,HideHumanImage5 BLOB,HideHumanImage6 BLOB"
			",HideObjectImage1 BLOB,HideObjectImage2 BLOB,HideObjectImage3 BLOB,HideObjectImage4 BLOB,HideObjectImage5 BLOB,HideObjectImage6 BLOB"
			",HideInObjectImage1 BLOB,HideInObjectImage2 BLOB,HideInObjectImage3 BLOB,HideInObjectImage4 BLOB,HideInObjectImage5 BLOB,HideInObjectImage6 BLOB"

			//move
			",ObjectIDmove1 varchar(5),ObjectIDmove2 varchar(5),ObjectIDmove3 varchar(5),ObjectIDmove4 varchar(5),ObjectIDmove5 varchar(5),ObjectIDmove6 varchar(5)"
			",HumanIDmove1 varchar(5),HumanIDmove2 varchar(5),HumanIDmove3 varchar(5),HumanIDmove4 varchar(5),HumanIDmove5 varchar(5),HumanIDmove6 varchar(5)"
			",MoveCenterX1 varchar(5),MoveCenterX2 varchar(5),MoveCenterX3 varchar(5),MoveCenterX4 varchar(5),MoveCenterX5 varchar(5),MoveCenterX6 varchar(5)"
			",MoveCenterY1 varchar(5),MoveCenterY2 varchar(5),MoveCenterY3 varchar(5),MoveCenterY4 varchar(5),MoveCenterY5 varchar(5),MoveCenterY6 varchar(5)"
			",MoveX1 varchar(5),MoveX2 varchar(5),MoveX3 varchar(5),MoveX4 varchar(5),MoveX5 varchar(5),MoveX6 varchar(5)"
			",MoveXX1 varchar(5),MoveXX2 varchar(5),MoveXX3 varchar(5),MoveXX4 varchar(5),MoveXX5 varchar(5),MoveXX6 varchar(5)"
			",MoveY1 varchar(5),MoveY2 varchar(5),MoveY3 varchar(5),MoveY4 varchar(5),MoveY5 varchar(5),MoveY6 varchar(5)"
			",MoveYY1 varchar(5),MoveYY2 varchar(5),MoveYY3 varchar(5),MoveYY4 varchar(5),MoveYY5 varchar(5),MoveYY6 varchar(5)"
			",MoveHumanX1 varchar(5),MoveHumanX2 varchar(5),MoveHumanX3 varchar(5),MoveHumanX4 varchar(5),MoveHumanX5 varchar(5),MoveHumanX6 varchar(5)"
			",MoveHumanXX1 varchar(5),MoveHumanXX2 varchar(5),MoveHumanXX3 varchar(5),MoveHumanXX4 varchar(5),MoveHumanXX5 varchar(5),MoveHumanXX6 varchar(5)"
			",MoveHumanY1 varchar(5),MoveHumanY2 varchar(5),MoveHumanY3 varchar(5),MoveHumanY4 varchar(5),MoveHumanY5 varchar(5),MoveHumanY6 varchar(5)"
			",MoveHumanYY1 varchar(5),MoveHumanYY2 varchar(5),MoveHumanYY3 varchar(5),MoveHumanYY4 varchar(5),MoveHumanYY5 varchar(5),MoveHumanYY6 varchar(5)"


			",MoveHumanImage1 BLOB,MoveHumanImage2 BLOB,MoveHumanImage3 BLOB,MoveHumanImage4 BLOB,MoveHumanImage5 BLOB,MoveHumanImage6 BLOB"
			",MoveAfterObjectImage1 BLOB,MoveAfterObjectImage2 BLOB,MoveAfterObjectImage3 BLOB,MoveAfterObjectImage4 BLOB,MoveAfterObjectImage5 BLOB,MoveAfterObjectImage6 BLOB"
			",MoveBeforeObjectImage1 BLOB,MoveBeforeObjectImage2 BLOB,MoveBeforeObjectImage3 BLOB,MoveBeforeObjectImage4 BLOB,MoveBeforeObjectImage5 BLOB,MoveBeforeObjectImage6 BLOB"

			//humanID
			",HumanIDKinect1 int(5),HumanIDKinect2 int(5),HumanIDKinect3 int(5),HumanIDKinect4 int(5),HumanIDKinect5 int(5),HumanIDKinect6 int(5)"
			",HumanIDKinectNO1 int (5),HumanIDKinectNO2 int (5),HumanIDKinectNO3 int (5),HumanIDKinectNO4 int (5),HumanIDKinectNO5 int (5),HumanIDKinectNO6 int (5)"
			//skeleton
			",SkeletonX1 varchar(200) ,SkeletonX2 varchar(200) ,SkeletonX3 varchar(200) ,SkeletonX4 varchar(200) ,SkeletonX5 varchar(200) ,SkeletonX6 varchar(200) "
			",SkeletonY1 varchar(200) ,SkeletonY2 varchar(200) ,SkeletonY3 varchar(200) ,SkeletonY4 varchar(200) ,SkeletonY5 varchar(200) ,SkeletonY6 varchar(200) "
			",SkeletonDepth1 varchar(200) ,SkeletonDepth2 varchar(200) ,SkeletonDepth3 varchar(200) ,SkeletonDepth4 varchar(200) ,SkeletonDepth5 varchar(200) ,SkeletonDepth6 varchar(200)"

			//name
			",NameKinectNO varchar(15),NameKinect1 varchar(15),NameKinect2 varchar(15),NameKinect3 varchar(15),NameKinect4 varchar(15),NameKinect5 varchar(15),NameKinect6 varchar(15))";

		if ( mysql_query( mysql, maintable.c_str() ) != 0 )
		{//メインテーブル作成SQL
			//fprintf( stderr, "%s\n", mysql_error( mysql ) );
			cout << mysql_error( mysql ) << endl;
			//exit(1);
		}// DEFAULT '0000-00-00 00:00:00

	}
}