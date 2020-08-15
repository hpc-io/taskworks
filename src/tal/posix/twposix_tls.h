/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Pthread thread driver rwlock */

#include "twposix.h"

#define TWPOSIX_Tls_cb \
	{ TWPOSIX_Tls_create, TWPOSIX_Tls_free, TWPOSIX_Tls_store, TWPOSIX_Tls_load }

terr_t TWPOSIX_Tls_create (TW_handle_t *k);
void TWPOSIX_Tls_free (TW_handle_t k);
void *TWPOSIX_Tls_load (TW_handle_t k);
void TWPOSIX_Tls_store (TW_handle_t k, void *data);
