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
#include "dispatcher.h"
/// \endcond

int TWI_Task_dep_all_complete_status_change (TW_Task_handle_t TWI_UNUSED task,
											 TW_Task_handle_t TWI_UNUSED parent,
											 int TWI_UNUSED old_status,
											 int new_status,
											 void *dep_data);
int TWI_Task_dep_all_complete_init (TW_Task_handle_t TWI_UNUSED task,
									int num_deps,
									void **dep_data);
int TWI_Task_dep_all_complete_finalize (TW_Task_handle_t TWI_UNUSED task, void *dep_data);

TW_Task_dep_handler_t TW_Task_dep_null;
TW_Task_dep_handler_t TWI_Task_dep_null = {
	0, NULL, NULL, NULL, NULL,
};
TW_Task_dep_handler_t TW_Task_dep_all_complete;
TW_Task_dep_handler_t TWI_Task_dep_all_complete = {
	TW_Task_STAT_COMPLETE,
	NULL,
	TWI_Task_dep_all_complete_init,
	TWI_Task_dep_all_complete_finalize,
	TWI_Task_dep_all_complete_status_change,
};

int TWI_Task_dep_all_complete_status_change (TW_Task_handle_t TWI_UNUSED task,
											 TW_Task_handle_t TWI_UNUSED parent,
											 int TWI_UNUSED old_status,
											 int new_status,
											 void *dep_data) {
	int is_zero;
	OPA_int_t *cp = (OPA_int_t *)dep_data;

	if (new_status == TW_Task_STAT_COMPLETE) {
		is_zero = OPA_decr_and_test_int (cp);
		if (is_zero) return TW_Task_STAT_QUEUEING;
	} else if (new_status == TW_Task_STAT_ABORT || new_status == TW_Task_STAT_FAILED) {
		return TW_Task_STAT_ABORT;
	}

	return TW_Task_STAT_WAITING;
}

int TWI_Task_dep_all_complete_init (TW_Task_handle_t TWI_UNUSED task,
									int num_deps,
									void **dep_data) {
	OPA_int_t *cp;

	cp = (OPA_int_t *)TWI_Malloc (sizeof (OPA_int_t));
	if (cp) {
		OPA_store_int (cp, num_deps);
		*dep_data = cp;
	} else {
		return -1;
	}

	return 0;
}

int TWI_Task_dep_all_complete_finalize (TW_Task_handle_t TWI_UNUSED task, void *dep_data) {
	TWI_Free (dep_data);
	return 0;
}
