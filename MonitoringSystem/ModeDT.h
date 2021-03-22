/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/

#ifndef	MODEDT_H__
#define	MODEDT_H__

#include	"CommonDef.h"

/* 顔検出モード */
enum DT_MODE {
	DT_MODE_DEFAULT = 0				/* デフォルト */
};

/* 顔画像方向 */
#define	DETECT_UP		0x01		/* 上向き顔画像 */
#define	DETECT_RIGHT	0x02		/* 右向き顔画像 */
#define	DETECT_LEFT		0x04		/* 左向き顔画像 */
#define	DETECT_DOWN		0x08		/* 下向き顔画像 */

/* 斜め横向き範囲 */
#define	DETECT_FRONT		0x01	/* 正面顔 */
#define	DETECT_HALF_PROFILE	0x03	/* 斜め横顔 */

/* 顔画像角度範囲 */
enum DETECT_ANGLE {
	DETECT_1ANGLE = 0,				/* １角度分（0°） */
	DETECT_3ANGLE					/* ３角度分（0°、-30°、+30°） */
};

/* 顔検出サイズモード */
enum DETECT_FACE_SIZE_MODE {
	DETECT_FACE_PIXEL = 0,			/* ピクセル指定 */
	DETECT_FACE_RATIO				/* 割合(%)指定 */
};

#endif	/* MODEDT_H__ */
