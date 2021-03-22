//-------------------------------------------------
// FileName : MonitoringSystemDlg.cpp
// Context  : インプリメンテーション ファイル
//
// Author   : Kazuhiro Maki (CV-lab.)
// Updata   : 2009/02/27
//-------------------------------------------------
#pragma execution_character_set("utf-8")
#include "stdafx.h"
#include "MonitoringSystem.h"
#include "MonitoringSystemDlg.h"
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int CameraNumber;  //使用カメラ番号
CCamera camera;//カメラ制御クラス


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog{
public:
	CAboutDlg();

	// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD){

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX){
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMonitoringSystemDlg ダイアログ




CMonitoringSystemDlg::CMonitoringSystemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitoringSystemDlg::IDD, pParent)
	, m_str_fps(_T(""))
	, m_cameraId(0)
	, mode( MODE_NONE )
	, m_str_dirPath_offlineInput(_T("")){
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMonitoringSystemDlg::setStrFps(CString str_fps){
	m_str_fps = str_fps;
	UpdateData(FALSE);	
}
CString CMonitoringSystemDlg::getStrDirPathOfflineInput(){
	return m_str_dirPath_offlineInput;
}

void CMonitoringSystemDlg::DoDataExchange(CDataExchange* pDX){
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_SCENE1,detect1);  //detect領域
	DDX_Control(pDX,IDC_SCENE2,detect2);
	DDX_Control(pDX,IDC_SCENE3,detect3);
	DDX_Text(pDX, IDC_STATIC_FPS, m_str_fps);
	DDX_CBIndex(pDX, IDC_COMBO2, m_cameraId);
	DDX_Control(pDX, IDC_COMBO2, m_control_CameraList);
	DDX_Control(pDX, IDC_CAMERA, m_control_cameraWindow);
	DDX_Text(pDX, IDC_EDIT2, m_str_dirPath_offlineInput);
}

