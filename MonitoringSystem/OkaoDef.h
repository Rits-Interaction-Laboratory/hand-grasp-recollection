/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2006 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/

/*
	Windows環境テスト用（windows.h 代替ヘッダ）
*/

/*
	更新履歴：
	------------------------------------------------------------------------------
	年月日		更新内容				備考
	------------------------------------------------------------------------------
*/
#ifndef	OKAODEF_H__
#define	OKAODEF_H__

typedef		signed char			INT8;
typedef		unsigned char		UINT8;
typedef		signed short		INT16;
typedef		unsigned short		UINT16;
typedef		signed int			INT32;
typedef		unsigned int		UINT32;

typedef		float		FLOAT32;
typedef		double		FLOAT64;

typedef		unsigned char		BYTE;
typedef		unsigned short		WORD;
typedef		unsigned long		DWORD;

typedef		int					BOOL;
#ifndef 	FALSE
#define 	FALSE				0
#endif
#ifndef 	TRUE
#define 	TRUE				1
#endif

//typedef	struct tagPOINT {
//	INT32	x;
//	INT32	y;
//} POINT;
//
//typedef struct tagSIZE {
//	INT32	cx;
//	INT32	cy;
//} SIZE;
//
//typedef struct tagRECT {
//	INT32	left;
//	INT32	top;
//	INT32	right;
//	INT32	bottom;
//} RECT;

#endif	/* OKAODEF_H__ */
