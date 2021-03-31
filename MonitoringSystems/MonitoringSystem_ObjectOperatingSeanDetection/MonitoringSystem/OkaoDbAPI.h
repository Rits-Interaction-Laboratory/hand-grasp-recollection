/*--------------------------------------------------------------------*/
/*  Copyright(C) 2007-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	��F�؃f�[�^�x�[�X���C�u�����`�o�h
*/
/*
	�X�V�����F
	------------------------------------------------------------------------------
	�N����		�X�V���e				���l
	07-01-25	�V�K�쐬				SDK Ver4 �Ή�
	------------------------------------------------------------------------------
*/

#ifndef	OKAODBAPI_H__
#define	OKAODBAPI_H__

#include	"OkaoAPI.h"
#include	"OkaoFrAPI.h"
#include	"ModeDB.h"

#ifdef	OKAO_BUILD
#include	"SdkFR.h"
#else
typedef	void *			HALBUMDB;		/* �A���o���f�[�^�x�[�X�n���h�� */
#endif	/* OKAO_BUILD */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- �f�[�^�x�[�X�n���h���̍쐬�^�폜 -----*/
OKAO_API HALBUMDB	OKAO_CreateDatabase(enum DB_MODE mode, unsigned int nUser, unsigned int nPicture);
															/* �f�[�^�x�[�X�n���h���쐬 */
OKAO_API int		OKAO_DeleteDatabase(HALBUMDB hDB);		/* �f�[�^�x�[�X�n���h���폜 */

/*----- �f�[�^�x�[�X���� -----*/
OKAO_API int		OKAO_RegistDatabase(HALBUMDB hDB, HFACERECOG hFR, RAWIMAGE *pImage, int nWidth,
						int nHeight, int nDepth, FR_USER_ID aUid[], int nVectorNo);
															/* �f�[�^�x�[�X�Ƀf�[�^��o�^ */
OKAO_API int		OKAO_DeleteDBUserData(HALBUMDB hDB, FR_USER_ID aUid[], int nVectorNo);
															/* �f�[�^�x�[�X���炩��nUserNo/nVectorNo�Ŏw�肳����F�؃f�[�^���폜 */
/*----- �f�[�^�x�[�X�f�[�^�擾�^�ݒ� -----*/
OKAO_API int		OKAO_GetDatabaseSize(HALBUMDB hDB, int *pnSize);
															/* �f�[�^�x�[�X�f�[�^�T�C�Y�擾 */
OKAO_API int		OKAO_GetDatabaseData(HALBUMDB hDB, BYTE *pbyBuffer);
															/* �f�[�^�x�[�X�f�[�^�擾 */
OKAO_API int		OKAO_SetDatabaseData(HALBUMDB hDB, BYTE *pbyBuffer, int *pnUserNum, int *pnVectorNo);
															/* �f�[�^�x�[�X�f�[�^�ݒ� */

/*----- �环�� -----*/
OKAO_API int		OKAO_FrIdentify(HFACERECOG hFRGallary, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HALBUMDB hDB, int nThreshold, HFRRESULT hResult);
																	/*-�f�[�^�x�[�X�ƍ� */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAODBAPI_H__ */
