/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	顔検出ライブラリＡＰＩ
*/

#ifndef	OKAODTAPI_H__
#define	OKAODTAPI_H__

#include	"OkaoAPI.h"
#include	"ModeDT.h"

#ifdef	OKAO_BUILD
#include	"SdkDT.h"
#else
typedef	void *		HDETECTION;		/* 顔検出ハンドル */
typedef	void *		HDTRESULT;		/* 顔検出結果ハンドル */
#endif

/* ライブラリ指定 */
#define	OKAO_DETECTION	0x00000001UL		/* 顔検出 */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- 顔検出ハンドルの作成／削除 -----*/
OKAO_API HDETECTION OKAO_CreateDetection(enum DT_MODE mode);/* 顔検出ハンドル作成 */
OKAO_API int		OKAO_DeleteDetection(HDETECTION  hDT);	/* 顔検出ハンドル削除 */
OKAO_API HDTRESULT 	OKAO_CreateDtResult(void);				/* 顔検出結果ハンドル作成 */
OKAO_API int		OKAO_DeleteDtResult(HDTRESULT hResult);	/* 顔検出結果ハンドル削除 */

/*----- 顔検出 -----*/
OKAO_API int 		OKAO_Detection(HDETECTION hDT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HDTRESULT hResult);		/* 顔検出 */
/*----- 顔検出結果 -----*/
OKAO_API int		OKAO_GetDtFaceCount(HDTRESULT hResult, int *pnCount);
															/*-顔検出人数 */
OKAO_API int		OKAO_GetDtCorner(HDTRESULT hResult, int nIndex, POINT *pptLeftTop, POINT *pptRightTop,
						POINT *pptLeftBottom, POINT *pptRightBottom, int *pnConfidence);
															/*-顔の位置取得 */
OKAO_API int		OKAO_GetDtFacePose(HDTRESULT hResult, int nIndex, int *pnPose);
															/*-顔向きフラグ取得 */

/*----- 顔検出最大数設定／取得 -----*/
OKAO_API int		OKAO_SetDtMaxFaceNumber(HDETECTION hDT, int nMax);
															/*-顔検出最大数設定 */
OKAO_API int		OKAO_GetDtMaxFaceNumber(HDETECTION hDT, int *pnMax);
															/*-顔検出最大数取得 */
/*----- 顔検出サイズ設定／取得 -----*/
OKAO_API int		OKAO_SetDtMinMaxFaceSizeMode(HDETECTION hDT,
						enum DETECT_FACE_SIZE_MODE nMode);	/* 顔サイズ指定のモード設定 */
OKAO_API int		OKAO_GetDtMinMaxFaceSizeMode(HDETECTION hDT,
						enum DETECT_FACE_SIZE_MODE *pnMode);/* 顔サイズ指定のモード取得 */
OKAO_API int		OKAO_SetDtFaceSizeRange(HDETECTION hDT, int nMinSize, int nMaxSize);
															/*-サイズ設定（ピクセル） */
OKAO_API int		OKAO_GetDtFaceSizeRange(HDETECTION hDT, int *pnMinSize, int *pnMaxSize);
															/*-サイズ取得（ピクセル） */
OKAO_API int		OKAO_SetDtFaceSizeRatioRange(HDETECTION hDT, int nMinSizeRatio, int nMaxSizeRatio);
															/*-サイズ設定（％指定） */
OKAO_API int		OKAO_GetDtFaceSizeRatioRange(HDETECTION hDT, int *pnMinSizeRatio, int *pnMaxSizeRatio);
															/*-サイズ取得（％指定） */

/*----- 顔検索方向設定／取得 -----*/
OKAO_API int		OKAO_SetDtDetectDirection(HDETECTION hDT, int nDirection);
															/*-顔方向設定 */
OKAO_API int		OKAO_GetDtDetectDirection(HDETECTION hDT, int *pnDirection);
															/*-顔方向取得 */
/*----- 角度範囲設定／取得 -----*/
OKAO_API int		OKAO_SetDtDetectAngle(HDETECTION hDT, enum DETECT_ANGLE angle);
															/*-顔角度範囲設定 */
OKAO_API int		OKAO_GetDtDetectAngle(HDETECTION hDT, enum DETECT_ANGLE *pAngle);
															/*-顔角度範囲取得 */
/*----- 探索ステップ数設定／取得 -----*/
OKAO_API int		OKAO_SetDtStep(HDETECTION hDT, int nStep);
															/*-探索ステップ数設定 */
OKAO_API int		OKAO_GetDtStep(HDETECTION hDT, int *pnStep);
															/*-探索ステップ数取得 */
/*----- カラーマスクの設定／取得 -----*/
OKAO_API int		OKAO_SetDtColorMask(HDETECTION hDT, BOOL bFlag);
															/*-カラーマスクの設定 */
OKAO_API int		OKAO_GetDtColorMask(HDETECTION hDT, BOOL *pbFlag);
															/*-カラーマスクの取得 */
/*----- 周辺マスクの設定／取得 -----*/
OKAO_API int		OKAO_SetDtRectangleMask(HDETECTION hDT, RECT rcArea);
															/*-周辺マスクの設定 */
OKAO_API int		OKAO_GetDtRectangleMask(HDETECTION hDT, RECT *prcArea);
															/*-周辺マスクの取得 */
/*----- しきい値設定／取得 -----*/
OKAO_API int		OKAO_SetDtThreshold(HDETECTION hDT, int nThreshold);
															/*-しきい値設定 */
OKAO_API int		OKAO_GetDtThreshold(HDETECTION hDT, int *pnThreshold);
															/*-しきい値取得 */
/*----- 斜め横向き範囲の設定／取得 -----*/
OKAO_API int		OKAO_SetDtPose(HDETECTION hDT, int nPose);
															/*-斜め横向き範囲設定 */
OKAO_API int		OKAO_GetDtPose(HDETECTION hDT, int *pnPose);
															/*-斜め横向き範囲取得 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAODTAPI_H__ */
