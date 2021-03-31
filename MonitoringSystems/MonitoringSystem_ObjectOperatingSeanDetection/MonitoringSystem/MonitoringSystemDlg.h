// MonitoringSystemDlg.h : ヘッダー ファイル
//

#pragma once
//#include "bmp.h"
#include "network.h"
#include "MonitoringSystem.h"
//#include "bmp.h"
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "afxwin.h"
#include "MultiData.h"
//#include"EventSearch.h"

typedef struct _record{
	int id;
	int key_point_parent_id;
	int frame;
	int cx;
	int cy;
	char hour[10];
	char minute[10];
	int human;
	bool scene;  //true:持ち込み false:持ち去り
	bool track;  //true:移動     false:持ち込み
} record;

// CMonitoringSystemDlg ダイアログ
//enum ModeType{ONLINE,OFFLINE};
class CMonitoringSystemDlg : public CDialog
{
private:
	//CEdit detect;  //検知画像描画領域
	BITMAPINFO bmpInfo; //bmpファイル用
	BITMAPINFO bmpInfoThumbnail;
	PAINTSTRUCT ps;

	HWND hwndImage;  //リアルタイム画像出力用領域
	HDC hdcImage;    //デバイスコンテキストのハンドル

	HWND hwndDiff; //差分画像出力用領域
	HDC hdcDiff;   //デバイスコンテキストのハンドル

	//HWND hwndPosi;  //keypoint画像出力用領域
	//HDC hdcPosi;    //デバイスコンテキストのハンドル

	HWND hwndDetect1; //検知画像出力用領域
	HDC hdcDetect1;   //デバイスコンテキストのハンドル
	HWND hwndDetect2; //検知画像出力用領域
	HDC hdcDetect2;   //デバイスコンテキストのハンドル
	HWND hwndDetect3; //検知画像出力用領域
	HDC hdcDetect3;   //デバイスコンテキストのハンドル

	HWND hwndLayer; //検知画像出力用領域
	HDC hdcLayer;   //デバイスコンテキストのハンドル

	HWND hwndCamera; //検知画像出力用領域
	HDC hdcCamera;   //デバイスコンテキストのハンドル

	HWND hwndThumnail; //検知画像出力用領域
	HDC hdcThumnail;   //デバイスコンテキストのハンドル

	HWND hwndObjectID;  //リアルタイム画像出力用領域
	HDC hdcObjectID;    //デバイスコンテキストのハンドル

	HWND hwndtrack;  //追跡確認用画像出力用領域
	HDC hdctrack;    //デバイスコンテキストのハンドル

	HWND hwnddepth;  //距離画像用画像出力用領域
	HDC hdcdepth;    //デバイスコンテキストのハンドル

	HWND hwndskelton;  //骨格画像用画像出力用領域
	HDC hdcskelton;    //デバイスコンテキストのハンドル

	CEdit detect1; //文字描画領域
	CEdit detect2;
	CEdit detect3;
	CFont font;   //表示フォント

	record obj[10];
	int count;     //物体数用（最大10）
	int tmp_count; //履歴を見る用
	bool disp_flag;     //履歴の表示が最新かどうか

	int mode; //オンラインorオフライン

	int track_frame;

public:

	std::map<std::string, MultiData> imagepack;

	CString BrowseForFolder( HWND p_hWnd, CString p_cSetStr
		, CString p_cRootStr, CString p_cCaptionStr, UINT p_uiFlags );


	void Disp_Log(char *msg);           //ダイアログに検知状況を出力
	void SceneOut(int num,int flag);  //検知画像にシーン解釈を追加

	//シーン状況のセット
	void SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene, int track_flag,int key_point_parent_id,int human); 

	void ImageOut(unsigned char *pix);   //リアルタイム画像出力
	void DiffOut(unsigned char *pix);    //差分画像出力
	//void PosiOut(unsigned char *pix);   //keypoint画像出力
	void Detect1Out(unsigned char *pix, int srcX  = 0, int srcY = 0); //検知画像出力
	void Detect2Out(unsigned char *pix, int srcX  = 0, int srcY = 0); //検知画像出力
	void Detect3Out(unsigned char *pix, int srcX  = 0, int srcY = 0); //検知画像出力

	void LayerOut(unsigned char *pix); //検知画像出力

	void showCamera(unsigned char *pix); //検知画像出力
	void showThumnail(unsigned char *pix); //検知画像出力

	void ImageObjectID(unsigned char *pix);
	void Imagetrack(unsigned char *pix);
	void Imagedepth(unsigned char *pix);
	void ImageSkelton(unsigned char *pix);

	void DetectOut();
	void DetectLoad(int num,int id,int flag, int cx = 0, int cy = 0);


	void Init(); //色々初期化
	unsigned char* getImageFromImageFile(CString strPath);
	void showNCamera(int n);
	int getMode();
	// コンストラクション
public:
	CMonitoringSystemDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

	void setStrFps(CString);
	CString getStrDirPathOfflineInput();

	// ダイアログ データ
	enum { IDD = IDD_MONITORINGSYSTEM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


	// 実装
protected:
	HICON m_hIcon;




	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOnline();
public:
	afx_msg void OnBnClickedOffline();
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnBnClickedButton2();
public:
	afx_msg void OnBnClickedButton3();
	CString m_str_fps;
	int m_cameraId;
	CComboBox m_control_CameraList;
	afx_msg void OnCbnSelchangeCombo2();
	CStatic m_control_cameraWindow;
	afx_msg void OnBnClickedButton4();
	CString m_str_dirPath_offlineInput;
	afx_msg void OnBnClickedButtonSelectdir();
	afx_msg void OnBnClickedButton5();
};
