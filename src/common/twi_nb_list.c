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

/* No wait linked list */

#include "twi_nb_list.h"

#include <stdint.h>

#include "taskworks_internal.h"

#define DEL_FLAG  0x01ULL
#define FLAG_MASK 0xfffffffffffffff4ULL

#define MARK_DEL(A)	 ((TWI_Nb_list_itr_t) ((uint64_t) (A) | DEL_FLAG))
#define GET_PTR(A)	 ((TWI_Nb_list_itr_t) ((uint64_t) (A)&FLAG_MASK))
#define IS_SET(A, B) ((uint64_t) (A)&B)
//#define ONDEL(A)	 IS_SET (OPA_load_ptr (&(A->next)), DEL_FLAG)
//#define NEXT(A)		 GET_PTR (OPA_load_ptr (&(A->next)))
//#define PRE(A)		 GET_PTR (OPA_load_ptr (&(A->pre)))

#define INC_REF TWI_Nb_list_inc_ref (l)
#define DEC_REF TWI_Nb_list_dec_ref (l)

static inline void TWI_Nb_list_recycle (TWI_Nb_list_handle_t l, TWI_Nb_list_itr_t pos) {
	TWI_Bool_t done = TWI_FALSE;
	TWI_Nb_list_itr_t i, j;

	while (done != TWI_TRUE) {
		done = TWI_TRUE;
		i	 = &(l->head);
		while (i && i != pos) {
			j = i;
			i = (TWI_Nb_list_itr_t)OPA_load_ptr (&(i->next_free));
		}
		if (i) {
			if (pos == OPA_cas_ptr (&(j->next_free), pos, NULL)) {
				while (i) {
					j = i;
					i = (TWI_Nb_list_itr_t)OPA_load_ptr (&(i->next_free));
					TWI_Free (j);
				}
			} else {
				done = TWI_FALSE;
			}
		}
	}
}

terr_t TWI_Nb_list_create (TWI_Nb_list_handle_t *l) {
	terr_t err = TW_SUCCESS;

	*l = (TWI_Nb_list_handle_t)TWI_Malloc (sizeof (TWI_Nb_list_t));
	CHECK_PTR (*l);

	err = TWI_Nb_list_init (*l);
	CHECK_ERR

err_out:;
	return err;
}

terr_t TWI_Nb_list_free (TWI_Nb_list_handle_t l) {
	terr_t err = TW_SUCCESS;

	err = TWI_Nb_list_finalize (l);
	CHECK_ERR

	TWI_Free (l);

err_out:;
	return err;
}

terr_t TWI_Nb_list_init (TWI_Nb_list_handle_t l) {
	terr_t err = TW_SUCCESS;

	DEBUG_ENTER_FUNC (3)

	OPA_store_ptr (&(l->head.next), &(l->tail));
	OPA_store_ptr (&(l->tail.next), NULL);
	OPA_store_ptr (&(l->head.next_free), NULL);
	OPA_store_ptr (&(l->tail.next_free), NULL);
	OPA_store_int (&(l->ref), 0);

	DEBUG_PRINTF (2, "Created non-blocking list %p\n", (void *)l);

	DEBUG_EXIT_FUNC (3)
	return err;
}

terr_t TWI_Nb_list_finalize (TWI_Nb_list_handle_t l) {
	terr_t err = TW_SUCCESS;
	TWI_Nb_list_itr_t i, j;

	DEBUG_ENTER_FUNC (3)

	DEBUG_PRINTF (2, "Finaling non-blocking list %p\n", (void *)l);

	if (!l) { ASSIGN_ERR (TW_ERR_INVAL) }

	i = OPA_load_ptr (&(l->head.next));
	while (i != TWI_Nb_list_end (l)) {
		j = i;
		i = OPA_load_ptr (&(i->next));
		TWI_Free (j);
	}
	OPA_store_ptr (&(l->head.next), NULL);

err_out:;
	DEBUG_EXIT_FUNC (3)
	return err;
}

