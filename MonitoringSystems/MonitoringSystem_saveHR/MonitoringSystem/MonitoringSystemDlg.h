// MonitoringSystemDlg.h : �w�b�_�[ �t�@�C��
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
	bool scene;  //true:�������� false:��������
	bool track;  //true:�ړ�     false:��������
} record;

// CMonitoringSystemDlg �_�C�A���O
//enum ModeType{ONLINE,OFFLINE};
class CMonitoringSystemDlg : public CDialog
{
private:
	//CEdit detect;  //���m�摜�`��̈�
	BITMAPINFO bmpInfo; //bmp�t�@�C���p
	BITMAPINFO bmpInfoThumbnail;
	PAINTSTRUCT ps;

	HWND hwndImage;  //���A���^�C���摜�o�͗p�̈�
	HDC hdcImage;    //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndDiff; //�����摜�o�͗p�̈�
	HDC hdcDiff;   //�f�o�C�X�R���e�L�X�g�̃n���h��

	//HWND hwndPosi;  //keypoint�摜�o�͗p�̈�
	//HDC hdcPosi;    //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndDetect1; //���m�摜�o�͗p�̈�
	HDC hdcDetect1;   //�f�o�C�X�R���e�L�X�g�̃n���h��
	HWND hwndDetect2; //���m�摜�o�͗p�̈�
	HDC hdcDetect2;   //�f�o�C�X�R���e�L�X�g�̃n���h��
	HWND hwndDetect3; //���m�摜�o�͗p�̈�
	HDC hdcDetect3;   //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndLayer; //���m�摜�o�͗p�̈�
	HDC hdcLayer;   //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndCamera; //���m�摜�o�͗p�̈�
	HDC hdcCamera;   //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndThumnail; //���m�摜�o�͗p�̈�
	HDC hdcThumnail;   //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndObjectID;  //���A���^�C���摜�o�͗p�̈�
	HDC hdcObjectID;    //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndtrack;  //�ǐՊm�F�p�摜�o�͗p�̈�
	HDC hdctrack;    //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwnddepth;  //�����摜�p�摜�o�͗p�̈�
	HDC hdcdepth;    //�f�o�C�X�R���e�L�X�g�̃n���h��

	HWND hwndskelton;  //���i�摜�p�摜�o�͗p�̈�
	HDC hdcskelton;    //�f�o�C�X�R���e�L�X�g�̃n���h��

	CEdit detect1; //�����`��̈�
	CEdit detect2;
	CEdit detect3;
	CFont font;   //�\���t�H���g

	record obj[10];
	int count;     //���̐��p�i�ő�10�j
	int tmp_count; //����������p
	bool disp_flag;     //�����̕\�����ŐV���ǂ���

	int mode; //�I�����C��or�I�t���C��

	int track_frame;

public:

	std::map<std::string, MultiData> imagepack;

	CString BrowseForFolder( HWND p_hWnd, CString p_cSetStr
		, CString p_cRootStr, CString p_cCaptionStr, UINT p_uiFlags );


	void Disp_Log(char *msg);           //�_�C�A���O�Ɍ��m�󋵂��o��
	void SceneOut(int num,int flag);  //���m�摜�ɃV�[�����߂�ǉ�

	//�V�[���󋵂̃Z�b�g
	void SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene, int track_flag,int key_point_parent_id,int human); 

	void ImageOut(unsigned char *pix);   //���A���^�C���摜�o��
	void DiffOut(unsigned char *pix);    //�����摜�o��
	//void PosiOut(unsigned char *pix);   //keypoint�摜�o��
	void Detect1Out(unsigned char *pix, int srcX  = 0, int srcY = 0); //���m�摜�o��
	void Detect2Out(unsigned char *pix, int srcX  = 0, int srcY = 0); //���m�摜�o��
	void Detect3Out(unsigned char *pix, int srcX  = 0, int srcY = 0); //���m�摜�o��

	void LayerOut(unsigned char *pix); //���m�摜�o��

	void showCamera(unsigned char *pix); //���m�摜�o��
	void showThumnail(unsigned char *pix); //���m�摜�o��

	void ImageObjectID(unsigned char *pix);
	void Imagetrack(unsigned char *pix);
	void Imagedepth(unsigned char *pix);
	void ImageSkelton(unsigned char *pix);

	void DetectOut();
	void DetectLoad(int num,int id,int flag, int cx = 0, int cy = 0);


	void Init(); //�F�X������
	unsigned char* getImageFromImageFile(CString strPath);
	void showNCamera(int n);
	int getMode();
	// �R���X�g���N�V����
public:
	CMonitoringSystemDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^

	void setStrFps(CString);
	CString getStrDirPathOfflineInput();

	// �_�C�A���O �f�[�^
	enum { IDD = IDD_MONITORINGSYSTEM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g


	// ����
protected:
	HICON m_hIcon;




	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
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
