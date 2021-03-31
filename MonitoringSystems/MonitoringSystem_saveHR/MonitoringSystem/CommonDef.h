/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/*
	�X�V�����F
	------------------------------------------------------------------------------
	�N����		�X�V���e				���l
	03-09-02	enum FEATURE_POINT		��튯���o�̋@�\�ǉ��̂���
	03-10-06	FEATURE_NO_POINT �ǉ�	POINT[FEATURE_***] �̖������W���`
	04-02-12	OKAO_TIMEOUT �ǉ�		���ʃR�[�h�ɏ����^�C���A�E�g���`
	06-10-11	�G���[�R�[�h�ǉ�		���ʃR�[�h�ɖ��Ή��C���[�h�Ⴂ�C�\��C�n���h���G���[���`
	07-11-20	DSP�őΉ�				�d���o�^�G���[��ǉ���`
	------------------------------------------------------------------------------
*/
#ifndef	COMMONDEF_H__
#define	COMMONDEF_H__

typedef unsigned char	RAWIMAGE;		/* �q�`�v�C���[�W�f�[�^ */

/*----- ��튯�C���f�b�N�X -----*/
enum FEATURE_POINT {
	FEATURE_LEFT_EYE = 0,				/* ���ځi�����j */
	FEATURE_RIGHT_EYE,					/* �E�ځi�����j */
	FEATURE_MOUTH,						/* ���i�����j */
	/* SDK Ver 2.0 �ȍ~ */
	FEATURE_LEFT_EYE_IN,				/* ���� �ړ� */
	FEATURE_LEFT_EYE_OUT,				/* ���� �ڐK */
	FEATURE_RIGHT_EYE_IN,				/* �E�� �ړ� */
	FEATURE_RIGHT_EYE_OUT,				/* �E�� �ڐK */
	FEATURE_MOUTH_LEFT,					/* ���� �� */
	FEATURE_MOUTH_RIGHT,				/* ���� �E */
	FEATURE_LEFT_EYE_PUPIL,				/* ���� �� */
	FEATURE_RIGHT_EYE_PUPIL,			/* �E�� �� */
	FEATURE_KIND_MAX					/* �C���f�b�N�X�̐� */
};

/*----- ��튯�̖������W�l -----*/
#define	FEATURE_NO_POINT		-1			/* x,y �����ɐݒ� */


/*----- OKAO API ���ʃR�[�h -----*/
#define		OKAO_NORMAL				0		/* ����I�� */
#define		OKAO_TIMEOUT			1		/* �����^�C���A�E�g */
#define		OKAO_NOTIMPLEMENTED		2		/* ���󖢑Ή��̋@�\ */
#define		OKAO_ERR_VARIOUS		-1		/* ����`�G���[ */
#define		OKAO_ERR_INITIALIZE		-2		/* �������G���[ */
#define		OKAO_ERR_INVALIDPARAM	-3		/* �����G���[ */
#define		OKAO_ERR_ALLOCMEMORY	-4		/* �������m�ۃG���[ */
#define		OKAO_ERR_MODEDIFFERENT	-5		/* �w�胂�[�h�Ⴂ�G���[ */
#define		OKAO_ERR_NOALLOC		-6		/* (�g�ݍ��ݗp) */
#define		OKAO_ERR_NOHANDLE		-7		/* �n���h���G���[ */
#define		OKAO_ERR_DUPLICATE		-8		/* �d���o�^�G���[ */

#endif	/* COMMONDEF_H__ */