terr_t TWI_Nb_list_insert_front (TWI_Nb_list_handle_t l, void *data) {
	terr_t err = TW_SUCCESS;

	INC_REF;

	err = TWI_Nb_list_insert_at (l, &(l->head), data, NULL);
	CHECK_ERR

	DEC_REF;

err_out:;
	return err;
}

terr_t TWI_Nb_list_insert_at (TWI_Nb_list_handle_t l,
							  TWI_Nb_list_itr_t pos,
							  void *data,
							  TWI_Bool_t *successp) {
	terr_t err				   = TW_SUCCESS;
	TWI_Bool_t success		   = TWI_TRUE;
	TWI_Nb_list_itr_t new_node = NULL;
	TWI_Nb_list_itr_t old_next, cur_next;

	INC_REF;

	// if (pos == &(l->tail)) ASSIGN_ERR (TW_ERR_INVAL)

	new_node = (TWI_Nb_list_itr_t)TWI_Malloc (sizeof (TWI_Nb_list_node_t));
	CHECK_PTR (new_node);
	new_node->data = data;

	// The next pointer of previous node
	/*
	if (pos) {
		next = &(pos->next);
		pre	 = pos;
		OPA_store_ptr (&(new_node->pre), pos);
	} else {
		next = &(l->head);
		pre	 = NULL;
		OPA_store_ptr (&(new_node->pre), NULL);
	}
	*/

	// Try update the next of pos
	cur_next = OPA_load_ptr (&(pos->next));
	do {
		if (IS_SET (cur_next, DEL_FLAG)) {
			success = TWI_FALSE;
			break;
		}
		old_next = cur_next;
		OPA_store_ptr (&(new_node->next), old_next);
		cur_next = OPA_cas_ptr (&(pos->next), old_next, new_node);
	} while (cur_next != old_next);

	/*
		if (success) {
			// Try update the prev of the next node
			// pre are not guaranteed to be up to date, so its ok to fail
			OPA_cas_ptr (&(old_next->pre), pre, new_node);
		} else {
			TWI_Free (new_node);
		}
	*/

	if (successp) { *successp = success; }

err_out:;
	if (err || (!success)) { TWI_Free (new_node); }

	DEC_REF;

	return err;
}

terr_t TWI_Nb_list_del_at (TWI_Nb_list_handle_t l, TWI_Nb_list_itr_t pos, int *successp) {
	terr_t err		   = TW_SUCCESS;
	TWI_Bool_t success = TWI_TRUE;
	int step		   = 0;
	TWI_Nb_list_itr_t pre, next, new_next, pre_next;

	INC_REF;

	// if (pos == &(l->head) || pos == &(l->tail)) ASSIGN_ERR (TW_ERR_INVAL)

	// Mark the next pointer for deletion
	while (step < 2) {
		next = TWI_Nb_list_next (pos);
		pre	 = TWI_Nb_list_pre (l, pos);

		switch (step) {
			case 0:;
				new_next = OPA_cas_ptr (&(pos->next), next, MARK_DEL (next));
				if (new_next == next) {
					step++;
				} else if (IS_SET (new_next, DEL_FLAG)) {
					// Abort, other already deleted by ither thread
					success = TWI_FALSE;
					step	= 5;
				}
				/* fall through */
				TWI_FALL_THROUGH;
			case 1:;
				// Linearization point
				pre_next = OPA_cas_ptr (&(pre->next), pos, next);
				if (pre_next == pos) { step++; }
			default:;
		}
	}

	if (success) {
		// Place on a list of free nodes
		new_next = OPA_load_ptr (&(l->head.next_free));
		do {
			next = new_next;
			OPA_store_ptr (&(pos->next_free), next);
			new_next = OPA_cas_ptr (&(l->head.next_free), next, pos);
		} while (next != new_next);
	}

	if (successp) *successp = success;

	// err_out:;
	DEC_REF;
	return err;
}

