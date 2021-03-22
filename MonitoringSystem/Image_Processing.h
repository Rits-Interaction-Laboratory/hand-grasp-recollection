// Image_Processing.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CImage_ProcessingApp:
// このクラスの実装については、Image_Processing.cpp を参照してください。
//

class CImage_ProcessingApp : public CWinApp
{
public:
	CImage_ProcessingApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CImage_ProcessingApp theApp;