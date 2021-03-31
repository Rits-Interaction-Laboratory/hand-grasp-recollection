/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	顔器官検出ライブラリＡＰＩ
*/

#ifndef	OKAOPTAPI_H__
#define	OKAOPTAPI_H__

#include	"OkaoAPI.h"
#include	"ModePT.h"

#ifdef	OKAO_BUILD
#include	"SdkPT.h"
#else
typedef	void *			HPOINTER;		/* 顔器官検出ハンドル */
typedef	void *			HDTRESULT;		/* 顔検出結果ハンドル */
typedef	void *			HPTRESULT;		/* 顔器官検出結果ハンドル */
#endif	/* OKAO_BUILD */

/* ライブラリ指定 */
#define	OKAO_POINTER	0x00000002UL		/* 顔器官検出 */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- 顔器官検出ハンドルの作成／削除 -----*/
OKAO_API HPOINTER	OKAO_CreatePointer(enum PT_MODE mode);	/* 顔器官検出ハンドル作成 */
OKAO_API int		OKAO_DeletePointer(HPOINTER hPT);		/* 顔器官検出ハンドル削除 */
OKAO_API HPTRESULT	OKAO_CreatePtResult(void);				/* 顔器官検出結果ハンドル作成 */
OKAO_API int		OKAO_DeletePtResult(HPTRESULT hResult);	/* 顔器官検出結果ハンドル削除 */

/*----- 顔位置設定 -----*/
OKAO_API int		OKAO_SetPtPosition(HPOINTER hPT, HDTRESULT hResult, int nIndex);
															/*-n番目の顔位置を顔検出から設定 */
/*----- 顔器官検出 -----*/
OKAO_API int		OKAO_Pointer(HPOINTER hPT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HPTRESULT hResult, int *pnConfidence);
															/*-顔器官検出 */
OKAO_API int		OKAO_PointerAndGaze(HPOINTER hPT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HPTRESULT hResult, int *pnConfidence);	
															/*-視線推定 */
/*----- 顔器官検出結果 -----*/
OKAO_API int		OKAO_GetPtPointNumber(HPTRESULT hResult, int *pnCount);
															/*-顔器官検出結果数の取得 */
OKAO_API int		OKAO_GetPtPoint(HPTRESULT hResult, POINT aptPoint[], int aConf[]);
															/*-顔器官検出結果の取得 */
OKAO_API int		OKAO_GetPtGazePoint(HPTRESULT hResult,
						int *pnGazeLR, int *pnGazeUD, int *pnConfidenceGaze);
															/*-視線推定結果の取得 */
OKAO_API int		OKAO_GetPtOpenLevel(HPTRESULT hResult,
						int *pnLeftEyeOpenLevel, int *pnRightEyeOpenLevel, int *pnMouthOpenLevel,
						int *pnLeftEyeOpenLevelConfidence, int *pnRightEyeOpenLevelConfidence, int *pnMouthOpenLevelConfidence);
															/*-顔器官開閉結果の取得 */
OKAO_API int		OKAO_GetPtDetailPointNumber(HPTRESULT hResult, int *pnCount);
															/*-顔器官開閉検出用特徴点数の取得 */
OKAO_API int		OKAO_GetPtDetailPoint(HPTRESULT hResult,
						POINT aptFeatureDetail[], int anConfidenceDetail[]);
															/*-顔器官開閉検出用特徴点の取得 */
OKAO_API int		OKAO_GetPtDirection(HPTRESULT hResult, int *pnUpDown, int *pnLeftRight,
						int *pnRoll, int *pnConfidence);	/* 顔向き結果の取得 */


#ifdef  __cplusplus
}
#endif

#endif	/* OKAOPTAPI_H__ */
