/*--------------------------------------------------------------------*/
/*  Copyright(C) 2007-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/*
	�X�V�����F
	------------------------------------------------------------------------------
	�N����		�X�V���e				���l
	07-01-25	�V�K�쐬				SDK Ver4 �Ή�
	------------------------------------------------------------------------------
*/

#ifndef	MODEFR_H__
#define	MODEFR_H__

#include	"CommonDef.h"

#define USER_ID_LENGTH 16			/* �l���ʂh�c�̒��� */
typedef unsigned char FR_USER_ID;	/* �l���ʂh�c�i16�o�C�g��1�̂h�c�j */

/* �f�[�^�쐬���[�h */
enum FR_MODE {
	FR_MODE_DEFAULT = 0				/* �f�t�H���g(�W��) */
};

#endif	/* MODEFR_H__ */
