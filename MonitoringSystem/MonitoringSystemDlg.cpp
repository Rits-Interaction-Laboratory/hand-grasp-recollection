//-------------------------------------------------
// FileName : MonitoringSystemDlg.cpp
// Context  : �C���v�������e�[�V���� �t�@�C��
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


int CameraNumber;  //�g�p�J�����ԍ�
CCamera camera;//�J��������N���X


// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialog{
public:
	CAboutDlg();

	// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	// ����
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


// CMonitoringSystemDlg �_�C�A���O




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
	DDX_Control(pDX,IDC_SCENE1,detect1);  //detect�̈�
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


// CMonitoringSystemDlg ���b�Z�[�W �n���h��


//�X���b�h(�I�����C��)
static UINT AFX_CDECL ThreadProcCalc(LPVOID pParam){
	CCamera *cp;      //�J��������N���X(�X���b�h�p)
	cp=(CCamera*)pParam;
	cp->SendCommand(CameraNumber,5);
	return 0;
}

//�X���b�h(�I�t���C��)
static UINT AFX_CDECL ThreadOffLine(LPVOID pParam){
	CCamera *cp;      //�J��������N���X(�X���b�h�p)
	cp=(CCamera*)pParam;
	cp->OffLine();
	return 0;
}

BOOL CMonitoringSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "�o�[�W�������..." ���j���[���V�X�e�� ���j���[�ɒǉ����܂��B

	// IDM_ABOUTBOX �́A�V�X�e�� �R�}���h�͈͓̔��ɂȂ���΂Ȃ�܂���B
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

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	// TODO: �������������ɒǉ����܂��B
	Init();
	//GetDlgItem(IDC_BUTTON_FOLDER)->EnableWindow(FALSE);


	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
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

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void CMonitoringSystemDlg::OnPaint(){
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CMonitoringSystemDlg::OnQueryDragIcon(){
	return static_cast<HCURSOR>(m_hIcon);
}

//�_�C�A���O�o��

//------------------------------------------------------
// Name     : Disp_Log(char *msg)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.05.22
// Function : LOG �ɕ�������o��
// Argument : �o�͂��镶����
// return   : �Ȃ�
//------------------------------------------------------
void CMonitoringSystemDlg::Disp_Log(char *msg){
	((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel("\r\n");
	((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel(msg);
}


//�{�^������

//-------------------------------------------------
// Name     : OnBnClickedOnline()
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2006.07.26
// Function : �l�b�g���[�N�J����3���瓮�摜�擾
// Argument : �Ȃ�
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::OnBnClickedOnline(){
	//���̃J�������g�p���̏ꍇ
	/*if(CameraNumber>0){
	cp->set_stop();
	Sleep(200);
	}*/
	std::cout<<"Online Start" <<std::endl;
	GetDlgItem(IDC_OFFLINE)->EnableWindow(FALSE);
	//CameraNumber = 1;  //�g�p�J�����̐ݒ�
	if( mode == MODE_NONE )
	{
		mode = MODE_ONLINE;     //���[�h�̐ݒ�
		CWnd::SetDlgItemTextA(IDC_ONLINE, "STOP");
	}
	else if( mode == MODE_ONLINE )
	{
		mode = MODE_NONE;
		CWnd::SetDlgItemTextA(IDC_ONLINE, "ONLINE");
	}
	//���̃N���X�̃|�C���^��n��
	camera.set_pParent(this);


	//�X���b�h��p���ăJ��������摜���擾
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
// Function : �I�t���C���ŏ���
// Argument : �Ȃ�
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::OnBnClickedOffline(){
	mode = MODE_OFFLINE;  //���[�h�̐ݒ�
	GetDlgItem(IDC_OFFLINE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ONLINE)->EnableWindow(FALSE);
	//���̃N���X�̃|�C���^��n��
	camera.set_pParent(this);

	//�X���b�h��p���ăJ��������摜���擾
	AfxBeginThread(ThreadOffLine,&camera,THREAD_PRIORITY_BELOW_NORMAL);
}

//-------------------------------------------------
// Name     : ImageOut(unsigned char *pix)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : ���A���^�C���摜�o��
// Argument : pix:�o�͉摜�f�[�^
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::ImageOut(unsigned char *pix){
	//�摜�o��
	::BeginPaint(hwndImage,&ps);//�w�肳�ꂽ�E�B���h�E�ɑ΂��ĕ`��̏���(HWND  hwnd,//  �E�B���h�E�̃n���h�� LPPAINTSTRUCT  lpPaint  //  �`��������\���̂ւ̃|�C���^);
	SetStretchBltMode(hdcImage,COLORONCOLOR);//�w�肳�ꂽ�f�o�C�X�Ɨ��r�b�g�}�b�v�iDIB�j���̒����`�s�N�Z���̐F�f�[�^���A�w�肳�ꂽ�����`�փR�s�[(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h�� int iStretchMode // �r�b�g�}�b�v�̐L�k���[�h�s�N�Z�����폜���܂��B��菜���_�̏������炩�̌`�ňێ����悤�Ƃ͂����A�P���ɂ����̓_���폜���܂��B);
	StretchDIBits(hdcImage,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h��int XDest, // �R�s�[�撷���`�̍������ x ���W int YDest,// �R�s�[�撷���`�̍������ y ���Wint nDestWidth, // �R�s�[�撷���`�̕�int nDestHeight,// �R�s�[�撷���`�̍���int XSrc,// �R�s�[�������`�̍������ x ���Wint YSrc,// �R�s�[�������`�̍������ x ���Wint nSrcWidth,// �R�s�[�������`�̕�int nSrcHeight,// �R�s�[�������`�̍���
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// �r�b�g�}�b�v�̃r�b�gCONST BITMAPINFO *lpBitsInfo, // �r�b�g�}�b�v�̃f�[�^UINT iUsage,// �f�[�^�̎�� DWORD dwRop// ���X�^�I�y���[�V�����R�[�h);
	::EndPaint(hwndImage,&ps);
}

//-------------------------------------------------
// Name     : DiffOut(unsigned char *pix)
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : �����摜�o��
// Argument : pix:�o�͉摜�f�[�^
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::DiffOut(unsigned char *pix){
	//�摜�o��
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
// Function : keypoint�摜�o��
// Argument : pix:�o�͉摜�f�[�^
// return   : �Ȃ�
//-------------------------------------------------
//void CMonitoringSystemDlg::PosiOut(unsigned char *pix)
//{
//	//�摜�o��
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
// Function : ���o�摜�o��1
// Argument : pix:�o�͉摜�f�[�^
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::Detect1Out(unsigned char *pix, int srcX, int srcY){
	//�摜�o��
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
// Function : ���A���^�C���摜�o��
// Argument : pix:�o�͉摜�f�[�^
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::ImageSkelton(unsigned char *pix){
	//�摜�o��
	::BeginPaint(hwndskelton,&ps);//�w�肳�ꂽ�E�B���h�E�ɑ΂��ĕ`��̏���(HWND  hwnd,//  �E�B���h�E�̃n���h�� LPPAINTSTRUCT  lpPaint  //  �`��������\���̂ւ̃|�C���^);
	SetStretchBltMode(hdcskelton,COLORONCOLOR);//�w�肳�ꂽ�f�o�C�X�Ɨ��r�b�g�}�b�v�iDIB�j���̒����`�s�N�Z���̐F�f�[�^���A�w�肳�ꂽ�����`�փR�s�[(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h�� int iStretchMode // �r�b�g�}�b�v�̐L�k���[�h�s�N�Z�����폜���܂��B��菜���_�̏������炩�̌`�ňێ����悤�Ƃ͂����A�P���ɂ����̓_���폜���܂��B);
	StretchDIBits(hdcskelton,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h��int XDest, // �R�s�[�撷���`�̍������ x ���W int YDest,// �R�s�[�撷���`�̍������ y ���Wint nDestWidth, // �R�s�[�撷���`�̕�int nDestHeight,// �R�s�[�撷���`�̍���int XSrc,// �R�s�[�������`�̍������ x ���Wint YSrc,// �R�s�[�������`�̍������ x ���Wint nSrcWidth,// �R�s�[�������`�̕�int nSrcHeight,// �R�s�[�������`�̍���
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// �r�b�g�}�b�v�̃r�b�gCONST BITMAPINFO *lpBitsInfo, // �r�b�g�}�b�v�̃f�[�^UINT iUsage,// �f�[�^�̎�� DWORD dwRop// ���X�^�I�y���[�V�����R�[�h);
	::EndPaint(hwndskelton,&ps);
}
void CMonitoringSystemDlg::ImageObjectID(unsigned char *pix){
	//�摜�o��
	::BeginPaint(hwndObjectID,&ps);//�w�肳�ꂽ�E�B���h�E�ɑ΂��ĕ`��̏���(HWND  hwnd,//  �E�B���h�E�̃n���h�� LPPAINTSTRUCT  lpPaint  //  �`��������\���̂ւ̃|�C���^);
	SetStretchBltMode(hdcObjectID,COLORONCOLOR);//�w�肳�ꂽ�f�o�C�X�Ɨ��r�b�g�}�b�v�iDIB�j���̒����`�s�N�Z���̐F�f�[�^���A�w�肳�ꂽ�����`�փR�s�[(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h�� int iStretchMode // �r�b�g�}�b�v�̐L�k���[�h�s�N�Z�����폜���܂��B��菜���_�̏������炩�̌`�ňێ����悤�Ƃ͂����A�P���ɂ����̓_���폜���܂��B);
	StretchDIBits(hdcObjectID,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h��int XDest, // �R�s�[�撷���`�̍������ x ���W int YDest,// �R�s�[�撷���`�̍������ y ���Wint nDestWidth, // �R�s�[�撷���`�̕�int nDestHeight,// �R�s�[�撷���`�̍���int XSrc,// �R�s�[�������`�̍������ x ���Wint YSrc,// �R�s�[�������`�̍������ x ���Wint nSrcWidth,// �R�s�[�������`�̕�int nSrcHeight,// �R�s�[�������`�̍���
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// �r�b�g�}�b�v�̃r�b�gCONST BITMAPINFO *lpBitsInfo, // �r�b�g�}�b�v�̃f�[�^UINT iUsage,// �f�[�^�̎�� DWORD dwRop// ���X�^�I�y���[�V�����R�[�h);
	::EndPaint(hwndObjectID,&ps);
}

void CMonitoringSystemDlg::Imagetrack(unsigned char *pix){
	//�摜�o��
	::BeginPaint(hwndtrack,&ps);//�w�肳�ꂽ�E�B���h�E�ɑ΂��ĕ`��̏���(HWND  hwnd,//  �E�B���h�E�̃n���h�� LPPAINTSTRUCT  lpPaint  //  �`��������\���̂ւ̃|�C���^);
	SetStretchBltMode(hdctrack,COLORONCOLOR);//�w�肳�ꂽ�f�o�C�X�Ɨ��r�b�g�}�b�v�iDIB�j���̒����`�s�N�Z���̐F�f�[�^���A�w�肳�ꂽ�����`�փR�s�[(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h�� int iStretchMode // �r�b�g�}�b�v�̐L�k���[�h�s�N�Z�����폜���܂��B��菜���_�̏������炩�̌`�ňێ����悤�Ƃ͂����A�P���ɂ����̓_���폜���܂��B);
	StretchDIBits(hdctrack,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h��int XDest, // �R�s�[�撷���`�̍������ x ���W int YDest,// �R�s�[�撷���`�̍������ y ���Wint nDestWidth, // �R�s�[�撷���`�̕�int nDestHeight,// �R�s�[�撷���`�̍���int XSrc,// �R�s�[�������`�̍������ x ���Wint YSrc,// �R�s�[�������`�̍������ x ���Wint nSrcWidth,// �R�s�[�������`�̕�int nSrcHeight,// �R�s�[�������`�̍���
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// �r�b�g�}�b�v�̃r�b�gCONST BITMAPINFO *lpBitsInfo, // �r�b�g�}�b�v�̃f�[�^UINT iUsage,// �f�[�^�̎�� DWORD dwRop// ���X�^�I�y���[�V�����R�[�h);
	::EndPaint(hwndtrack,&ps);
}

void CMonitoringSystemDlg::Imagedepth(unsigned char *pix){
	//�摜�o��
	::BeginPaint(hwnddepth,&ps);//�w�肳�ꂽ�E�B���h�E�ɑ΂��ĕ`��̏���(HWND  hwnd,//  �E�B���h�E�̃n���h�� LPPAINTSTRUCT  lpPaint  //  �`��������\���̂ւ̃|�C���^);
	SetStretchBltMode(hdcdepth,COLORONCOLOR);//�w�肳�ꂽ�f�o�C�X�Ɨ��r�b�g�}�b�v�iDIB�j���̒����`�s�N�Z���̐F�f�[�^���A�w�肳�ꂽ�����`�փR�s�[(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h�� int iStretchMode // �r�b�g�}�b�v�̐L�k���[�h�s�N�Z�����폜���܂��B��菜���_�̏������炩�̌`�ňێ����悤�Ƃ͂����A�P���ɂ����̓_���폜���܂��B);
	StretchDIBits(hdcdepth,0,0,320,240,0,0,320,240,//int StretchDIBits(HDC hdc,// �f�o�C�X�R���e�L�X�g�̃n���h��int XDest, // �R�s�[�撷���`�̍������ x ���W int YDest,// �R�s�[�撷���`�̍������ y ���Wint nDestWidth, // �R�s�[�撷���`�̕�int nDestHeight,// �R�s�[�撷���`�̍���int XSrc,// �R�s�[�������`�̍������ x ���Wint YSrc,// �R�s�[�������`�̍������ x ���Wint nSrcWidth,// �R�s�[�������`�̕�int nSrcHeight,// �R�s�[�������`�̍���
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);//CONST VOID *lpBits,// �r�b�g�}�b�v�̃r�b�gCONST BITMAPINFO *lpBitsInfo, // �r�b�g�}�b�v�̃f�[�^UINT iUsage,// �f�[�^�̎�� DWORD dwRop// ���X�^�I�y���[�V�����R�[�h);
	::EndPaint(hwnddepth,&ps);
}


//���o�摜�o��2
void CMonitoringSystemDlg::Detect2Out(unsigned char *pix, int srcX, int srcY){
	::BeginPaint(hwndDetect2,&ps);
	SetStretchBltMode(hdcDetect2,COLORONCOLOR);
	StretchDIBits(hdcDetect2,0,0,320,140, srcX,srcY,320,140,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndDetect2,&ps);
}

//���o�摜�o��3
void CMonitoringSystemDlg::Detect3Out(unsigned char *pix, int srcX, int srcY){
	::BeginPaint(hwndDetect3,&ps);
	SetStretchBltMode(hdcDetect3,COLORONCOLOR);
	StretchDIBits(hdcDetect3,0,0,320,140,srcX,srcY,320,140,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndDetect3,&ps);
}

//���C���摜�o��
void CMonitoringSystemDlg::LayerOut(unsigned char *pix){
	::BeginPaint(hwndLayer,&ps);
	SetStretchBltMode(hdcLayer,COLORONCOLOR);
	StretchDIBits(hdcLayer,0,0,320,240,0,0,320,240,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndLayer,&ps);
}

//�J�����摜�o��
void CMonitoringSystemDlg::showCamera(unsigned char *pix){
	::BeginPaint(hwndCamera,&ps);
	SetStretchBltMode(hdcCamera,COLORONCOLOR);
	StretchDIBits(hdcCamera,0,0,320,240,0,0,320,240,
		pix,&bmpInfo,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndCamera,&ps);
}

//�I�t���C���T���l�C���o��
void CMonitoringSystemDlg::showThumnail(unsigned char *pix){
	::BeginPaint(hwndThumnail,&ps);
	SetStretchBltMode(hdcThumnail,COLORONCOLOR);
	StretchDIBits(hdcThumnail,0,0,160,120,0,0,160,120,
		pix,&bmpInfoThumbnail,DIB_RGB_COLORS,SRCCOPY);
	::EndPaint(hwndThumnail,&ps);
}
//���ʉ摜�o��
void CMonitoringSystemDlg::DetectOut(){
	for( int i = 0 ; i < min( count, 3 ) ; ++i)
	{
		SceneOut(count-1-i,i+1);
		DetectLoad(obj[count-1-i].frame,obj[count-1-i].id,i+1, obj[count-1-i].cx,obj[count-1-i].cy);
		string keyString = "Event"+Util::toString( i+1 );
		string keyString_label = "�C�x���g����"+Util::toString( i+1 )+"���x��";

		//�l��ID������ID��
		if( obj[count-1-i].scene == true )
		{
			if(obj[count-1-i].track == true )//kawa
			{
				//���m�摜�o�͗̈�
			    string name_data="�l��ID:"+Util::toString(obj[count-1-i].human)+"������ID:"+Util::toString(obj[count-1-i].id)+"���ړ�";
				imagepack[ keyString ] = MultiData(name_data, "string",  "�ړ�"  );
			}
			else
			{string name_data="�l��ID:"+Util::toString(obj[count-1-i].human)+"������ID:"+Util::toString(obj[count-1-i].id)+"����������";
				imagepack[ keyString ] = MultiData( name_data, "string",  "��������"  );
			}

		}
		else
		{string name_data="�l��ID:"+Util::toString(obj[count-1-i].human)+"������ID:"+Util::toString(obj[count-1-i].id)+"����������";
			imagepack[ keyString ] = MultiData( name_data, "string",  "��������"  );
		}

	}
	disp_flag=true;
}

//���o�摜�ǂݍ���
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
		imagepack["event_image3"] = MultiData( Util::convertMat2String(cap),"image", "�C�x���g����3");
		cv::flip(cap,cap,0);
		Detect3Out(&cap.at<cv::Vec3b>(0,0)[0], sx,sy);
	}
	else if( flag == 2 )
	{
		imagepack["event_image2"] = MultiData( Util::convertMat2String(cap),"image", "�C�x���g����2");
		cv::flip(cap,cap,0);
		Detect2Out(&cap.at<cv::Vec3b>(0,0)[0], sx,sy);
	}
	else if( flag == 1 )
	{
		imagepack["event_image1"] = MultiData( Util::convertMat2String(cap),"image", "�C�x���g����1");
		cv::flip(cap,cap,0);
		Detect1Out(&cap.at<cv::Vec3b>(0,0)[0], sx,sy);
	}

}


//-------------------------------------------------
// Name     : 
// Author   : Kzuhiro MAKI (CV-lab.)
// Updata   : 2007.06.29
// Function : ���o���̏ڍ׏��o��
// Argument : pix:�o�͉摜�f�[�^
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene, int track_flag,int key_point_parent_id,int human){
	//�V�[�������̍X�V
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

	//�@�\���̂����������܂��B
	stBInfo.pidlRoot = NULL;	//���[�g�t�H���_�ł��B
	stBInfo.hwndOwner = p_hWnd;	//�\������_�C�A���O�̐e�E�B���h�E�̃n���h���ł��B
	stBInfo.pszDisplayName = chPutFolder;	//�I������Ă���t�H���_�����󂯂܂��B
	stBInfo.lpszTitle = p_cCaptionStr;	//�����̕�����ł��B
	stBInfo.ulFlags = p_uiFlags;	//�\���t���O�ł��B
	stBInfo.lpfn = NULL;	//�_�C�A���O�v���V�[�W���ւ̃|�C���^�ł��B
	stBInfo.lParam = 0L;	//�v���V�[�W���ɑ���p�����[�^�[�ł��B

	//�@�_�C�A���O�{�b�N�X��\�����܂��B
	pidlRetFolder = ::SHBrowseForFolder( &stBInfo );

	//�@pidlRetFolder�ɂ͂��̃t�H���_��\���A�C�e���h�c���X�g�ւ̃|�C���^��
	//�@�����Ă��܂��BchPutFolder�ɂ͑I�����ꂽ�t�H���_���i��t���p�X�j����
	//�@���������Ă��Ȃ��̂ŁA���̃|�C���^�𗘗p���܂��B

	if( pidlRetFolder != NULL )
	{
		//�@�t���p�X���擾���܂��B
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
// Function : ���m�摜�ɃV�[�����߂�ǉ�
// Argument : id�F���̌��m�ԍ�
//             x�F�o�͂���ʒu(x���W)
//             y�F�o�͂���ʒu(y���W)
// return   : �Ȃ�
//-------------------------------------------------
void CMonitoringSystemDlg::SceneOut(int num,int flag){
	char msg[4]; //�o�͕�����

	if(flag==1)
	{
		((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_SCENE1))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel("\r\n");
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel("\r\n");
		//ID�o��
		_itoa_s(obj[num].key_point_parent_id,msg,10);  //10�i���ŕϊ�
		((CEdit*)GetDlgItem(IDC_ID1))->ReplaceSel(msg);
		//�V�[�����ߏo��
		if(obj[num].scene==true)
		{
			if(obj[num].track == true)
			{
				//���m�摜�o�͗̈�
				CDC* pDC = detect1.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(0,255,0));
				pDC->TextOut(0,0,"�ړ�    ");
			}
			else
			{
				//���m�摜�o�͗̈�
				CDC* pDC = detect1.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->TextOut(0,0,"��������");
			}
			//((CEdit*)GetDlgItem(IDC_SCENE1))->ReplaceSel("��������");
		}
		else
		{
			//���m�摜�o�͗̈�
			CDC* pDC = detect1.GetDC();
			pDC->SelectObject(&font);
			pDC->SetTextColor(RGB(0,0,255));
			pDC->TextOut(0,0,"��������");
			//((CEdit*)GetDlgItem(IDC_SCENE1))->ReplaceSel("��������");
		}
		//�����o��
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel(obj[num].hour);
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel("�F");
		((CEdit*)GetDlgItem(IDC_TIME1))->ReplaceSel(obj[num].minute);
		//�d�S�o��
		_itoa_s(obj[num].cx,msg,10);  //10�i���ŕϊ�
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel("(");
		((CEdit*)GetDlgItem(IDC_POSITION1))->ReplaceSel(msg);
		_itoa_s(obj[num].cy,msg,10);  //10�i���ŕϊ�
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
				//���m�摜�o�͗̈�
				CDC* pDC = detect2.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(0,255,0));
				pDC->TextOut(0,0,"�ړ�    ");
			}
			else
			{
				//���m�摜�o�͗̈�
				CDC* pDC = detect2.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->TextOut(0,0,"��������");
			}
			//((CEdit*)GetDlgItem(IDC_SCENE2))->ReplaceSel("��������");
		}
		else
		{
			CDC* pDC = detect2.GetDC();
			pDC->SelectObject(&font);
			pDC->SetTextColor(RGB(0,0,255));
			pDC->TextOut(0,0,"�������� ");
			//((CEdit*)GetDlgItem(IDC_SCENE2))->ReplaceSel("��������");
		}
		((CEdit*)GetDlgItem(IDC_TIME2))->ReplaceSel(obj[num].hour);
		((CEdit*)GetDlgItem(IDC_TIME2))->ReplaceSel("�F");
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
				//���m�摜�o�͗̈�
				CDC* pDC = detect3.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(0,255,0));
				pDC->TextOut(0,0,"�ړ�    ");
			}
			else
			{
				//���m�摜�o�͗̈�
				CDC* pDC = detect3.GetDC();
				pDC->SelectObject(&font);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->TextOut(0,0,"��������");
			}
				//((CEdit*)GetDlgItem(IDC_SCENE3))->ReplaceSel("��������");
		}
		else
		{
			CDC* pDC = detect3.GetDC();
			pDC->SelectObject(&font);
			pDC->SetTextColor(RGB(0,0,255));
			pDC->TextOut(0,0,"��������");
			//((CEdit*)GetDlgItem(IDC_SCENE3))->ReplaceSel("��������");
		}
		((CEdit*)GetDlgItem(IDC_TIME3))->ReplaceSel(obj[num].hour);
		((CEdit*)GetDlgItem(IDC_TIME3))->ReplaceSel("�F");
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
// Function : ������
// Argument : �Ȃ�
// return   : �Ȃ�
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

	//�t�H���g�̍쐬
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
		_T("MS P�S�V�b�N")
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

	//�J������1(0index)�ɐݒ肵�Ă���
	m_control_CameraList.SetCurSel(0);//133.19.22.240�I��
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
				mode = MODE_ONLINE;     //���[�h�̐ݒ�
				CWnd::SetDlgItemTextA(IDC_ONLINE, "STOP");
			}
			else if( mode == MODE_ONLINE )
			{
				mode = MODE_NONE;
				CWnd::SetDlgItemTextA(IDC_ONLINE, "ONLINE");
			}
			//���̃N���X�̃|�C���^��n��
			camera.set_pParent(this);
			AfxBeginThread(ThreadProcCalc, &camera,THREAD_PRIORITY_BELOW_NORMAL);
		}
	}
}

//�����{�^��(�O)
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
		++tmp_count; //����炵���̂�߂�
	}
}

//�����{�^���i��j
void CMonitoringSystemDlg::OnBnClickedButton2(){
	tmp_count++; //���ݕ\�����Ă��镔���̈��

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

//�����{�^���i�ŐV�j
void CMonitoringSystemDlg::OnBnClickedButton3(){
	DetectOut();
	disp_flag = true; //����\�����ŐV
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
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	UpdateData(TRUE);
	showNCamera(m_cameraId);
}


void CMonitoringSystemDlg::OnBnClickedButton4(){
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	char file[128];
	char tmp[10];
	cv::Mat tracking;


	//�摜���Ȃ��ꍇ��i�����炵�Ă��邩�ǂ����T��
	while( 1 )
	{
		sprintf_s(file,"../DB/track_img/");
		sprintf_s(tmp,"%08lu",track_frame);
		strcat_s(file,tmp);
		strcat_s(file,".png");

		if( track_frame < 1 )
		{ //�I��
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
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	char file[128];
	char tmp[10];
	//obj[count-1].frame;
	cv::Mat tracking;


	//�摜���Ȃ��ꍇ��i�����炵�Ă��邩�ǂ����T��
	while( 1 )
	{
		sprintf_s(file,"../DB/track_img/");
		sprintf_s(tmp,"%08lu",track_frame);
		strcat_s(file,tmp);
		strcat_s(file,".png");

		if( track_frame>obj[count-1].frame )
		{ //�I��
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
	browseinfo.lpszTitle = _T("�t�H���_��I�����Ă�������");
	browseinfo.ulFlags = BIF_RETURNONLYFSDIRS;
	browseinfo.lParam = (LPARAM)lpszDefault;
	browseinfo.lpfn = _BrowseCallbackProc;

	if( !(lpitemidlist = SHBrowseForFolder(&browseinfo)) )
	{
		AfxMessageBox(_T("�t�H���_��I�����Ă��܂���ł���"));
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