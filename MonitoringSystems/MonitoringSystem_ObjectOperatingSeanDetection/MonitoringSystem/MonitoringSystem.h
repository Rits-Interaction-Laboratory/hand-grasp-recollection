// MonitoringSystem.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CMonitoringSystemApp:
// このクラスの実装については、MonitoringSystem.cpp を参照してください。
//

class CMonitoringSystemApp : public CWinApp
{
public:
	CMonitoringSystemApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CMonitoringSystemApp theApp;