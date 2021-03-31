// Image_ProcessingDlg.h : ヘッダー ファイル
//

#pragma once

#include "network.h"


// CImage_ProcessingDlg ダイアログ
class CImage_ProcessingDlg : public CDialog
{
// コンストラクション
public:
	CImage_ProcessingDlg(CWnd* pParent = NULL);	// 標準コンストラクタ

// ダイアログ データ
	enum { IDD = IDD_IMAGE_PROCESSING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


//以下自分で追加



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
	afx_msg void OnBnClickedCapture();
public:
	afx_msg void OnBnClickedOffline();
};
