/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/

#ifndef	MODEDT_H__
#define	MODEDT_H__

#include	"CommonDef.h"

/* �猟�o���[�h */
enum DT_MODE {
	DT_MODE_DEFAULT = 0				/* �f�t�H���g */
};

/* ��摜���� */
#define	DETECT_UP		0x01		/* �������摜 */
#define	DETECT_RIGHT	0x02		/* �E������摜 */
#define	DETECT_LEFT		0x04		/* ��������摜 */
#define	DETECT_DOWN		0x08		/* ��������摜 */

/* �΂߉������͈� */
#define	DETECT_FRONT		0x01	/* ���ʊ� */
#define	DETECT_HALF_PROFILE	0x03	/* �΂߉��� */

/* ��摜�p�x�͈� */
enum DETECT_ANGLE {
	DETECT_1ANGLE = 0,				/* �P�p�x���i0���j */
	DETECT_3ANGLE					/* �R�p�x���i0���A-30���A+30���j */
};

/* �猟�o�T�C�Y���[�h */
enum DETECT_FACE_SIZE_MODE {
	DETECT_FACE_PIXEL = 0,			/* �s�N�Z���w�� */
	DETECT_FACE_RATIO				/* ����(%)�w�� */
};

#endif	/* MODEDT_H__ */
