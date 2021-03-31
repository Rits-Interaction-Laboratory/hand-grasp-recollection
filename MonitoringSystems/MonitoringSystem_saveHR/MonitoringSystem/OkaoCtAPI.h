/*--------------------------------------------------------------------*/
/*  Copyright(C) 2004-2006 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	輪郭抽出ライブラリＡＰＩ
*/
/*
	更新履歴：
	------------------------------------------------------------------------------
	年月日		更新内容				備考
	06-12-28	新アルゴリズム対応		StartStage/EndStageの設定／取得関数新設
	------------------------------------------------------------------------------
*/

#ifndef	OKAOCTAPI_H__
#define	OKAOCTAPI_H__

#include	"OkaoAPI.h"
#include	"ModeCT.h"

#ifdef	OKAO_BUILD
#include	"SdkCT.h"
#else
typedef	void *			HCONTOUR;		/* 輪郭抽出ハンドル */
typedef	void *			HCTRESULT;		/* 輪郭抽出結果ハンドル */
typedef	void *			HPTRESULT;		/* 顔器官検出結果ハンドル */
#endif	/* OKAO_BUILD */

/* ライブラリ指定 */
#define	OKAO_CONTOUR	0x00000800UL		/* 顔器官輪郭抽出 */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- 輪郭抽出ハンドルの作成／削除 -----*/
OKAO_API HCONTOUR	OKAO_CreateContour(enum CT_MODE mode);	/* 輪郭抽出ハンドル作成 */
OKAO_API int		OKAO_DeleteContour(HCONTOUR hCT);		/* 輪郭抽出ハンドル削除 */
OKAO_API HCTRESULT	OKAO_CreateCtResult(void);				/* 輪郭抽出結果ハンドル作成 */
OKAO_API int		OKAO_DeleteCtResult(HCTRESULT hResult);	/* 輪郭抽出結果ハンドル削除 */

/*----- 顔器官設定 -----*/
OKAO_API int 		OKAO_SetCtPosition(HCONTOUR hCT, HPTRESULT hResult);
															/*-顔器官検出結果ハンドルから顔器官を設定 */
/*----- 輪郭抽出 -----*/
OKAO_API int		OKAO_Contour(HCONTOUR hCT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HCTRESULT hResult);
/*----- 輪郭検出結果 -----*/
OKAO_API int		OKAO_GetCtPointNumber(HCTRESULT hResult, int *pnCount);
															/*-輪郭検出結果数の取得 */
OKAO_API int		OKAO_GetCtPoint(HCTRESULT hResult, POINT aptContour[]);
															/*-輪郭検出結果の取得 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOCTAPI_H__ */
