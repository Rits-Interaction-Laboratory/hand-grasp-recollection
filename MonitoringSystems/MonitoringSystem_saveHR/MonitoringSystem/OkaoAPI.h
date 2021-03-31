/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	OKAO_SDK ライブラリＡＰＩ
*/
#ifndef	OKAOAPI_H__
#define	OKAOAPI_H__

#ifdef	OKAO_DLL_EXPORTS
/* エクスポート（ＤＬＬビルド） */
#	define	OKAO_API		__declspec( dllexport ) 
#else
#ifdef	OKAO_LIB
/* スタティックリンク */
#	define	OKAO_API
#else
/* インポート（アプリケーション デフォルト）  */
#	define	OKAO_API		__declspec( dllimport ) 
#endif
#endif

#ifdef	WIN32
#	pragma warning( disable : 4115 )
#	include	<windows.h>
#	pragma warning( default : 4115 )
#else	/* WIN32 */
#	include	"OkaoDef.h"
#endif	/* WIN32 */

/* ライブラリ指定 */
#define	OKAO_COMMON		0x80000000UL		/* 共通関数 */
#define	OKAO_ALL		0xffffffffUL		/* 一括指定 */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- バージョン取得 -----*/
OKAO_API int		OKAO_GetVersion(BYTE *pbyMajor, BYTE *pbyMinor);
OKAO_API int		OKAO_GetDetailVersion(DWORD dwMode, BYTE *pVersionString);

/*----- ライブラリ初期化／終了 -----*/
OKAO_API int		OKAO_Initialize(DWORD dwMode);			/* ライブラリ初期化 */
OKAO_API int		OKAO_Terminate(DWORD dwMode);			/* ライブラリ終了 */

/*----- 結果ハンドル内容の保存・読み込み -----*/
OKAO_API int		OKAO_GetResultSize(void *hResult, int *pnSize);	/* 結果ハンドル内容のサイズ取得 */
OKAO_API int		OKAO_GetResultData(void *hResult, BYTE *pbyBuffer);	/* 結果ハンドル内容の保存 */
OKAO_API int		OKAO_SetResultData(void *hResult, BYTE *pbyBuffer);	/* 結果ハンドル内容の読み込み */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOAPI_H__ */
