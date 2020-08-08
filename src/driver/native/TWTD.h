/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* TaskWroks thread driver */

#pragma once

#include "TWI.h"
#include "TWprivate.h"

typedef struct TWTD_driver_t *TWTD_handle_t;

typedef terr_t (*TWTD_driver_cb_t) (void);

typedef struct TWTD_driver_t {
	terr_t (*init) (TWTD_driver_cb_t main,
					int N,
					TW_handle_t *ht);	  // Initialize the driver with N threads
	terr_t (*finalize) (TW_handle_t ht);  // Finalize the engine with N threads

	terr_t (*add_thread) (TW_handle_t ht, int N);  // Increase the number of threads by N
	terr_t (*end_thread) (TW_handle_t ht, int N);  // End current thread

	terr_t (*create_mutex) (TW_handle_t *ht);  // Initialize a mutex
	terr_t (*free_mutex) (TW_handle_t ht);	   // Free a mutex

	terr_t (*create_semaphore) (TW_handle_t *ht);  // Initialize a semaphore
	terr_t (*free_semaphore) (TW_handle_t ht);	   // Free a semaphore

	terr_t (*create_rw_lock) (TW_handle_t *ht);	 // Initialize a read/write lock
	terr_t (*free_rw_lock) (TW_handle_t ht);	 // Free a read/write lock

	terr_t (*create_condition_var) (TW_handle_t *ht);  // Initialize a conditional variable
	terr_t (*free_condition_var) (TW_handle_t ht);	   // Free a conditional variable
} TWTD_driver_t;

// To combine high and low-level lib, we need a flexible driver interface