BEGIN_MESSAGE_MAP(CMonitoringSystemDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ONLINE, &CMonitoringSystemDlg::OnBnClickedOnline)
	ON_BN_CLICKED(IDC_OFFLINE, &CMonitoringSystemDlg::OnBnClickedOffline)
	ON_BN_CLICKED(IDC_BUTTON1, &CMonitoringSystemDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMonitoringSystemDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMonitoringSystemDlg::OnBnClickedButton3)
	//ON_STN_CLICKED(IDC_IMAGE, &CMonitoringSystemDlg::OnStnClickedImage)
	//ON_STN_CLICKED(IDC_SURF, &CMonitoringSystemDlg::OnStnClickedSurf)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CMonitoringSystemDlg::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON4, &CMonitoringSystemDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON_SELECTDIR, &CMonitoringSystemDlg::OnBnClickedButtonSelectdir)
	ON_BN_CLICKED(IDC_BUTTON5, &CMonitoringSystemDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CMonitoringSystemDlg メッセージ ハンドラ


//スレッド(オンライン)
static UINT AFX_CDECL ThreadProcCalc(LPVOID pParam){
	CCamera *cp;      //カメラ制御クラス(スレッド用)
	cp=(CCamera*)pParam;
	cp->SendCommand(CameraNumber,5);
	return 0;
}

//スレッド(オフライン)
static UINT AFX_CDECL ThreadOffLine(LPVOID pParam){
	CCamera *cp;      //カメラ制御クラス(スレッド用)
	cp=(CCamera*)pParam;
	cp->OffLine();
	return 0;
}

BOOL CMonitoringSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。
	Init();
	//GetDlgItem(IDC_BUTTON_FOLDER)->EnableWindow(FALSE);


	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CMonitoringSystemDlg::OnSysCommand(UINT nID, LPARAM lParam){
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CMonitoringSystemDlg::OnPaint(){
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CMonitoringSystemDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}

//ダイアログ出力

//------------------------------------------------------
// Name     : Disp_Log(char *msg)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.05.22
// Function : LOG に文字列を出力
// Argument : 出力する文字列
// return   : なし
//------------------------------------------------------
void CMonitoringSystemDlg::Disp_Log(char *msg){
	((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel("\r\n");
	((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel(msg);
}


//ボタン処理

//-------------------------------------------------
// Name     : OnBnClickedOnline()
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2006.07.26
// Function : ネットワークカメラ3から動画像取得
// Argument : なし
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::OnBnClickedOnline(){
	//他のカメラが使用中の場合
	/*if(CameraNumber>0){
	cp->set_stop();
	Sleep(200);
	}*/
	std::cout<<"Online Start" <<std::endl;
	GetDlgItem(IDC_OFFLINE)->EnableWindow(FALSE);
	//CameraNumber = 1;  //使用カメラの設定
	if( mode == MODE_NONE )
	{
		mode = MODE_ONLINE;     //モードの設定
		CWnd::SetDlgItemTextA(IDC_ONLINE, "STOP");
	}
	else if( mode == MODE_ONLINE )
	{
		mode = MODE_NONE;
		CWnd::SetDlgItemTextA(IDC_ONLINE, "ONLINE");
	}
	//このクラスのポインタを渡す
	camera.set_pParent(this);


	//スレッドを用いてカメラから画像を取得
	//CWinThread* pThread = 
	AfxBeginThread(ThreadProcCalc, &camera,THREAD_PRIORITY_BELOW_NORMAL);
	//	GetDlgItem(IDC_OFFLINE)->EnableWindow(TRUE);
	/*	DWORD    dwExitCode=STILL_ACTIVE;
	while(dwExitCode == STILL_ACTIVE){
	GetExitCodeThread(pThread,&dwExitCode);
	}
	std::cout<<"Online end" <<std::endl;
	*/
}

//-------------------------------------------------
// Name     : OnBnClickedOffline()
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2006.07.26
// Function : オフラインで処理
// Argument : なし
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::OnBnClickedOffline(){
	mode = MODE_OFFLINE;  //モードの設定
	GetDlgItem(IDC_OFFLINE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ONLINE)->EnableWindow(FALSE);
	//このクラスのポインタを渡す
	camera.set_pParent(this);

	//スレッドを用いてカメラから画像を取得
	AfxBeginThread(ThreadOffLine,&camera,THREAD_PRIORITY_BELOW_NORMAL);
}

//-------------------------------------------------
// Name     : ImageOut(unsigned char *pix)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : リアルタイム画像出力
// Argument : pix:出力画像データ
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::ImageOut(unsigned char *pix){
	//画像出力
	::BeginPaint(hwndImage,&ps);//指定されたウィンドウに対して描画の準備(HWND  hwnd,//  ウィンドウのハンドル LPPAINTSTRUCT  lpPaint  //  描画情報を持つ構造体へのポインタ);
	SetStretchBltMode(hdcImage,COLORONCOLOR);//指定されたデバイス独立ビットマップ（DIB）内の長方形ピクセルの色データを、指定された長方形へコピー(HDC hdc,// デバイスコンテキストのハンドル int iStretchMode // ビットマップの伸縮モードピクセルを削除します。取り除く点の情報を何らかの形で維持せようとはせず、単純にそれらの点を削除します。);
	StretchDIBits(hdcImage,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// デバイスコンテキストのハンドルint XDest, // コピー先長方形の左上隅の x 座標 int YDest,// コピー先長方形の左上隅の y 座標int nDestWidth, // コピー先長方形の幅int nDestHeight,// コピー先長方形の高さint XSrc,// コピー元長方形の左上隅の x 座標int YSrc,// コピー元長方形の左上隅の x 座標int nSrcWidth,// コピー元長方形の幅int nSrcHeight,// コピー元長方形の高さ
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// ビットマップのビットCONST BITMAPINFO *lpBitsInfo, // ビットマップのデータUINT iUsage,// データの種類 DWORD dwRop// ラスタオペレーションコード);
	::EndPaint(hwndImage,&ps);
}

//-------------------------------------------------
// Name     : DiffOut(unsigned char *pix)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : 差分画像出力
// Argument : pix:出力画像データ
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::DiffOut(unsigned char *pix){
	//画像出力
	::BeginPaint(hwndDiff,&ps);
	SetStretchBltMode(hdcDiff,COLORONCOLOR);
	StretchDIBits(hdcDiff,0,0,320,240,0,0,320,240,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndDiff,&ps);
}

//-------------------------------------------------
// Name     : SurfOut(unsigned char *pix)
// Author   : Kazuhiro OKAMOTO (CV-lab.)
// Updata   : 2011.12.26
// Function : keypoint画像出力
// Argument : pix:出力画像データ
// return   : なし
//-------------------------------------------------
//void CMonitoringSystemDlg::PosiOut(unsigned char *pix)
//{
//	//画像出力
//	::BeginPaint(hwndPosi,&ps);
//		SetStretchBltMode(hdcPosi,COLORONCOLOR);
//		StretchDIBits(hdcPosi,0,0,320,240,0,0,320,240,
//		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
//	::EndPaint(hwndPosi,&ps);
//}

//-------------------------------------------------
// Name     : Detect1Out(unsigned char *pix)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : 検出画像出力1
// Argument : pix:出力画像データ
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::Detect1Out(unsigned char *pix, int srcX, int srcY){
	//画像出力
	::BeginPaint(hwndDetect1,&ps);
	SetStretchBltMode(hdcDetect1,COLORONCOLOR);
	StretchDIBits(hdcDetect1,0,0,320,140, srcX,srcY,320,140,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndDetect1,&ps);
}

//-------------------------------------------------
// Name     : ImageOut(unsigned char *pix)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : リアルタイム画像出力
// Argument : pix:出力画像データ
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::ImageSkelton(unsigned char *pix){
	//画像出力
	::BeginPaint(hwndskelton,&ps);//指定されたウィンドウに対して描画の準備(HWND  hwnd,//  ウィンドウのハンドル LPPAINTSTRUCT  lpPaint  //  描画情報を持つ構造体へのポインタ);
	SetStretchBltMode(hdcskelton,COLORONCOLOR);//指定されたデバイス独立ビットマップ（DIB）内の長方形ピクセルの色データを、指定された長方形へコピー(HDC hdc,// デバイスコンテキストのハンドル int iStretchMode // ビットマップの伸縮モードピクセルを削除します。取り除く点の情報を何らかの形で維持せようとはせず、単純にそれらの点を削除します。);
	StretchDIBits(hdcskelton,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// デバイスコンテキストのハンドルint XDest, // コピー先長方形の左上隅の x 座標 int YDest,// コピー先長方形の左上隅の y 座標int nDestWidth, // コピー先長方形の幅int nDestHeight,// コピー先長方形の高さint XSrc,// コピー元長方形の左上隅の x 座標int YSrc,// コピー元長方形の左上隅の x 座標int nSrcWidth,// コピー元長方形の幅int nSrcHeight,// コピー元長方形の高さ
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// ビットマップのビットCONST BITMAPINFO *lpBitsInfo, // ビットマップのデータUINT iUsage,// データの種類 DWORD dwRop// ラスタオペレーションコード);
	::EndPaint(hwndskelton,&ps);
}
void CMonitoringSystemDlg::ImageObjectID(unsigned char *pix){
	//画像出力
	::BeginPaint(hwndObjectID,&ps);//指定されたウィンドウに対して描画の準備(HWND  hwnd,//  ウィンドウのハンドル LPPAINTSTRUCT  lpPaint  //  描画情報を持つ構造体へのポインタ);
	SetStretchBltMode(hdcObjectID,COLORONCOLOR);//指定されたデバイス独立ビットマップ（DIB）内の長方形ピクセルの色データを、指定された長方形へコピー(HDC hdc,// デバイスコンテキストのハンドル int iStretchMode // ビットマップの伸縮モードピクセルを削除します。取り除く点の情報を何らかの形で維持せようとはせず、単純にそれらの点を削除します。);
	StretchDIBits(hdcObjectID,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// デバイスコンテキストのハンドルint XDest, // コピー先長方形の左上隅の x 座標 int YDest,// コピー先長方形の左上隅の y 座標int nDestWidth, // コピー先長方形の幅int nDestHeight,// コピー先長方形の高さint XSrc,// コピー元長方形の左上隅の x 座標int YSrc,// コピー元長方形の左上隅の x 座標int nSrcWidth,// コピー元長方形の幅int nSrcHeight,// コピー元長方形の高さ
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// ビットマップのビットCONST BITMAPINFO *lpBitsInfo, // ビットマップのデータUINT iUsage,// データの種類 DWORD dwRop// ラスタオペレーションコード);
	::EndPaint(hwndObjectID,&ps);
}

void CMonitoringSystemDlg::Imagetrack(unsigned char *pix){
	//画像出力
	::BeginPaint(hwndtrack,&ps);//指定されたウィンドウに対して描画の準備(HWND  hwnd,//  ウィンドウのハンドル LPPAINTSTRUCT  lpPaint  //  描画情報を持つ構造体へのポインタ);
	SetStretchBltMode(hdctrack,COLORONCOLOR);//指定されたデバイス独立ビットマップ（DIB）内の長方形ピクセルの色データを、指定された長方形へコピー(HDC hdc,// デバイスコンテキストのハンドル int iStretchMode // ビットマップの伸縮モードピクセルを削除します。取り除く点の情報を何らかの形で維持せようとはせず、単純にそれらの点を削除します。);
	StretchDIBits(hdctrack,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// デバイスコンテキストのハンドルint XDest, // コピー先長方形の左上隅の x 座標 int YDest,// コピー先長方形の左上隅の y 座標int nDestWidth, // コピー先長方形の幅int nDestHeight,// コピー先長方形の高さint XSrc,// コピー元長方形の左上隅の x 座標int YSrc,// コピー元長方形の左上隅の x 座標int nSrcWidth,// コピー元長方形の幅int nSrcHeight,// コピー元長方形の高さ
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// ビットマップのビットCONST BITMAPINFO *lpBitsInfo, // ビットマップのデータUINT iUsage,// データの種類 DWORD dwRop// ラスタオペレーションコード);
	::EndPaint(hwndtrack,&ps);
}

void CMonitoringSystemDlg::Imagedepth(unsigned char *pix){
	//画像出力
	::BeginPaint(hwnddepth,&ps);//指定されたウィンドウに対して描画の準備(HWND  hwnd,//  ウィンドウのハンドル LPPAINTSTRUCT  lpPaint  //  描画情報を持つ構造体へのポインタ);
	SetStretchBltMode(hdcdepth,COLORONCOLOR);//指定されたデバイス独立ビットマップ（DIB）内の長方形ピクセルの色データを、指定された長方形へコピー(HDC hdc,// デバイスコンテキストのハンドル int iStretchMode // ビットマップの伸縮モードピクセルを削除します。取り除く点の情報を何らかの形で維持せようとはせず、単純にそれらの点を削除します。);
	StretchDIBits(hdcdepth,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// デバイスコンテキストのハンドルint XDest, // コピー先長方形の左上隅の x 座標 int YDest,// コピー先長方形の左上隅の y 座標int nDestWidth, // コピー先長方形の幅int nDestHeight,// コピー先長方形の高さint XSrc,// コピー元長方形の左上隅の x 座標int YSrc,// コピー元長方形の左上隅の x 座標int nSrcWidth,// コピー元長方形の幅int nSrcHeight,// コピー元長方形の高さ
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// ビットマップのビットCONST BITMAPINFO *lpBitsInfo, // ビットマップのデータUINT iUsage,// データの種類 DWORD dwRop// ラスタオペレーションコード);
	::EndPaint(hwnddepth,&ps);
}


//検出画像出力2
void CMonitoringSystemDlg::Detect2Out(unsigned char *pix, int srcX, int srcY){
	::BeginPaint(hwndDetect2,&ps);
	SetStretchBltMode(hdcDetect2,COLORONCOLOR);
	StretchDIBits(hdcDetect2,0,0,320,140, srcX,srcY,320,140,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndDetect2,&ps);
}

//検出画像出力3
void CMonitoringSystemDlg::Detect3Out(unsigned char *pix, int srcX, int srcY){
	::BeginPaint(hwndDetect3,&ps);
	SetStretchBltMode(hdcDetect3,COLORONCOLOR);
	StretchDIBits(hdcDetect3,0,0,320,140,srcX,srcY,320,140,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndDetect3,&ps);
}

//レイヤ画像出力
void CMonitoringSystemDlg::LayerOut(unsigned char *pix){
	::BeginPaint(hwndLayer,&ps);
	SetStretchBltMode(hdcLayer,COLORONCOLOR);
	StretchDIBits(hdcLayer,0,0,320,240,0,0,320,240,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndLayer,&ps);
}

//カメラ画像出力
void CMonitoringSystemDlg::showCamera(unsigned char *pix){
	::BeginPaint(hwndCamera,&ps);
	SetStretchBltMode(hdcCamera,COLORONCOLOR);
	StretchDIBits(hdcCamera,0,0,320,240,0,0,320,240,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndCamera,&ps);
}

//オフラインサムネイル出力
void CMonitoringSystemDlg::showThumnail(unsigned char *pix){
	::BeginPaint(hwndThumnail,&ps);
	SetStretchBltMode(hdcThumnail,COLORONCOLOR);
	StretchDIBits(hdcThumnail,0,0,160,120,0,0,160,120,
		pix,&bmpInfoThumbnail,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndThumnail,&ps);
}
//結果画像出力
void CMonitoringSystemDlg::DetectOut(){
	for( int i = 0 ; i < min( count, 3 ) ; ++i)
	{
		SceneOut(count-1-i,i+1);
		DetectLoad(obj[count-1-i].frame,obj[count-1-i].id,i+1, obj[count-1-i].cx,obj[count-1-i].cy);
		string keyString = "Event"+Util::toString( i+1 );
		string keyString_label = "イベント履歴"+Util::toString( i+1 )+"ラベル";

		//人物IDが物体IDを
		if( obj[count-1-i].scene == true )
		{
			if(obj[count-1-i].track == true )//kawa
			{
				//検知画像出力領域
			    string name_data="人物ID:"+Util::toString(obj[count-1-i].human)+"が物体ID:"+Util::toString(obj[count-1-i].id)+"を移動";
				imagepack[ keyString ] = MultiData(name_data, "string",  "移動"  );
			}
			else
			{string name_data="人物ID:"+Util::toString(obj[count-1-i].human)+"が物体ID:"+Util::toString(obj[count-1-i].id)+"を持ち込み";
				imagepack[ keyString ] = MultiData( name_data, "string",  "持ち込み"  );
			}

		}
		else
		{string name_data="人物ID:"+Util::toString(obj[count-1-i].human)+"が物体ID:"+Util::toString(obj[count-1-i].id)+"を持ち去り";
			imagepack[ keyString ] = MultiData( name_data, "string",  "持ち去り"  );
		}

	}
	disp_flag=true;
}

//検出画像読み込み
void CMonitoringSystemDlg::DetectLoad(int num,int id,int flag, int cx, int cy){
	cv::Mat cap;
	std::string img_fname1;
	std::ostringstream dir,frame_name;
	dir  << "../DB/Detected-Object/" << num<<"-"<<id<< "/";
	frame_name << num<<"-"<<id;
	img_fname1  = img_fname1  + dir.str() + "result-" + frame_name.str() + ".png";
	cap=cv::imread(img_fname1);
	int sx = 0, sy;
	cv::Rect rect;
	if(cy < 140)
	{
		sy = 100 ;rect=cv::Rect(0,0,320,140);
	}
	else
	{
		sy = 0;rect=cv::Rect(0,100,320,240);
	}
	//cv::Mat img=cap(rect);
	//cap.copyTo(img);

	//cap=cap(rect);
	//img.copyTo(cap);

	if( flag == 3 )
	{
		imagepack["event_image3"] = MultiData( Util::convertMat2String(cap),"image", "イベント履歴3");
		cv::flip(cap,cap,0);
		Detect3Out(&cap.at<cv::Vec3b>(0,0)[0], sx,sy);
	}
	else if( flag == 2 )
	{
		imagepack["event_image2"] = MultiData( Util::convertMat2String(cap),"image", "イベント履歴2");
		cv::flip(cap,cap,0);
		Detect2Out(&cap.at<cv::Vec3b>(0,0)[0], sx,sy);
	}
	else if( flag == 1 )
	{
		imagepack["event_image1"] = MultiData( Util::convertMat2String(cap),"image", "イベント履歴1");
		cv::flip(cap,cap,0);
		Detect1Out(&cap.at<cv::Vec3b>(0,0)[0], sx,sy);
	}

}


//-------------------------------------------------
// Name     : 
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : 検出時の詳細情報出力
// Argument : pix:出力画像データ
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene, int track_flag,int key_point_parent_id,int human){
	//シーン履歴の更新
	if( count > 9 )
	{
		for( int i=0 ; i < 9 ; ++i)
		{
			obj[i] = obj[i+1];
		}
		count=9;
	}
	//cout << "(" << cx << "," << cy <<")" <<"model_id:"<<id<< endl;
	obj[count].id = id;//id;
	obj[count].frame = frame;
	obj[count].cx = cx;
	obj[count].cy = cy;
	obj[count].key_point_parent_id=key_point_parent_id;
	strcpy_s(obj[count].hour,hour);
	strcpy_s(obj[count].minute,minute);
	obj[count].scene = scene;
	obj[count].human=human;
	if( track_flag == 2 )
	{
		obj[count].track =true;track_frame = frame;
		GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON5)->EnableWindow(TRUE);
	}//kawa
	else
	{
		obj[count].track=false;
	}
	++count;
}

CString CMonitoringSystemDlg::BrowseForFolder( HWND p_hWnd, CString p_cSetStr, CString p_cRootStr, CString p_cCaptionStr, UINT p_uiFlags ){
	BOOL		bRes;
	char		chPutFolder[MAX_PATH];
	LPITEMIDLIST	pidlRetFolder;
	BROWSEINFO	stBInfo;
	CString		cRetStr;

	//　構造体を初期化します。
	stBInfo.pidlRoot = NULL;	//ルートフォルダです。
	stBInfo.hwndOwner = p_hWnd;	//表示するダイアログの親ウィンドウのハンドルです。
	stBInfo.pszDisplayName = chPutFolder;	//選択されているフォルダ名を受けます。
	stBInfo.lpszTitle = p_cCaptionStr;	//説明の文字列です。
	stBInfo.ulFlags = p_uiFlags;	//表示フラグです。
	stBInfo.lpfn = NULL;	//ダイアログプロシージャへのポインタです。
	stBInfo.lParam = 0L;	//プロシージャに送るパラメーターです。

	//　ダイアログボックスを表示します。
	pidlRetFolder = ::SHBrowseForFolder( &stBInfo );

	//　pidlRetFolderにはそのフォルダを表すアイテムＩＤリストへのポインタが
	//　入っています。chPutFolderには選択されたフォルダ名（非フルパス）だけ
	//　しか入っていないので、このポインタを利用します。

	if( pidlRetFolder != NULL )
	{
		//　フルパスを取得します。
		bRes = ::SHGetPathFromIDList( pidlRetFolder, chPutFolder );
		if( bRes != FALSE )
		{
			cRetStr = chPutFolder;
		}
		::CoTaskMemFree( pidlRetFolder );
	}

	return cRetStr;
}


//-------------------------------------------------
// Name     : SceneOut(int id)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : 検知画像にシーン解釈を追加
// Argument : id：物体検知番号
//             x：出力する位置(x座標)
//             y：出力する位置(y座標)
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::SceneOut(int num,int flag){
	char msg[4]; //出力文字列

	if(flag==1)
	{
		((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_SCENE1))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel("\r\n");
		//ID出力
		_itoa_s(obj[num].key_point_parent_id,msg,10);  //10進数で変換
		((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel(msg);
		//シーン解釈出力
		if(obj[num].scene==true)
		{
			if(obj[num].track == true)
			{
				//検知画像出力領域
				CDC* pDC = detect1.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(0,255,0));
				pDC->TextOut(0,0,"移動    ");
			}
			else
			{
				//検知画像出力領域
				CDC* pDC = detect1.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->TextOut(0,0,"持ち込み");
			}
			//((CEdit*)GetDlgItem(IDC_SCENE1))->ReplaceSel("持ち込み");
		}
		else
		{
			//検知画像出力領域
			CDC* pDC = detect1.GetDC();
			pDC->SelectObject(&font);
			pDC->SetTextColor(RGB(0,0,255));
			pDC->TextOut(0,0,"持ち去り");
			//((CEdit*)GetDlgItem(IDC_SCENE1))->ReplaceSel("持ち去り");
		}
		//時刻出力
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel(obj[num].hour);
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel("：");
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel(obj[num].minute);
		//重心出力
		_itoa_s(obj[num].cx,msg,10);  //10進数で変換
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel("(");
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel(msg);
		_itoa_s(obj[num].cy,msg,10);  //10進数で変換
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel(",");
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel(msg);
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel(")");
	}
	else if(flag==2)
	{
		((CEdit*)GetDlgItem(IDC_ID2))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_SCENE2))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_TIME2))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_POSITION2))->ReplaceSel("\r\n");
		_itoa_s(obj[num].key_point_parent_id,msg,10);
		((CEdit*)GetDlgItem(IDC_ID2))->ReplaceSel(msg);
		if(obj[num].scene==true)
		{
			if(obj[num].track == true)
			{
				//検知画像出力領域
				CDC* pDC = detect2.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(0,255,0));
				pDC->TextOut(0,0,"移動    ");
			}
			else
			{
				//検知画像出力領域
				CDC* pDC = detect2.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->TextOut(0,0,"持ち込み");
			}
			//((CEdit*)GetDlgItem(IDC_SCENE2))->ReplaceSel("持ち込み");
		}
		else
		{
			CDC* pDC = detect2.GetDC();
			pDC->SelectObject(&font);
			pDC->SetTextColor(RGB(0,0,255));
			pDC->TextOut(0,0,"持ち去り ");
			//((CEdit*)GetDlgItem(IDC_SCENE2))->ReplaceSel("持ち去り");
		}
		((CEdit*)GetDlgItem(IDC_TIME2))->ReplaceSel(obj[num].hour);
		((CEdit*)GetDlgItem(IDC_TIME2))->ReplaceSel("：");
		((CEdit*)GetDlgItem(IDC_TIME2))->ReplaceSel(obj[num].minute);
		_itoa_s(obj[num].cx,msg,10);
		((CEdit*)GetDlgItem(IDC_POSITION2))->ReplaceSel("(");
		((CEdit*)GetDlgItem(IDC_POSITION2))->ReplaceSel(msg);
		_itoa_s(obj[num].cy,msg,10);
		((CEdit*)GetDlgItem(IDC_POSITION2))->ReplaceSel(",");
		((CEdit*)GetDlgItem(IDC_POSITION2))->ReplaceSel(msg);
		((CEdit*)GetDlgItem(IDC_POSITION2))->ReplaceSel(")");
	}
	else
	{
		((CEdit*)GetDlgItem(IDC_ID3))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_SCENE3))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_TIME3))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_POSITION3))->ReplaceSel("\r\n");
		_itoa_s(obj[num].key_point_parent_id,msg,10);
		((CEdit*)GetDlgItem(IDC_ID3))->ReplaceSel(msg);
		if(obj[num].scene==true)
		{
			if(obj[num].track == true)
			{
				//検知画像出力領域
				CDC* pDC = detect3.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(0,255,0));
				pDC->TextOut(0,0,"移動    ");
			}
			else
			{
				//検知画像出力領域
				CDC* pDC = detect3.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->TextOut(0,0,"持ち込み");
			}
				//((CEdit*)GetDlgItem(IDC_SCENE3))->ReplaceSel("持ち込み");
		}
		else
		{
			CDC* pDC = detect3.GetDC();
			pDC->SelectObject(&font);
			pDC->SetTextColor(RGB(0,0,255));
			pDC->TextOut(0,0,"持ち去り");
			//((CEdit*)GetDlgItem(IDC_SCENE3))->ReplaceSel("持ち去り");
		}
		((CEdit*)GetDlgItem(IDC_TIME3))->ReplaceSel(obj[num].hour);
		((CEdit*)GetDlgItem(IDC_TIME3))->ReplaceSel("：");
		((CEdit*)GetDlgItem(IDC_TIME3))->ReplaceSel(obj[num].minute);
		_itoa_s(obj[num].cx,msg,10);
		((CEdit*)GetDlgItem(IDC_POSITION3))->ReplaceSel("(");
		((CEdit*)GetDlgItem(IDC_POSITION3))->ReplaceSel(msg);
		_itoa_s(obj[num].cy,msg,10);
		((CEdit*)GetDlgItem(IDC_POSITION3))->ReplaceSel(",");
		((CEdit*)GetDlgItem(IDC_POSITION3))->ReplaceSel(msg);
		((CEdit*)GetDlgItem(IDC_POSITION3))->ReplaceSel(")");
	}
}

