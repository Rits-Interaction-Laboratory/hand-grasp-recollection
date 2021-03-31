/*--------------------------------------------------------------------*/
/*  Copyright(C) 2004-2006 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	�֊s���o���C�u�����`�o�h
*/
/*
	�X�V�����F
	------------------------------------------------------------------------------
	�N����		�X�V���e				���l
	06-12-28	�V�A���S���Y���Ή�		StartStage/EndStage�̐ݒ�^�擾�֐��V��
	------------------------------------------------------------------------------
*/

#ifndef	OKAOCTAPI_H__
#define	OKAOCTAPI_H__

#include	"OkaoAPI.h"
#include	"ModeCT.h"

#ifdef	OKAO_BUILD
#include	"SdkCT.h"
#else
typedef	void *			HCONTOUR;		/* �֊s���o�n���h�� */
typedef	void *			HCTRESULT;		/* �֊s���o���ʃn���h�� */
typedef	void *			HPTRESULT;		/* ��튯���o���ʃn���h�� */
#endif	/* OKAO_BUILD */

/* ���C�u�����w�� */
#define	OKAO_CONTOUR	0x00000800UL		/* ��튯�֊s���o */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- �֊s���o�n���h���̍쐬�^�폜 -----*/
OKAO_API HCONTOUR	OKAO_CreateContour(enum CT_MODE mode);	/* �֊s���o�n���h���쐬 */
OKAO_API int		OKAO_DeleteContour(HCONTOUR hCT);		/* �֊s���o�n���h���폜 */
OKAO_API HCTRESULT	OKAO_CreateCtResult(void);				/* �֊s���o���ʃn���h���쐬 */
OKAO_API int		OKAO_DeleteCtResult(HCTRESULT hResult);	/* �֊s���o���ʃn���h���폜 */

/*----- ��튯�ݒ� -----*/
OKAO_API int 		OKAO_SetCtPosition(HCONTOUR hCT, HPTRESULT hResult);
															/*-��튯���o���ʃn���h�������튯��ݒ� */
/*----- �֊s���o -----*/
OKAO_API int		OKAO_Contour(HCONTOUR hCT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HCTRESULT hResult);
/*----- �֊s���o���� -----*/
OKAO_API int		OKAO_GetCtPointNumber(HCTRESULT hResult, int *pnCount);
															/*-�֊s���o���ʐ��̎擾 */
OKAO_API int		OKAO_GetCtPoint(HCTRESULT hResult, POINT aptContour[]);
															/*-�֊s���o���ʂ̎擾 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOCTAPI_H__ */
