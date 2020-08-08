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

#include "twi_disposer.h"

#include <stdint.h>

#define BIN_SIZE 4

TWI_Disposer_handle_t TWI_Disposer_create (void) {
	TWI_Disposer_handle_t dp;

	dp = (TWI_Disposer_handle_t)TWI_Malloc (sizeof (TWI_Disposer_t));
	if (dp) {
		if (TWI_Disposer_init (dp) != TW_SUCCESS) {
			TWI_Free (dp);
			dp = NULL;
		}
	}

	return dp;
}
terr_t TWI_Disposer_init (TWI_Disposer_handle_t dp) {
	terr_t err;

	err = TWI_Tls_init (&(dp->tid));
	CHECK_ERR
	err = TWI_Tls_init (&(dp->cnt));
	CHECK_ERR

	OPA_store_ptr (&(dp->head), NULL);

	dp->lock = TWI_Mutex_create ();
	CHECK_PTR (dp->lock);

	OPA_store_int (&(dp->ts), 1);  // Start at 1, we assume everyone agreed on 0
	dp->tmin = 0;

	dp->nt		 = 0;
	dp->nt_alloc = 32;
	dp->tss		 = (int *)TWI_Malloc (sizeof (uint64_t) * (size_t) (dp->nt_alloc));

err_out:;
	return err;
}

void TWI_Disposer_finalize (TWI_Disposer_handle_t dp) {
	TWI_Trash_t *i, *j;

	for (i = OPA_load_ptr (&(dp->head)); i; i = j) {
		j = i->next;

		i->handler (i->obj);
		TWI_Free (i);
	}

	TWI_Free (dp->tss);
	TWI_Mutex_free (dp->lock);
	TWI_Tls_finalize (dp->tid);
	TWI_Tls_finalize (dp->cnt);
}

void TWI_Disposer_free (TWI_Disposer_handle_t dp) {
	TWI_Disposer_finalize (dp);
	TWI_Free (dp);
}

void TWI_Disposer_join (TWI_Disposer_handle_t dp) {
	int cnt, id;
	void *tmp;

	cnt = (int)(size_t) (tmp = (TWI_Tls_get (dp->cnt)));

	TWI_Tls_store (dp->cnt, (void *)(size_t) (cnt + 1));  // Record this join

	if (cnt == 0) {	 // the first time we join
		TWI_Mutex_lock (dp->lock);

		id = dp->nt++;
		TWI_Tls_store (dp->tid, (void *)(size_t) (id));	 // When we join

		if (dp->nt == dp->nt_alloc) {
			dp->nt_alloc *= 2;
			dp->tss = (int *)TWI_Realloc (dp->tss, sizeof (int) * (size_t) (dp->nt_alloc));
		}

		dp->tss[id] = OPA_load_int (&(dp->ts));

		TWI_Mutex_unlock (dp->lock);
	}
}

void TWI_Disposer_leave (TWI_Disposer_handle_t dp) {
	int cnt;
	void *tmp;

	cnt = ((int)(size_t) (tmp = TWI_Tls_get (dp->cnt))) - 1;
	TWI_Tls_store (dp->cnt, (void *)(size_t)cnt);

	if (cnt == 0) {	 // The last time we leave
		TWI_Mutex_lock (dp->lock);
		dp->nt--;
		TWI_Mutex_unlock (dp->lock);
	}
}

terr_t TWI_Disposer_dispose (TWI_Disposer_handle_t dp, void *obj, TW_Trash_handler_t handler) {
	terr_t err = TW_SUCCESS;
	TWI_Trash_t *tp;

	tp = (TWI_Trash_t *)TWI_Malloc (sizeof (TWI_Trash_t));
	CHECK_PTR (tp);
	tp->handler = handler;
	tp->obj		= obj;

	// TWI_Mutex_rlock (dp->lock);
	tp->ts = OPA_fetch_and_incr_int (&(dp->ts));
	do {
		tp->next = OPA_load_ptr (&(dp->head));
	} while (OPA_cas_ptr (&(dp->head), tp->next, tp) != tp->next);
	// TWI_Mutex_runlock (dp->lock);

err_out:;
	return err;
}

void TWI_Disposer_flush (TWI_Disposer_handle_t dp) {
	int i;
	int id;
	int locked;
	int tmin;
	void *tmp;
	TWI_Trash_t *j, *k;

	id = (int)(size_t) (tmp = TWI_Tls_get (dp->tid));

	dp->tss[id] = OPA_load_int (&(dp->ts));

	// Check for every BIN_SIZE items
	if (dp->tss[id] - dp->tmin > BIN_SIZE) {
		// Only one thread need to do it
		TWI_Mutex_trylock (dp->lock, &locked);
		if (locked) {
			// Find min agreed time stamp
			tmin = 0;
			for (i = 0; i < dp->nt; i++) {
				if (tmin < dp->tss[i]) tmin = dp->tss[i];
			}

			// Search for first node agreed by everyone
			for (j = OPA_load_ptr (&(dp->head)); j; j = k) {
				k = j->next;

				if (j->ts <= tmin) {
					j->next = NULL;
					break;
				}
			}

			// For easier implementation, we only delete nodes afterward, leave the node itself to
			// next time
			for (; k; k = j) {
				j = k->next;
				k->handler (k->obj);
				TWI_Free (k);
			}

			dp->tmin = tmin;
			TWI_Mutex_unlock (dp->lock);
		}
	}
}
