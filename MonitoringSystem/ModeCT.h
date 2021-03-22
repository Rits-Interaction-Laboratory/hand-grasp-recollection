/*--------------------------------------------------------------------*/
/*  Copyright(C) 2004-2006 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/

#ifndef	MODECT_H__
#define	MODECT_H__

#include	"CommonDef.h"

enum CT_MODE {
	CT_MODE_DEFAULT = 0,		/* デフォルト(標準) */
	CT_MODE_FAST				/* 高速(動画用) */
};

/*----- 顔器官輪郭抽出点インデックス -----*/
enum CONTOUR_POINT {
	CONTOUR_EYEL_1 = 0,			/* 左目 第１点 */
	CONTOUR_EYEL_2,				/* 左目 第２点 */
	CONTOUR_EYEL_3,				/* 左目 第３点 */
	CONTOUR_EYEL_4,				/* 左目 第４点 */
	CONTOUR_EYEL_5,				/* 左目 第５点 */
	CONTOUR_EYEL_6,				/* 左目 第６点 */
	CONTOUR_EYEL_7,				/* 左目 第７点 */
	CONTOUR_EYEL_8,				/* 左目 第８点 */
	CONTOUR_EYEBROWL_1,			/* 左眉 第１点 */
	CONTOUR_EYEBROWL_2,			/* 左眉 第２点 */
	CONTOUR_EYEBROWL_3,			/* 左眉 第３点 */
	CONTOUR_EYEBROWL_4,			/* 左眉 第４点 */
	CONTOUR_EYEBROWL_5,			/* 左眉 第５点 */
	CONTOUR_EYEBROWL_6,			/* 左眉 第６点 */
	CONTOUR_EYEBROWL_7,			/* 左眉 第７点 */
	CONTOUR_EYEBROWL_8,			/* 左眉 第８点 */
	CONTOUR_EYER_1,				/* 右目 第１点 */
	CONTOUR_EYER_2,				/* 右目 第２点 */
	CONTOUR_EYER_3,				/* 右目 第３点 */
	CONTOUR_EYER_4,				/* 右目 第４点 */
	CONTOUR_EYER_5,				/* 右目 第５点 */
	CONTOUR_EYER_6,				/* 右目 第６点 */
	CONTOUR_EYER_7,				/* 右目 第７点 */
	CONTOUR_EYER_8,				/* 右目 第８点 */
	CONTOUR_EYEBROWR_1,			/* 右眉 第１点 */
	CONTOUR_EYEBROWR_2,			/* 右眉 第２点 */
	CONTOUR_EYEBROWR_3,			/* 右眉 第３点 */
	CONTOUR_EYEBROWR_4,			/* 右眉 第４点 */
	CONTOUR_EYEBROWR_5,			/* 右眉 第５点 */
	CONTOUR_EYEBROWR_6,			/* 右眉 第６点 */
	CONTOUR_EYEBROWR_7,			/* 右眉 第７点 */
	CONTOUR_EYEBROWR_8,			/* 右眉 第８点 */
	CONTOUR_NOSE_1,				/* 鼻 第１点 */
	CONTOUR_NOSE_2,				/* 鼻 第２点 */
	CONTOUR_NOSE_3,				/* 鼻 第３点 */
	CONTOUR_NOSE_4,				/* 鼻 第４点 */
	CONTOUR_NOSE_5,				/* 鼻 第５点 */
	CONTOUR_NOSE_6,				/* 鼻 第６点 */
	CONTOUR_NOSE_7,				/* 鼻 第７点 */
	CONTOUR_NOSE_8,				/* 鼻 第８点 */
	CONTOUR_NOSE_9,				/* 鼻 第９点 */	
	CONTOUR_NOSE_10,			/* 鼻 第１０点 */
	CONTOUR_NOSE_11,			/* 鼻 第１１点 */
	CONTOUR_NOSE_12,			/* 鼻 第１２点 */
	CONTOUR_MOUTH_1,			/* 口 第１点 */
	CONTOUR_MOUTH_2,			/* 口 第２点 */
	CONTOUR_MOUTH_3,			/* 口 第３点 */
	CONTOUR_MOUTH_4,			/* 口 第４点 */
	CONTOUR_MOUTH_5,			/* 口 第５点 */
	CONTOUR_MOUTH_6,			/* 口 第６点 */
	CONTOUR_MOUTH_7,			/* 口 第７点 */
	CONTOUR_MOUTH_8,			/* 口 第８点 */
	CONTOUR_MOUTH_9,			/* 口 第９点 */	
	CONTOUR_MOUTH_10,			/* 口 第１０点 */
	CONTOUR_MOUTH_11,			/* 口 第１１点 */
	CONTOUR_MOUTH_12,			/* 口 第１２点 */
	CONTOUR_MOUTH_13,			/* 口 第１３点 */
	CONTOUR_MOUTH_14,			/* 口 第１４点 */
	CONTOUR_MOUTH_15,			/* 口 第１５点 */
	CONTOUR_MOUTH_16,			/* 口 第１６点 */
	CONTOUR_MOUTH_17,			/* 口 第１７点 */
	CONTOUR_MOUTH_18,			/* 口 第１８点 */
	CONTOUR_MOUTH_19,			/* 口 第１９点 */
	CONTOUR_MOUTH_20,			/* 口 第２０点 */
	CONTOUR_MOUTH_21,			/* 口 第２１点 */
	CONTOUR_MOUTH_22,			/* 口 第２２点 */
	CONTOUR_FACE_1,				/* 顔輪郭 第１点 */
	CONTOUR_FACE_2,				/* 顔輪郭 第２点 */
	CONTOUR_FACE_3,				/* 顔輪郭 第３点 */
	CONTOUR_FACE_4,				/* 顔輪郭 第４点 */
	CONTOUR_FACE_5,				/* 顔輪郭 第５点 */
	CONTOUR_FACE_6,				/* 顔輪郭 第６点 */
	CONTOUR_FACE_7,				/* 顔輪郭 第７点 */
	CONTOUR_FACE_8,				/* 顔輪郭 第８点 */
	CONTOUR_FACE_9,				/* 顔輪郭 第９点 */	
	CONTOUR_FACE_10,			/* 顔輪郭 第１０点 */
	CONTOUR_FACE_11,			/* 顔輪郭 第１１点 */
	CONTOUR_FACE_12,			/* 顔輪郭 第１２点 */
	CONTOUR_FACE_13,			/* 顔輪郭 第１３点 */
	CONTOUR_FACE_14,			/* 顔輪郭 第１４点 */
	CONTOUR_FACE_15,			/* 顔輪郭 第１５点 */
	CONTOUR_FACE_16,			/* 顔輪郭 第１６点 */
	CONTOUR_FACE_17,			/* 顔輪郭 第１７点 */
	CONTOUR_FACE_18,			/* 顔輪郭 第１８点 */
	CONTOUR_FACE_19,			/* 顔輪郭 第１９点 */
	CONTOUR_FACE_20,			/* 顔輪郭 第２０点 */
	CONTOUR_FACE_21,			/* 顔輪郭 第２１点 */
	CONTOUR_KIND_MAX			/* 抽出点の数 */
};

/*----- 顔器官輪郭の無効座標値 -----*/
#define	CONTOUR_NO_POINT	-1		/* x,y 両方に設定 */

#endif	/* MODECT_H__ */
