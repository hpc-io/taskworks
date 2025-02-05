/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Error codes used by */

#pragma once

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

#define TW_N_ERR_CODE 0x400

#ifdef TWI_DEBUG
#define DEBUG_ABORT                      \
	{                                    \
		char *env;                       \
		env = getenv ("TW_DEBUG_ABORT"); \
		if (env) { abort (); }           \
	}
#define PRINT_ERR_MSG(E, M) \
	{ printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, M, E); }
#ifdef _WIN32
#define PRINT_OS_ERR(R)                                                              \
	{                                                                                \
		LPVOID win32msg;                                                             \
		DWORD win32err;                                                              \
		\                                                                            \
                                                                                     \
			win32err = (DWORD)R;                                                     \
                                                                                     \
		FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | \
						   FORMAT_MESSAGE_IGNORE_INSERTS,                            \
					   NULL, win32err, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),   \
					   (LPTSTR)&win32msg, 0, NULL);                                  \
		printf ("Win32 error: %s", win32msg);                                        \
	}
#else
#define PRINT_OS_ERR(R) \
	{ printf ("POSIX error: %s", strerror (R)); }
#endif
#else
#define DEBUG_ABORT \
	{}
#define PRINT_ERR_MSG(E, M)
#define PRINT_OS_ERR(R)
#endif
#define PRINT_ERR(E) PRINT_ERR_MSG (E, TW_Get_err_msg (E))

#define CHECK_RET(R)           \
	{                          \
		if (R != TW_SUCCESS) { \
			PRINT_ERR (R)      \
			DEBUG_ABORT        \
			goto err_out;      \
		}                      \
	}
#define CHECK_RETR(R)          \
	{                          \
		if (R != TW_SUCCESS) { \
			PRINT_ERR (R)      \
			DEBUG_ABORT        \
			return err_out;    \
		}                      \
	}
#define CHECK_ERR CHECK_RET (err)

#define ASSIGN_ERR(R) \
	{                 \
		err = R;      \
		PRINT_ERR (R) \
		DEBUG_ABORT   \
		goto err_out; \
	}
#define RET_ERR(R)    \
	{                 \
		PRINT_ERR (R) \
		DEBUG_ABORT   \
		return R;     \
	}

#define CHECK_PTR(P)          \
	{                         \
		if (P == NULL) {      \
			err = TW_ERR_MEM; \
			PRINT_ERR (err)   \
			DEBUG_ABORT       \
			goto err_out;     \
		}                     \
	}

#define RET_OS_ERR(R)    \
	{                    \
		err = TW_ERR_OS; \
		PRINT_OS_ERR (R) \
		DEBUG_ABORT      \
		goto err_out;    \
	}

#ifdef HAVE_MPI
#define CHECK_MPIRET(R)         \
	{                           \
		if (R != MPI_SUCCESS) { \
			PRINT_ERR (R)       \
			DEBUG_ABORT         \
			goto err_out;       \
		}                       \
	}
#define CHECK_MPIERR CHECK_MPIRET (mpierr)
#endif