//-------------------------------------------------
// Name     : Init()
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : 初期化
// Argument : なし
// return   : なし
//-------------------------------------------------
void CMonitoringSystemDlg::Init(){
	bmpInfo.bmiHeader.biHeight        = 240;
	bmpInfo.bmiHeader.biWidth         = 320;
	bmpInfo.bmiHeader.biBitCount      = 24;
	bmpInfo.bmiHeader.biClrImportant  = 0;
	bmpInfo.bmiHeader.biClrUsed       = 0;
	bmpInfo.bmiHeader.biCompression   = 0;
	bmpInfo.bmiHeader.biPlanes        = 1;
	bmpInfo.bmiHeader.biSize          = 40;
	bmpInfo.bmiHeader.biXPelsPerMeter = 0;
	bmpInfo.bmiHeader.biYPelsPerMeter = 0;

	bmpInfoThumbnail.bmiHeader.biHeight        = 120;
	bmpInfoThumbnail.bmiHeader.biWidth         = 160;
	bmpInfoThumbnail.bmiHeader.biBitCount      = 24;
	bmpInfoThumbnail.bmiHeader.biClrImportant  = 0;
	bmpInfoThumbnail.bmiHeader.biClrUsed       = 0;
	bmpInfoThumbnail.bmiHeader.biCompression   = 0;
	bmpInfoThumbnail.bmiHeader.biPlanes        = 1;
	bmpInfoThumbnail.bmiHeader.biSize          = 40;
	bmpInfoThumbnail.bmiHeader.biXPelsPerMeter = 0;
	bmpInfoThumbnail.bmiHeader.biYPelsPerMeter = 0;

	hwndImage = NULL;
	GetDlgItem(IDC_IMAGE,&hwndImage);
	hdcImage = ::GetDC(hwndImage);

	hwndDiff = NULL;
	GetDlgItem(IDC_DIFF,&hwndDiff);
	hdcDiff = ::GetDC(hwndDiff);

	//hwndPosi = NULL;
	//GetDlgItem(IDC_POSI,&hwndPosi);
	//hdcPosi = ::GetDC(hwndPosi);

	hwndDetect1 = NULL;
	GetDlgItem(IDC_DETECT1,&hwndDetect1);
	hdcDetect1 = ::GetDC(hwndDetect1);

	hwndDetect2 = NULL;
	GetDlgItem(IDC_DETECT2,&hwndDetect2);
	hdcDetect2 = ::GetDC(hwndDetect2);

	hwndDetect3 = NULL;
	GetDlgItem(IDC_DETECT3,&hwndDetect3);
	hdcDetect3 = ::GetDC(hwndDetect3);

	hwndLayer = NULL;
	GetDlgItem(IDC_LAYER,&hwndLayer);
	hdcLayer = ::GetDC(hwndLayer);

	hwndCamera = NULL;
	GetDlgItem(IDC_CAMERA,&hwndCamera);
	hdcCamera = ::GetDC(hwndCamera);

	hwndThumnail = NULL;
	GetDlgItem(IDC_THUMNAIL,&hwndThumnail);
	hdcThumnail = ::GetDC(hwndThumnail);

	hwndObjectID = NULL;
	GetDlgItem(IDC_ObjectID,&hwndObjectID);
	hdcObjectID = ::GetDC(hwndObjectID);

	hwndtrack = NULL;
	GetDlgItem(IDC_track,&hwndtrack);
	hdctrack = ::GetDC(hwndtrack);

	hwnddepth = NULL;
	GetDlgItem(IDC_depth_img,&hwnddepth);
	hdcdepth = ::GetDC(hwnddepth);

	hwndskelton = NULL;
	GetDlgItem(IDC_Skelton,&hwndskelton);
	hdcskelton = ::GetDC(hwndskelton);

	count=0;
	tmp_count=0;
	disp_flag=true;

	//フォントの作成
	font.CreateFont(
		20,
		0,
		0,
		0,
		FW_BOLD,
		FALSE,
		FALSE,
		FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY,
		DEFAULT_PITCH,
		_T("MS Pゴシック")
		);
	TCHAR szCurrentDirectory[MAX_PATH];
	if (!::GetCurrentDirectory(MAX_PATH, szCurrentDirectory))
	{
		// Error
	}
	else
	{
		m_str_dirPath_offlineInput = szCurrentDirectory;
		string str_dirPath_offlineInput = m_str_dirPath_offlineInput;
		int posSplitToken = str_dirPath_offlineInput.rfind( "\\", str_dirPath_offlineInput.size() - 1);
		if( posSplitToken == str_dirPath_offlineInput.npos )
		{
			exit(1);
		}
		m_str_dirPath_offlineInput = str_dirPath_offlineInput.substr(0, posSplitToken).c_str();
		m_str_dirPath_offlineInput += "\\img\\sample";
	}
	UpdateData(FALSE);

	unsigned char *cImgThumbnail = getImageFromImageFile(m_str_dirPath_offlineInput);
	if(cImgThumbnail == NULL)
	{

	}
	else 
	{
		showThumnail(cImgThumbnail);
	}

	//カメラを1(0index)に設定しておく
	m_control_CameraList.SetCurSel(0);//133.19.22.240選択
	showNCamera(0);
	UpdateData(TRUE);
	UpdateData(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON5)->EnableWindow(FALSE);

	ifstream ifs("ignoreDialog.txt");
	if (ifs.fail())
	{
		std::cerr << "invalid text: [ ignoreDialog.txt ]" << std::endl;
	}
	else{
		string line;
		getline( ifs, line );
		if( line == "true" ){
			if( mode == MODE_NONE )
			{
				mode = MODE_ONLINE;     //モードの設定
				CWnd::SetDlgItemTextA(IDC_ONLINE, "STOP");
			}
			else if( mode == MODE_ONLINE )
			{
				mode = MODE_NONE;
				CWnd::SetDlgItemTextA(IDC_ONLINE, "ONLINE");
			}
			//このクラスのポインタを渡す
			camera.set_pParent(this);
			AfxBeginThread(ThreadProcCalc, &camera,THREAD_PRIORITY_BELOW_NORMAL);
		}
	}
}

