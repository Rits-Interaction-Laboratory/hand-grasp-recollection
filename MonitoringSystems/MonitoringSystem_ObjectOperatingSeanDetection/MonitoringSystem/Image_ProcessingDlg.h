// Image_ProcessingDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once

#include "network.h"


// CImage_ProcessingDlg �_�C�A���O
class CImage_ProcessingDlg : public CDialog
{
// �R���X�g���N�V����
public:
	CImage_ProcessingDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^

// �_�C�A���O �f�[�^
	enum { IDD = IDD_IMAGE_PROCESSING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g


//�ȉ������Œǉ�



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
	afx_msg void OnBnClickedCapture();
public:
	afx_msg void OnBnClickedOffline();
};
