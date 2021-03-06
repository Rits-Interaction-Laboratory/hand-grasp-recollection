/*--------------------------------------------------------------------*/
/*  Copyright(C) 2007-2007 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/*
	更新履歴：
	------------------------------------------------------------------------------
	年月日		更新内容				備考
	07-01-25	新規作成				SDK Ver4 対応
	------------------------------------------------------------------------------
*/

#ifndef	MODEDB_H__
#define	MODEDB_H__

#include	"ModeFR.h"
#include	"CommonDef.h"

/* データベースモード */
enum DB_MODE {
	DB_MODE_DEFAULT = 0				/* デフォルト */
};

/*----- データベース内の指定個人識別ＩＤの全クリア指定 -----*/
#define	DB_DELALL			-1

#endif	/* MODEDB_H__ */