//履歴ボタン(前)
void CMonitoringSystemDlg::OnBnClickedButton1(){
	if( disp_flag == true )
	{
		tmp_count=count-1;
	}
	else
	{
		--tmp_count;
	}
	if( tmp_count >= 3 )
	{
		for( int i = 0 ; i < 3 ; ++i )
		{
			SceneOut(tmp_count-1-i,3-i);
			DetectLoad(obj[tmp_count-1-i].frame,obj[tmp_count-1-i].id,3-i, obj[tmp_count-1-i].cx, obj[tmp_count-1-i].cy);
		}
		disp_flag = false;
	}
	else
	{
		++tmp_count; //一つ減らしたのを戻す
	}
}

//履歴ボタン（後）
void CMonitoringSystemDlg::OnBnClickedButton2(){
	tmp_count++; //現在表示している部分の一つ後

	if( tmp_count <= count )
	{
		for( int i = 0 ; i < 3 ; ++i )
		{
			SceneOut(tmp_count-1-i,3-i);
			DetectLoad(obj[tmp_count-1-i].frame,obj[tmp_count-1-i].id,3-i, obj[tmp_count-1-i].cx, obj[tmp_count-1-i].cy);
		}
	}
	disp_flag = false;
}

//履歴ボタン（最新）
void CMonitoringSystemDlg::OnBnClickedButton3(){
	DetectOut();
	disp_flag = true; //履歴表示が最新
}