TWI_Nb_list_itr_t TWI_Nb_list_find (TWI_Nb_list_handle_t l, void *data) {
	TWI_Nb_list_itr_t i;

	INC_REF;

	for (i = TWI_Nb_list_begin (l); i != TWI_Nb_list_end (l); i = TWI_Nb_list_next (i)) {
		if (i->data == data) { break; }
	}

	INC_REF;
	return i;
}

terr_t TWI_Nb_list_del (TWI_Nb_list_handle_t l, void *data) {
	terr_t err		   = TW_SUCCESS;
	TWI_Bool_t success = TWI_FALSE;
	TWI_Nb_list_itr_t i;

	INC_REF;

	do {
		i = TWI_Nb_list_find (l, data);
		if (i == TWI_Nb_list_end (l)) { break; }
		err = TWI_Nb_list_del_at (l, i, &success);
		CHECK_ERR
	} while (success != TWI_TRUE);

	/*
		prep = &(l->head);
		i	 = OPA_load_ptr (prep);
		while (i) {
			if (i->data != data) {
				prep = &(i->next);
				i	 = OPA_load_ptr (prep);
			} else {
				nxt = OPA_load_ptr (&(i->next));

				if (i == OPA_cas_ptr (prep, i, nxt)) {
					break;
				} else {
					// Continue search updated node
					i = OPA_load_ptr (prep);
				}
			}
		}
	*/
err_out:;
	DEC_REF;
	return err;
}

TWI_Nb_list_itr_t TWI_Nb_list_begin (TWI_Nb_list_handle_t l) {
	TWI_Nb_list_itr_t ret;

	DEBUG_ENTER_FUNC (3)

	ret = (TWI_Nb_list_itr_t)OPA_load_ptr (&(l->head.next));

	DEBUG_EXIT_FUNC (3)
	return ret;
}
TWI_Nb_list_itr_t TWI_Nb_list_end (TWI_Nb_list_handle_t l) { return &(l->tail); }
TWI_Nb_list_itr_t TWI_Nb_list_next (TWI_Nb_list_itr_t itr) {
	if (itr) {
		void *tmp = OPA_load_ptr (&(itr->next));

		return ((TWI_Nb_list_itr_t)GET_PTR (tmp));
	}

	return NULL;
}
TWI_Nb_list_itr_t TWI_Nb_list_pre (TWI_Nb_list_handle_t l, TWI_Nb_list_itr_t itr) {
	TWI_Nb_list_itr_t cur, pre;

	pre = &(l->head);
	for (cur = TWI_Nb_list_begin (l); cur != TWI_Nb_list_end (l); cur = TWI_Nb_list_next (cur)) {
		if (cur == itr) { break; }
		pre = cur;
	}

	if (cur == TWI_Nb_list_end (l)) return NULL;
	return pre;

	/*
		pre = PRE (itr);
		if (!pre) pre = OPA_load_ptr (&(l->head));

		do {
			old_pre = pre;

			if (ONDEL (pre)) {	// If deleted, search until previous available node
				do { pre = TWI_Nb_list_pre (l, pre); } while (pre && ONDEL (pre));
			} else {
				cur = pre;
				while (cur != itr) {
					pre = cur;
					cur = Next (cur);
				}
			}

			if (old_pre != pre) {
				if (ONDEL (itr)) {
					pre = OPA_cas_ptr (&(itr->pre), MARK_DEL (old_pre), MARK_DEL (pre));
				} else {
					pre = OPA_cas_ptr (&(itr->pre), old_pre, pre);
				}
				pre = GET_PTR (pre);
			}
		} while (pre != old_pre);
		*/

	return pre;
}

void TWI_Nb_list_inc_ref (TWI_Nb_list_handle_t l) {
	int val;
	TWI_Nb_list_itr_t pos;
	pos = OPA_load_ptr (&(l->head.next_free));
	val = OPA_fetch_and_incr_int (&(l->ref));
	if (val == 0) { TWI_Nb_list_recycle (l, pos); }
}

void TWI_Nb_list_dec_ref (TWI_Nb_list_handle_t l) { OPA_decr_int (&(l->ref)); }
