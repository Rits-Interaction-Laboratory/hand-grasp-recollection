/*--------------------------------------------------------------------*/
/*  Copyright(C) 2007-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	顔認証データベースライブラリＡＰＩ
*/
/*
	更新履歴：
	------------------------------------------------------------------------------
	年月日		更新内容				備考
	07-01-25	新規作成				SDK Ver4 対応
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
typedef	void *			HALBUMDB;		/* アルバムデータベースハンドル */
#endif	/* OKAO_BUILD */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- データベースハンドルの作成／削除 -----*/
OKAO_API HALBUMDB	OKAO_CreateDatabase(enum DB_MODE mode, unsigned int nUser, unsigned int nPicture);
															/* データベースハンドル作成 */
OKAO_API int		OKAO_DeleteDatabase(HALBUMDB hDB);		/* データベースハンドル削除 */

/*----- データベース操作 -----*/
OKAO_API int		OKAO_RegistDatabase(HALBUMDB hDB, HFACERECOG hFR, RAWIMAGE *pImage, int nWidth,
						int nHeight, int nDepth, FR_USER_ID aUid[], int nVectorNo);
															/* データベースにデータを登録 */
OKAO_API int		OKAO_DeleteDBUserData(HALBUMDB hDB, FR_USER_ID aUid[], int nVectorNo);
															/* データベースからからnUserNo/nVectorNoで指定される顔認証データを削除 */
/*----- データベースデータ取得／設定 -----*/
OKAO_API int		OKAO_GetDatabaseSize(HALBUMDB hDB, int *pnSize);
															/* データベースデータサイズ取得 */
OKAO_API int		OKAO_GetDatabaseData(HALBUMDB hDB, BYTE *pbyBuffer);
															/* データベースデータ取得 */
OKAO_API int		OKAO_SetDatabaseData(HALBUMDB hDB, BYTE *pbyBuffer, int *pnUserNum, int *pnVectorNo);
															/* データベースデータ設定 */

/*----- 顔識別 -----*/
OKAO_API int		OKAO_FrIdentify(HFACERECOG hFRGallary, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HALBUMDB hDB, int nThreshold, HFRRESULT hResult);
																	/*-データベース照合 */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAODBAPI_H__ */