void CMonitoringSystemDlg::showNCamera(int n){
	CameraNumber = n;
	char nameCameraPath[256];
	sprintf_s(nameCameraPath,"./jpg/camera/camera%d.jpg", CameraNumber+1);
	cv::Mat imgCamera=cv::imread(string(nameCameraPath));
	if( imgCamera.data != NULL )
	{
		unsigned char *cImgCamera = (unsigned char*)calloc( sizeof(char), imgCamera.cols * imgCamera.rows * 3);
		for( int i = 0 ; i < imgCamera.rows ; ++i )
		{
			for( int j = 0; j < imgCamera.cols ; ++j )
			{
				unsigned int pos = ((imgCamera.rows - 1 - i) * imgCamera.cols + j) * 3;
				unsigned int posOriginal = ((i) * imgCamera.cols + j) * 3;
				cImgCamera[pos] = imgCamera.data[posOriginal];
				cImgCamera[pos + 1] = imgCamera.data[posOriginal + 1];
				cImgCamera[pos + 2] = imgCamera.data[posOriginal + 2];
			}
		}
		showCamera(cImgCamera);
	}
}
void CMonitoringSystemDlg::OnCbnSelchangeCombo2(){
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
	showNCamera(m_cameraId);
}


void CMonitoringSystemDlg::OnBnClickedButton4(){
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	char file[128];
	char tmp[10];
	cv::Mat tracking;


	//画像がない場合はiを減らしてあるかどうか探す
	while( 1 )
	{
		sprintf_s(file,"../DB/track_img/");
		sprintf_s(tmp,"%08lu",track_frame);
		strcat_s(file,tmp);
		strcat_s(file,".png");

		if( track_frame < 1 )
		{ //終了
			break;
		}
		tracking = cv::imread(file);
		if( tracking.empty() )
		{
			--track_frame;
		}
		else
		{
			cv::flip(tracking,tracking, 0);
			//cout<<"a"<<endl;
			Imagetrack(&tracking.data[0]);
			--track_frame;
			break;
		}
	}
}
void CMonitoringSystemDlg::OnBnClickedButton5(){
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	char file[128];
	char tmp[10];
	//obj[count-1].frame;
	cv::Mat tracking;


	//画像がない場合はiを減らしてあるかどうか探す
	while( 1 )
	{
		sprintf_s(file,"../DB/track_img/");
		sprintf_s(tmp,"%08lu",track_frame);
		strcat_s(file,tmp);
		strcat_s(file,".png");

		if( track_frame>obj[count-1].frame )
		{ //終了
			break;
		}

		tracking = cv::imread(file);

		if( tracking.empty() )
		{
			++track_frame;
		}
		else
		{
			cv::flip(tracking,tracking, 0);
			Imagetrack(&tracking.data[0]);
			++track_frame;
			break;
		}
	}
}



