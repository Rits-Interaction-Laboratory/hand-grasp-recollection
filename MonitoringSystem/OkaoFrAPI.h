/*--------------------------------------------------------------------*/
/*  Copyright(C) 2007-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	顔認証ライブラリＡＰＩ
*/

#ifndef	OKAOFRAPI_H__
#define	OKAOFRAPI_H__

#include	"OkaoAPI.h"
#include	"ModeFR.h"

#ifdef	OKAO_BUILD
#include	"SdkFR.h"
#else
typedef	void *			HFACERECOG;		/* 顔認証データハンドル */
typedef	void *			HPTRESULT;		/* 顔器官検出結果ハンドル */
typedef	void *			HFRRESULT;		/* 顔識別結果格納ハンドル */
#endif	/* OKAO_BUILD */

/* ライブラリ指定 */
#define	OKAO_RECOGNITION	0x00000004UL		/* 顔認証 */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- 顔認証データハンドル作成／削除 -----*/
OKAO_API HFACERECOG	OKAO_CreateFaceRecognition(enum FR_MODE mode);	/* 顔認証データハンドル作成 */
OKAO_API int		OKAO_DeleteFaceRecognition(HFACERECOG hFR);		/* 顔認証データハンドル削除 */

/*----- 顔識別結果格納ハンドル作成／削除 -----*/
OKAO_API HFRRESULT	OKAO_CreateIdentifyResult(unsigned int nMax);	/* 顔識別結果格納ハンドル作成 */
OKAO_API int		OKAO_DeleteIdentifyResult(HFRRESULT hResult);	/* 顔識別結果格納ハンドル削除 */

/*----- 顔認証データの設定 -----*/
OKAO_API int		OKAO_SetFrDataFromPtResult(HFACERECOG hFR, RAWIMAGE *pImage, int nWidth, int nHeight,
												int nDepth, HPTRESULT hResult);
																	/*-顔器官検出結果格納ハンドルから顔認証データを設定 */
/*----- 顔識別結果 -----*/
OKAO_API int		OKAO_GetFrResult(HFRRESULT hResult, FR_USER_ID *pUid, int anConf[], int *pnCount);
																	/*-顔識別結果取得 */
/*----- 顔識別結果統合 -----*/
OKAO_API int		OKAO_FrIntegrate(HFRRESULT hInResult[], int nResultNum, HFRRESULT hOutResult, int *pnOutIndex);
																	/*-顔識別統合結果取得 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOFRAPI_H__ */
