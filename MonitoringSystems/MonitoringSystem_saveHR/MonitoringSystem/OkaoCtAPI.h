/*--------------------------------------------------------------------*/
/*  Copyright(C) 2004-2006 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	ΦsoCu`oh
*/
/*
	XVπF
	------------------------------------------------------------------------------
	Nϊ		XVΰe				υl
	06-12-28	VASYΞ		StartStage/EndStageΜέθ^ζΎΦVέ
	------------------------------------------------------------------------------
*/

#ifndef	OKAOCTAPI_H__
#define	OKAOCTAPI_H__

#include	"OkaoAPI.h"
#include	"ModeCT.h"

#ifdef	OKAO_BUILD
#include	"SdkCT.h"
#else
typedef	void *			HCONTOUR;		/* Φsonh */
typedef	void *			HCTRESULT;		/* ΦsoΚnh */
typedef	void *			HPTRESULT;		/* ην―oΚnh */
#endif	/* OKAO_BUILD */

/* Cuwθ */
#define	OKAO_CONTOUR	0x00000800UL		/* ην―Φso */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- ΦsonhΜμ¬^ν -----*/
OKAO_API HCONTOUR	OKAO_CreateContour(enum CT_MODE mode);	/* Φsonhμ¬ */
OKAO_API int		OKAO_DeleteContour(HCONTOUR hCT);		/* Φsonhν */
OKAO_API HCTRESULT	OKAO_CreateCtResult(void);				/* ΦsoΚnhμ¬ */
OKAO_API int		OKAO_DeleteCtResult(HCTRESULT hResult);	/* ΦsoΚnhν */

/*----- ην―έθ -----*/
OKAO_API int 		OKAO_SetCtPosition(HCONTOUR hCT, HPTRESULT hResult);
															/*-ην―oΚnh©ηην―πέθ */
/*----- Φso -----*/
OKAO_API int		OKAO_Contour(HCONTOUR hCT, RAWIMAGE *pImage, int nWidth, int nHeight,
						int nDepth, HCTRESULT hResult);
/*----- ΦsoΚ -----*/
OKAO_API int		OKAO_GetCtPointNumber(HCTRESULT hResult, int *pnCount);
															/*-ΦsoΚΜζΎ */
OKAO_API int		OKAO_GetCtPoint(HCTRESULT hResult, POINT aptContour[]);
															/*-ΦsoΚΜζΎ */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOCTAPI_H__ */
