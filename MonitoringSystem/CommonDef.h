/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/*
	更新履歴：
	------------------------------------------------------------------------------
	年月日		更新内容				備考
	03-09-02	enum FEATURE_POINT		顔器官検出の機能追加のため
	03-10-06	FEATURE_NO_POINT 追加	POINT[FEATURE_***] の無効座標を定義
	04-02-12	OKAO_TIMEOUT 追加		結果コードに処理タイムアウトを定義
	06-10-11	エラーコード追加		結果コードに未対応，モード違い，予約，ハンドルエラーを定義
	07-11-20	DSP版対応				重複登録エラーを追加定義
	------------------------------------------------------------------------------
*/
#ifndef	COMMONDEF_H__
#define	COMMONDEF_H__

typedef unsigned char	RAWIMAGE;		/* ＲＡＷイメージデータ */

/*----- 顔器官インデックス -----*/
enum FEATURE_POINT {
	FEATURE_LEFT_EYE = 0,				/* 左目（中央） */
	FEATURE_RIGHT_EYE,					/* 右目（中央） */
	FEATURE_MOUTH,						/* 口（中央） */
	/* SDK Ver 2.0 以降 */
	FEATURE_LEFT_EYE_IN,				/* 左目 目頭 */
	FEATURE_LEFT_EYE_OUT,				/* 左目 目尻 */
	FEATURE_RIGHT_EYE_IN,				/* 右目 目頭 */
	FEATURE_RIGHT_EYE_OUT,				/* 右目 目尻 */
	FEATURE_MOUTH_LEFT,					/* 口元 左 */
	FEATURE_MOUTH_RIGHT,				/* 口元 右 */
	FEATURE_LEFT_EYE_PUPIL,				/* 左目 瞳 */
	FEATURE_RIGHT_EYE_PUPIL,			/* 右目 瞳 */
	FEATURE_KIND_MAX					/* インデックスの数 */
};

/*----- 顔器官の無効座標値 -----*/
#define	FEATURE_NO_POINT		-1			/* x,y 両方に設定 */


/*----- OKAO API 結果コード -----*/
#define		OKAO_NORMAL				0		/* 正常終了 */
#define		OKAO_TIMEOUT			1		/* 処理タイムアウト */
#define		OKAO_NOTIMPLEMENTED		2		/* 現状未対応の機能 */
#define		OKAO_ERR_VARIOUS		-1		/* 未定義エラー */
#define		OKAO_ERR_INITIALIZE		-2		/* 初期化エラー */
#define		OKAO_ERR_INVALIDPARAM	-3		/* 引数エラー */
#define		OKAO_ERR_ALLOCMEMORY	-4		/* メモリ確保エラー */
#define		OKAO_ERR_MODEDIFFERENT	-5		/* 指定モード違いエラー */
#define		OKAO_ERR_NOALLOC		-6		/* (組み込み用) */
#define		OKAO_ERR_NOHANDLE		-7		/* ハンドルエラー */
#define		OKAO_ERR_DUPLICATE		-8		/* 重複登録エラー */

#endif	/* COMMONDEF_H__ */