INT CALLBACK _BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData) {
	TCHAR szDir[MAX_PATH];
	switch(uMsg)
	{
	case BFFM_INITIALIZED: 
		// WParam is TRUE since you are passing a path.
		// It would be FALSE if you were passing a pidl.
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)pData);
		break;
	case BFFM_SELCHANGED: 
		// Set the status window to the currently selected path.
		if ( SHGetPathFromIDList((LPITEMIDLIST)lParam ,szDir) )
		{
			SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
		}
		break;
	}
	return 0;
}

CString GetFolderFullpath(LPCTSTR lpszDefault){
	TCHAR buffDisplayName[MAX_PATH];
	TCHAR fullpath[MAX_PATH];
	BROWSEINFO  browseinfo;
	LPITEMIDLIST lpitemidlist;

	ZeroMemory(&browseinfo, sizeof( BROWSEINFO ));
	browseinfo.pszDisplayName = buffDisplayName ;
	browseinfo.lpszTitle = _T("フォルダを選択してください");
	browseinfo.ulFlags = BIF_RETURNONLYFSDIRS;
	browseinfo.lParam = (LPARAM)lpszDefault;
	browseinfo.lpfn = _BrowseCallbackProc;

	if( !(lpitemidlist = SHBrowseForFolder(&browseinfo)) )
	{
		AfxMessageBox(_T("フォルダを選択していませんでした"));
		return CString(_T(""));
	}
	else
	{
		SHGetPathFromIDList(lpitemidlist, fullpath);       
		CoTaskMemFree(lpitemidlist); 
		return CString(fullpath);
	}
}

