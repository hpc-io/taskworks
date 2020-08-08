/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,
 *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Mutex wrapper for win32 and posix */

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "taskworks_internal.h"
#include "twi_tls.h"

#ifdef _WIN32
terr_t TWI_Tls_init (TWI_Tls_handle_t m) {
	*m = TlsAlloc ();
	if (*m == TLS_OUT_OF_INDEXES) RET_ERR (TW_ERR_MEM);
	return TW_SUCCESS;
}

void TWI_Tls_finalize (TWI_Tls_t m) { TlsFree (m); }

void *TWI_Tls_get (TWI_Tls_t m) { return TlsGetValue (m); }

void TWI_Tls_store (TWI_Tls_t m, void *data) { TlsSetValue (m, data); }
#else
terr_t TWI_Tls_init (TWI_Tls_handle_t m) {
	if (pthread_key_create (m, NULL) != 0) RET_ERR (TW_ERR_MEM)
	return TW_SUCCESS;
}

void TWI_Tls_finalize (TWI_Tls_t m) { pthread_key_delete (m); }

void *TWI_Tls_get (TWI_Tls_t m) { return pthread_getspecific (m); }

void TWI_Tls_store (TWI_Tls_t m, void *data) { pthread_setspecific (m, data); }
#endif