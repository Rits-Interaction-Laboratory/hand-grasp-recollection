// Image_Processing.h : PROJECT_NAME �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C���ł��B
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH �ɑ΂��Ă��̃t�@�C�����C���N���[�h����O�� 'stdafx.h' ���C���N���[�h���Ă�������"
#endif

#include "resource.h"		// ���C�� �V���{��


// CImage_ProcessingApp:
// ���̃N���X�̎����ɂ��ẮAImage_Processing.cpp ���Q�Ƃ��Ă��������B
//

class CImage_ProcessingApp : public CWinApp
{
public:
	CImage_ProcessingApp();

// �I�[�o�[���C�h
	public:
	virtual BOOL InitInstance();

// ����

	DECLARE_MESSAGE_MAP()
};

extern CImage_ProcessingApp theApp;