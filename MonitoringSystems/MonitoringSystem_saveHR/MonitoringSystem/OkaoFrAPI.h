/*--------------------------------------------------------------------*/
/*  Copyright(C) 2007-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	��F�؃��C�u�����`�o�h
*/

#ifndef	OKAOFRAPI_H__
#define	OKAOFRAPI_H__

#include	"OkaoAPI.h"
#include	"ModeFR.h"

#ifdef	OKAO_BUILD
#include	"SdkFR.h"
#else
typedef	void *			HFACERECOG;		/* ��F�؃f�[�^�n���h�� */
typedef	void *			HPTRESULT;		/* ��튯���o���ʃn���h�� */
typedef	void *			HFRRESULT;		/* �环�ʌ��ʊi�[�n���h�� */
#endif	/* OKAO_BUILD */

/* ���C�u�����w�� */
#define	OKAO_RECOGNITION	0x00000004UL		/* ��F�� */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- ��F�؃f�[�^�n���h���쐬�^�폜 -----*/
OKAO_API HFACERECOG	OKAO_CreateFaceRecognition(enum FR_MODE mode);	/* ��F�؃f�[�^�n���h���쐬 */
OKAO_API int		OKAO_DeleteFaceRecognition(HFACERECOG hFR);		/* ��F�؃f�[�^�n���h���폜 */

/*----- �环�ʌ��ʊi�[�n���h���쐬�^�폜 -----*/
OKAO_API HFRRESULT	OKAO_CreateIdentifyResult(unsigned int nMax);	/* �环�ʌ��ʊi�[�n���h���쐬 */
OKAO_API int		OKAO_DeleteIdentifyResult(HFRRESULT hResult);	/* �环�ʌ��ʊi�[�n���h���폜 */

/*----- ��F�؃f�[�^�̐ݒ� -----*/
OKAO_API int		OKAO_SetFrDataFromPtResult(HFACERECOG hFR, RAWIMAGE *pImage, int nWidth, int nHeight,
												int nDepth, HPTRESULT hResult);
																	/*-��튯���o���ʊi�[�n���h�������F�؃f�[�^��ݒ� */
/*----- �环�ʌ��� -----*/
OKAO_API int		OKAO_GetFrResult(HFRRESULT hResult, FR_USER_ID *pUid, int anConf[], int *pnCount);
																	/*-�环�ʌ��ʎ擾 */
/*----- �环�ʌ��ʓ��� -----*/
OKAO_API int		OKAO_FrIntegrate(HFRRESULT hInResult[], int nResultNum, HFRRESULT hOutResult, int *pnOutIndex);
																	/*-�环�ʓ������ʎ擾 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOFRAPI_H__ */