unsigned char* CMonitoringSystemDlg::getImageFromImageFile(CString strPath){
	char fnameThumnail[256];
	string extentionImg[3] = {"jpg","bmp","png"};
	int idxExtention = 0; 
	int _idxExtention = idxExtention;
	cv::Mat imgThumnail;
	for( int j = 0 ; j < 3 ; ++j )
	{
		sprintf_s(fnameThumnail,"%s/%08lu.%s", strPath, 0, extentionImg[idxExtention].c_str());
		if((imgThumnail = cv::imread(fnameThumnail)).data != NULL)
		{
			break;
		}
		idxExtention = (idxExtention + 1) % 3;
		if(idxExtention == _idxExtention)
		{
			cout << "No Image" << endl;
			break ;
		}
	}
	if( imgThumnail.data!=NULL )
	{
		cv::Mat imgResizeThumnail(cv::Size(160,120), CV_8UC3, cv::Scalar(255,255,255));
		double scale = (double)imgResizeThumnail.rows / (double)imgThumnail.rows;
		cv::resize(imgThumnail, imgResizeThumnail, cv::Size(), scale, scale);
		unsigned char *cImgThumnail = (unsigned char*)calloc( sizeof(char), imgResizeThumnail.cols * imgResizeThumnail.rows * 3);
		cout << "("<<imgThumnail.rows << ", "<< imgThumnail.cols << ")" << endl;
		cout << "("<<imgResizeThumnail.rows << ", "<< imgResizeThumnail.cols << ")" << endl;
		for( int i = imgResizeThumnail.rows - 1 ; i >= 0 ; --i )
		{
			for( int j = imgResizeThumnail.cols - 1; j >= 0 ; --j )
			{
				unsigned int pos = ((imgResizeThumnail.rows - 1 - i) * imgResizeThumnail.cols + j) * 3;
				unsigned int posOriginal = ((i) * imgResizeThumnail.cols + j) * 3;
				cImgThumnail[pos] = imgResizeThumnail.data[posOriginal];
				cImgThumnail[pos + 1] = imgResizeThumnail.data[posOriginal + 1];
				cImgThumnail[pos + 2] = imgResizeThumnail.data[posOriginal + 2];
			}
		}
		return cImgThumnail;
	}
	return NULL;
}
void CMonitoringSystemDlg::OnBnClickedButtonSelectdir(){
	CString strFolderFullpath = GetFolderFullpath(m_str_dirPath_offlineInput);
	if (strFolderFullpath != _T(""))
	{
		m_str_dirPath_offlineInput = strFolderFullpath;
		std::cout << "offlinedir:" << m_str_dirPath_offlineInput << std::endl;
		unsigned char *cImgThumbnail = getImageFromImageFile(m_str_dirPath_offlineInput);
		if(cImgThumbnail == NULL)
		{

		}
		else
		{
			showThumnail(cImgThumbnail);
		}
	}
	else
	{

	}
}


int CMonitoringSystemDlg::getMode(){
	//cout << mode << "(mode)" << endl;
	return mode;
}