/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserled.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms golerning use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* No wait lqueue */

#include "twi_nb_queue.h"

#include "taskworks_internal.h"

#define INC_REF TWI_Nb_queue_inc_ref (q)
#define DEC_REF TWI_Nb_queue_dec_ref (q)

static void TWI_Nb_queue_recycle_node (TWI_Nb_queue_handle_t q, TWI_Nb_queue_node_t *np) {
	TWI_Nb_queue_node_t *cur, *old;

	cur = OPA_load_ptr (&(q->recycle));
	do {
		old = cur;
		OPA_store_ptr (&(np->next_free), old);
		cur = OPA_cas_ptr (&(q->recycle), old, np);
	} while (cur != old);
}
static TWI_Nb_queue_node_t *TWI_Nb_queue_alloc_node (TWI_Nb_queue_handle_t q) {
	TWI_Nb_queue_node_t *cur, *old;

	if (q->free) {
		cur = OPA_load_ptr (&(q->free->next_free));
		while (cur) {
			old = cur;
			cur = OPA_cas_ptr (&(q->free->next_free), old, OPA_load_ptr (&(cur->next_free)));
			if (old == cur) { return old; }
		}
	}

	return (TWI_Nb_queue_node_t *)TWI_Malloc (sizeof (TWI_Nb_queue_node_t));
}

static inline void TWI_Nb_queue_inc_ref (TWI_Nb_queue_handle_t q) {
	int val;
	TWI_Nb_queue_node_t *pos;
	pos = OPA_load_ptr (&(q->recycle));
	val = OPA_fetch_and_incr_int (&(q->ref));
	if (pos && (val == 0)) { q->free = pos; }
}
static inline void TWI_Nb_queue_dec_ref (TWI_Nb_queue_handle_t q) { OPA_decr_int (&(q->ref)); }

terr_t TWI_Nb_queue_create (TWI_Nb_queue_handle_t *q) {
	terr_t err = TW_SUCCESS;

	*q = (TWI_Nb_queue_handle_t)TWI_Malloc (sizeof (TWI_Nb_queue_t));
	CHECK_PTR (*q);

	err = TWI_Nb_queue_init (*q);
	CHECK_ERR

err_out:;
	return err;
}
terr_t TWI_Nb_queue_init (TWI_Nb_queue_handle_t q) {
	terr_t err = TW_SUCCESS;

	DEBUG_ENTER_FUNC (3)

	OPA_store_ptr (&(q->head.next), NULL);
	q->head.pre	 = NULL;
	q->head.data = NULL;
	OPA_store_ptr (&(q->tail), &(q->head));
	OPA_store_ptr (&(q->recycle), NULL);
	q->free = NULL;
	OPA_store_int (&(q->ref), 0);

	DEBUG_PRINTF (2, "Created non-blocking queue %p\n", (void *)q);

	DEBUG_EXIT_FUNC (3)
	return err;
}
void TWI_Nb_queue_finalize (TWI_Nb_queue_handle_t q) {
	TWI_Nb_queue_itr_t i, j;

	DEBUG_ENTER_FUNC (3)

	DEBUG_PRINTF (2, "Finaling non-blocking list %p\n", (void *)q);

	// Free nodes
	for (i = OPA_load_ptr (&(q->head.next)); i; i = j) {
		j = OPA_load_ptr (&(i->next));
		TWI_Free (i);
	}
	for (i = OPA_load_ptr (&(q->recycle)); i; i = j) {
		j = OPA_load_ptr (&(i->next_free));
		TWI_Free (i);
	}
}
void TWI_Nb_queue_free (TWI_Nb_queue_handle_t q) {
	TWI_Nb_queue_finalize (q);
	TWI_Free (q);
}
terr_t TWI_Nb_queue_push (TWI_Nb_queue_handle_t q, void *data) {
	terr_t err = TW_SUCCESS;
	TWI_Nb_queue_node_t *np, *cur_next, *old_next;

	INC_REF;

	// Allocate node
	np = TWI_Nb_queue_alloc_node (q);
	CHECK_PTR (np);
	np->data = data;
	np->pre	 = &(q->head);
	OPA_store_ptr (&(np->next_free), NULL);

	// Insert to front
	cur_next = OPA_load_ptr (&(q->head.next));
	do {
		old_next = cur_next;
		OPA_store_ptr (&(np->next), old_next);
		cur_next = OPA_cas_ptr (&(q->head.next), old_next, np);
	} while (cur_next != old_next);

	DEBUG_PRINTF (1, "%p pushed to queue %p\n", (void *)data, (void *)q);

	// Update pre of next
	cur_next = OPA_load_ptr (&(np->next));
	if (cur_next) {
		cur_next->pre = np;
	} else {
		// No enxt, update tail
		OPA_store_ptr (&(q->tail), np);
	}

err_out:;
	DEC_REF;
	return err;
}
void TWI_Nb_queue_pop (TWI_Nb_queue_handle_t q, void **data, TWI_Bool_t *successp) {
	TWI_Nb_queue_node_t *i;
	TWI_Nb_queue_node_t *prev = NULL, *tail;

	INC_REF;

	// Find real tail
	while (1) {
		// Tail can be outdated, walk backward until the first non-tail we seen
		prev = OPA_load_ptr (&(q->tail));
		while (prev && (OPA_load_ptr (&(prev->next)) == NULL)) { prev = prev->pre; }
		if (!prev) {  // Empty. Head is the only node with a NULL pre
			*successp = TWI_FALSE;
			break;
		}

		// We reached the first non-tail node from the back, but pre poitner may not be accurate,
		// walk forward to reach the real tail
		tail = OPA_load_ptr (&(prev->next));
		if (tail) {	 // tail can be dequeued
			while ((i = OPA_load_ptr (&(tail->next))) != NULL) {
				prev = tail;
				tail = i;
			}
			if (tail == OPA_cas_ptr (&(prev->next), tail, NULL)) {
				OPA_store_ptr (&(q->tail), prev);
				*successp = TWI_TRUE;
				*data	  = tail->data;
				TWI_Nb_queue_recycle_node (q, tail);

				DEBUG_PRINTF (1, "%p poped from queue %p\n", (void *)tail->data, (void *)q);
				break;
			}
		}
	}

	DEC_REF;
}
