/*--------------------------------------------------------------------*/
/*  Copyright(C) 2003-2008 by OMRON Corporation                       */
/*  All Rights Reserved.                                              */
/*--------------------------------------------------------------------*/
/* 
	OKAO_SDK ���C�u�����`�o�h
*/
#ifndef	OKAOAPI_H__
#define	OKAOAPI_H__

#ifdef	OKAO_DLL_EXPORTS
/* �G�N�X�|�[�g�i�c�k�k�r���h�j */
#	define	OKAO_API		__declspec( dllexport ) 
#else
#ifdef	OKAO_LIB
/* �X�^�e�B�b�N�����N */
#	define	OKAO_API
#else
/* �C���|�[�g�i�A�v���P�[�V���� �f�t�H���g�j  */
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

/* ���C�u�����w�� */
#define	OKAO_COMMON		0x80000000UL		/* ���ʊ֐� */
#define	OKAO_ALL		0xffffffffUL		/* �ꊇ�w�� */

#ifdef  __cplusplus
extern "C" {
#endif

/*----- �o�[�W�����擾 -----*/
OKAO_API int		OKAO_GetVersion(BYTE *pbyMajor, BYTE *pbyMinor);
OKAO_API int		OKAO_GetDetailVersion(DWORD dwMode, BYTE *pVersionString);

/*----- ���C�u�����������^�I�� -----*/
OKAO_API int		OKAO_Initialize(DWORD dwMode);			/* ���C�u���������� */
OKAO_API int		OKAO_Terminate(DWORD dwMode);			/* ���C�u�����I�� */

/*----- ���ʃn���h�����e�̕ۑ��E�ǂݍ��� -----*/
OKAO_API int		OKAO_GetResultSize(void *hResult, int *pnSize);	/* ���ʃn���h�����e�̃T�C�Y�擾 */
OKAO_API int		OKAO_GetResultData(void *hResult, BYTE *pbyBuffer);	/* ���ʃn���h�����e�̕ۑ� */
OKAO_API int		OKAO_SetResultData(void *hResult, BYTE *pbyBuffer);	/* ���ʃn���h�����e�̓ǂݍ��� */

#ifdef  __cplusplus
}
#endif

#endif	/* OKAOAPI_H__ */
