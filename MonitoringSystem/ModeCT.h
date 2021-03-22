/*--------------------------------------------------------------------*/
/*  Copyright(C) 2004-2006 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/

#ifndef	MODECT_H__
#define	MODECT_H__

#include	"CommonDef.h"

enum CT_MODE {
	CT_MODE_DEFAULT = 0,		/* �f�t�H���g(�W��) */
	CT_MODE_FAST				/* ����(����p) */
};

/*----- ��튯�֊s���o�_�C���f�b�N�X -----*/
enum CONTOUR_POINT {
	CONTOUR_EYEL_1 = 0,			/* ���� ��P�_ */
	CONTOUR_EYEL_2,				/* ���� ��Q�_ */
	CONTOUR_EYEL_3,				/* ���� ��R�_ */
	CONTOUR_EYEL_4,				/* ���� ��S�_ */
	CONTOUR_EYEL_5,				/* ���� ��T�_ */
	CONTOUR_EYEL_6,				/* ���� ��U�_ */
	CONTOUR_EYEL_7,				/* ���� ��V�_ */
	CONTOUR_EYEL_8,				/* ���� ��W�_ */
	CONTOUR_EYEBROWL_1,			/* ���� ��P�_ */
	CONTOUR_EYEBROWL_2,			/* ���� ��Q�_ */
	CONTOUR_EYEBROWL_3,			/* ���� ��R�_ */
	CONTOUR_EYEBROWL_4,			/* ���� ��S�_ */
	CONTOUR_EYEBROWL_5,			/* ���� ��T�_ */
	CONTOUR_EYEBROWL_6,			/* ���� ��U�_ */
	CONTOUR_EYEBROWL_7,			/* ���� ��V�_ */
	CONTOUR_EYEBROWL_8,			/* ���� ��W�_ */
	CONTOUR_EYER_1,				/* �E�� ��P�_ */
	CONTOUR_EYER_2,				/* �E�� ��Q�_ */
	CONTOUR_EYER_3,				/* �E�� ��R�_ */
	CONTOUR_EYER_4,				/* �E�� ��S�_ */
	CONTOUR_EYER_5,				/* �E�� ��T�_ */
	CONTOUR_EYER_6,				/* �E�� ��U�_ */
	CONTOUR_EYER_7,				/* �E�� ��V�_ */
	CONTOUR_EYER_8,				/* �E�� ��W�_ */
	CONTOUR_EYEBROWR_1,			/* �E�� ��P�_ */
	CONTOUR_EYEBROWR_2,			/* �E�� ��Q�_ */
	CONTOUR_EYEBROWR_3,			/* �E�� ��R�_ */
	CONTOUR_EYEBROWR_4,			/* �E�� ��S�_ */
	CONTOUR_EYEBROWR_5,			/* �E�� ��T�_ */
	CONTOUR_EYEBROWR_6,			/* �E�� ��U�_ */
	CONTOUR_EYEBROWR_7,			/* �E�� ��V�_ */
	CONTOUR_EYEBROWR_8,			/* �E�� ��W�_ */
	CONTOUR_NOSE_1,				/* �@ ��P�_ */
	CONTOUR_NOSE_2,				/* �@ ��Q�_ */
	CONTOUR_NOSE_3,				/* �@ ��R�_ */
	CONTOUR_NOSE_4,				/* �@ ��S�_ */
	CONTOUR_NOSE_5,				/* �@ ��T�_ */
	CONTOUR_NOSE_6,				/* �@ ��U�_ */
	CONTOUR_NOSE_7,				/* �@ ��V�_ */
	CONTOUR_NOSE_8,				/* �@ ��W�_ */
	CONTOUR_NOSE_9,				/* �@ ��X�_ */	
	CONTOUR_NOSE_10,			/* �@ ��P�O�_ */
	CONTOUR_NOSE_11,			/* �@ ��P�P�_ */
	CONTOUR_NOSE_12,			/* �@ ��P�Q�_ */
	CONTOUR_MOUTH_1,			/* �� ��P�_ */
	CONTOUR_MOUTH_2,			/* �� ��Q�_ */
	CONTOUR_MOUTH_3,			/* �� ��R�_ */
	CONTOUR_MOUTH_4,			/* �� ��S�_ */
	CONTOUR_MOUTH_5,			/* �� ��T�_ */
	CONTOUR_MOUTH_6,			/* �� ��U�_ */
	CONTOUR_MOUTH_7,			/* �� ��V�_ */
	CONTOUR_MOUTH_8,			/* �� ��W�_ */
	CONTOUR_MOUTH_9,			/* �� ��X�_ */	
	CONTOUR_MOUTH_10,			/* �� ��P�O�_ */
	CONTOUR_MOUTH_11,			/* �� ��P�P�_ */
	CONTOUR_MOUTH_12,			/* �� ��P�Q�_ */
	CONTOUR_MOUTH_13,			/* �� ��P�R�_ */
	CONTOUR_MOUTH_14,			/* �� ��P�S�_ */
	CONTOUR_MOUTH_15,			/* �� ��P�T�_ */
	CONTOUR_MOUTH_16,			/* �� ��P�U�_ */
	CONTOUR_MOUTH_17,			/* �� ��P�V�_ */
	CONTOUR_MOUTH_18,			/* �� ��P�W�_ */
	CONTOUR_MOUTH_19,			/* �� ��P�X�_ */
	CONTOUR_MOUTH_20,			/* �� ��Q�O�_ */
	CONTOUR_MOUTH_21,			/* �� ��Q�P�_ */
	CONTOUR_MOUTH_22,			/* �� ��Q�Q�_ */
	CONTOUR_FACE_1,				/* ��֊s ��P�_ */
	CONTOUR_FACE_2,				/* ��֊s ��Q�_ */
	CONTOUR_FACE_3,				/* ��֊s ��R�_ */
	CONTOUR_FACE_4,				/* ��֊s ��S�_ */
	CONTOUR_FACE_5,				/* ��֊s ��T�_ */
	CONTOUR_FACE_6,				/* ��֊s ��U�_ */
	CONTOUR_FACE_7,				/* ��֊s ��V�_ */
	CONTOUR_FACE_8,				/* ��֊s ��W�_ */
	CONTOUR_FACE_9,				/* ��֊s ��X�_ */	
	CONTOUR_FACE_10,			/* ��֊s ��P�O�_ */
	CONTOUR_FACE_11,			/* ��֊s ��P�P�_ */
	CONTOUR_FACE_12,			/* ��֊s ��P�Q�_ */
	CONTOUR_FACE_13,			/* ��֊s ��P�R�_ */
	CONTOUR_FACE_14,			/* ��֊s ��P�S�_ */
	CONTOUR_FACE_15,			/* ��֊s ��P�T�_ */
	CONTOUR_FACE_16,			/* ��֊s ��P�U�_ */
	CONTOUR_FACE_17,			/* ��֊s ��P�V�_ */
	CONTOUR_FACE_18,			/* ��֊s ��P�W�_ */
	CONTOUR_FACE_19,			/* ��֊s ��P�X�_ */
	CONTOUR_FACE_20,			/* ��֊s ��Q�O�_ */
	CONTOUR_FACE_21,			/* ��֊s ��Q�P�_ */
	CONTOUR_KIND_MAX			/* ���o�_�̐� */
};

/*----- ��튯�֊s�̖������W�l -----*/
#define	CONTOUR_NO_POINT	-1		/* x,y �����ɐݒ� */

#endif	/* MODECT_H__ */
