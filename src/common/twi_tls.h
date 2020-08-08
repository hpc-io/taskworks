/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Cross-platform read write lock wrapper */

#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32
typedef DWORD TWI_Tls_t;
#else
typedef pthread_key_t TWI_Tls_t;
#endif
typedef TWI_Tls_t *TWI_Tls_handle_t;

terr_t TWI_Tls_init (TWI_Tls_handle_t m);
void TWI_Tls_finalize (TWI_Tls_t m);
void *TWI_Tls_get (TWI_Tls_t m);
void TWI_Tls_store (TWI_Tls_t m, void *data);