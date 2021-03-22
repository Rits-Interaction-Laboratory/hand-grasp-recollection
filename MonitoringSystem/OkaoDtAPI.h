/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	�猟�o���C�u�����`�o�h
*/

#ifndef	OKAODTAPI_H__
#define	OKAODTAPI_H__

#include	"OkaoAPI.h"
#include	"ModeDT.h"

#ifdef	OKAO_BUILD
#include	"SdkDT.h"
#else
typedef	void *		HDETECTION;		/* �猟�o�n���h�� */
typedef	void *		HDTRESULT;		/* �猟�o���ʃn���h�� */
#endif

/* ���C�u�����w�� */
#define	OKAO_DETECTION	0x00000001UL		/* �猟�o */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- �猟�o�n���h���̍쐬�^�폜 -----*/
OKAO_API HDETECTION OKAO_CreateDetection(enum DT_MODE mode);/* �猟�o�n���h���쐬 */
OKAO_API int		OKAO_DeleteDetection(HDETECTION  hDT);	/* �猟�o�n���h���폜 */
OKAO_API HDTRESULT 	OKAO_CreateDtResult(void);				/* �猟�o���ʃn���h���쐬 */
OKAO_API int		OKAO_DeleteDtResult(HDTRESULT hResult);	/* �猟�o���ʃn���h���폜 */

/*----- �猟�o -----*/
OKAO_API int 		OKAO_Detection(HDETECTION hDT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HDTRESULT hResult);		/* �猟�o */
/*----- �猟�o���� -----*/
OKAO_API int		OKAO_GetDtFaceCount(HDTRESULT hResult, int *pnCount);
															/*-�猟�o�l�� */
OKAO_API int		OKAO_GetDtCorner(HDTRESULT hResult, int nIndex, POINT *pptLeftTop, POINT *pptRightTop,
						POINT *pptLeftBottom, POINT *pptRightBottom, int *pnConfidence);
															/*-��̈ʒu�擾 */
OKAO_API int		OKAO_GetDtFacePose(HDTRESULT hResult, int nIndex, int *pnPose);
															/*-������t���O�擾 */

/*----- �猟�o�ő吔�ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtMaxFaceNumber(HDETECTION hDT, int nMax);
															/*-�猟�o�ő吔�ݒ� */
OKAO_API int		OKAO_GetDtMaxFaceNumber(HDETECTION hDT, int *pnMax);
															/*-�猟�o�ő吔�擾 */
/*----- �猟�o�T�C�Y�ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtMinMaxFaceSizeMode(HDETECTION hDT,
						enum DETECT_FACE_SIZE_MODE nMode);	/* ��T�C�Y�w��̃��[�h�ݒ� */
OKAO_API int		OKAO_GetDtMinMaxFaceSizeMode(HDETECTION hDT,
						enum DETECT_FACE_SIZE_MODE *pnMode);/* ��T�C�Y�w��̃��[�h�擾 */
OKAO_API int		OKAO_SetDtFaceSizeRange(HDETECTION hDT, int nMinSize, int nMaxSize);
															/*-�T�C�Y�ݒ�i�s�N�Z���j */
OKAO_API int		OKAO_GetDtFaceSizeRange(HDETECTION hDT, int *pnMinSize, int *pnMaxSize);
															/*-�T�C�Y�擾�i�s�N�Z���j */
OKAO_API int		OKAO_SetDtFaceSizeRatioRange(HDETECTION hDT, int nMinSizeRatio, int nMaxSizeRatio);
															/*-�T�C�Y�ݒ�i���w��j */
OKAO_API int		OKAO_GetDtFaceSizeRatioRange(HDETECTION hDT, int *pnMinSizeRatio, int *pnMaxSizeRatio);
															/*-�T�C�Y�擾�i���w��j */

/*----- �猟�������ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtDetectDirection(HDETECTION hDT, int nDirection);
															/*-������ݒ� */
OKAO_API int		OKAO_GetDtDetectDirection(HDETECTION hDT, int *pnDirection);
															/*-������擾 */
/*----- �p�x�͈͐ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtDetectAngle(HDETECTION hDT, enum DETECT_ANGLE angle);
															/*-��p�x�͈͐ݒ� */
OKAO_API int		OKAO_GetDtDetectAngle(HDETECTION hDT, enum DETECT_ANGLE *pAngle);
															/*-��p�x�͈͎擾 */
/*----- �T���X�e�b�v���ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtStep(HDETECTION hDT, int nStep);
															/*-�T���X�e�b�v���ݒ� */
OKAO_API int		OKAO_GetDtStep(HDETECTION hDT, int *pnStep);
															/*-�T���X�e�b�v���擾 */
/*----- �J���[�}�X�N�̐ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtColorMask(HDETECTION hDT, BOOL bFlag);
															/*-�J���[�}�X�N�̐ݒ� */
OKAO_API int		OKAO_GetDtColorMask(HDETECTION hDT, BOOL *pbFlag);
															/*-�J���[�}�X�N�̎擾 */
/*----- ���Ӄ}�X�N�̐ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtRectangleMask(HDETECTION hDT, RECT rcArea);
															/*-���Ӄ}�X�N�̐ݒ� */
OKAO_API int		OKAO_GetDtRectangleMask(HDETECTION hDT, RECT *prcArea);
															/*-���Ӄ}�X�N�̎擾 */
/*----- �������l�ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtThreshold(HDETECTION hDT, int nThreshold);
															/*-�������l�ݒ� */
OKAO_API int		OKAO_GetDtThreshold(HDETECTION hDT, int *pnThreshold);
															/*-�������l�擾 */
/*----- �΂߉������͈͂̐ݒ�^�擾 -----*/
OKAO_API int		OKAO_SetDtPose(HDETECTION hDT, int nPose);
															/*-�΂߉������͈͐ݒ� */
OKAO_API int		OKAO_GetDtPose(HDETECTION hDT, int *pnPose);
															/*-�΂߉������͈͎擾 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAODTAPI_H__ */
