/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	��튯���o���C�u�����`�o�h
*/

#ifndef	OKAOPTAPI_H__
#define	OKAOPTAPI_H__

#include	"OkaoAPI.h"
#include	"ModePT.h"

#ifdef	OKAO_BUILD
#include	"SdkPT.h"
#else
typedef	void *			HPOINTER;		/* ��튯���o�n���h�� */
typedef	void *			HDTRESULT;		/* �猟�o���ʃn���h�� */
typedef	void *			HPTRESULT;		/* ��튯���o���ʃn���h�� */
#endif	/* OKAO_BUILD */

/* ���C�u�����w�� */
#define	OKAO_POINTER	0x00000002UL		/* ��튯���o */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- ��튯���o�n���h���̍쐬�^�폜 -----*/
OKAO_API HPOINTER	OKAO_CreatePointer(enum PT_MODE mode);	/* ��튯���o�n���h���쐬 */
OKAO_API int		OKAO_DeletePointer(HPOINTER hPT);		/* ��튯���o�n���h���폜 */
OKAO_API HPTRESULT	OKAO_CreatePtResult(void);				/* ��튯���o���ʃn���h���쐬 */
OKAO_API int		OKAO_DeletePtResult(HPTRESULT hResult);	/* ��튯���o���ʃn���h���폜 */

/*----- ��ʒu�ݒ� -----*/
OKAO_API int		OKAO_SetPtPosition(HPOINTER hPT, HDTRESULT hResult, int nIndex);
															/*-n�Ԗڂ̊�ʒu���猟�o����ݒ� */
/*----- ��튯���o -----*/
OKAO_API int		OKAO_Pointer(HPOINTER hPT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HPTRESULT hResult, int *pnConfidence);
															/*-��튯���o */
OKAO_API int		OKAO_PointerAndGaze(HPOINTER hPT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HPTRESULT hResult, int *pnConfidence);	
															/*-�������� */
/*----- ��튯���o���� -----*/
OKAO_API int		OKAO_GetPtPointNumber(HPTRESULT hResult, int *pnCount);
															/*-��튯���o���ʐ��̎擾 */
OKAO_API int		OKAO_GetPtPoint(HPTRESULT hResult, POINT aptPoint[], int aConf[]);
															/*-��튯���o���ʂ̎擾 */
OKAO_API int		OKAO_GetPtGazePoint(HPTRESULT hResult,
						int *pnGazeLR, int *pnGazeUD, int *pnConfidenceGaze);
															/*-�������茋�ʂ̎擾 */
OKAO_API int		OKAO_GetPtOpenLevel(HPTRESULT hResult,
						int *pnLeftEyeOpenLevel, int *pnRightEyeOpenLevel, int *pnMouthOpenLevel,
						int *pnLeftEyeOpenLevelConfidence, int *pnRightEyeOpenLevelConfidence, int *pnMouthOpenLevelConfidence);
															/*-��튯�J���ʂ̎擾 */
OKAO_API int		OKAO_GetPtDetailPointNumber(HPTRESULT hResult, int *pnCount);
															/*-��튯�J���o�p�����_���̎擾 */
OKAO_API int		OKAO_GetPtDetailPoint(HPTRESULT hResult,
						POINT aptFeatureDetail[], int anConfidenceDetail[]);
															/*-��튯�J���o�p�����_�̎擾 */
OKAO_API int		OKAO_GetPtDirection(HPTRESULT hResult, int *pnUpDown, int *pnLeftRight,
						int *pnRoll, int *pnConfidence);	/* ��������ʂ̎擾 */


#ifdef  __cplusplus
}
#endif

#endif	/* OKAOPTAPI_H__ */
