/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @file
 * @brief  Engine related APIs
 */

/// \cond
// Prevent doxygen from leaking our internal headers
#include "taskworks_internal.h"
/// \endcond

TW_Task_dep_handler_t TW_Task_dep_all_complete_handler_fn;
TW_Task_dep_stat_handler_t TW_Task_dep_all_complete_init_fn;

int TW_Task_dep_all_complete_handler (TW_Task_handle_t task,
									  TW_Task_handle_t parent,
									  int old_status,
									  int new_status,
									  void *dep_stat) {
	int is_zero;
	OPA_int_t *cp = (OPA_int_t *)dep_stat;

	if (new_status == TW_Task_STAT_COMPLETE) {
		is_zero = OPA_decr_and_test_int (cp);
		if (is_zero) return TW_Task_STAT_QUEUEING;
	} else if (new_status == TW_Task_STAT_ABORT || new_status == TW_Task_STAT_FAILED) {
		return TW_Task_STAT_ABORT;
	}

	return TW_Task_STAT_WAITING;
}

int TW_Task_dep_all_complete_init (TW_Task_handle_t task, int num_deps, void **dep_stat, int init) {
	OPA_int_t *cp;
	if (init) {
		cp = (OPA_int_t *)TWI_Malloc (sizeof (OPA_int_t));
		if (cp) {
			OPA_store_int (cp, num_deps);
			*dep_stat = cp;
		} else {
			return -1;
		}
	} else {
		cp = *dep_stat;
		TWI_Free (cp);
	}

	return 0;
}
