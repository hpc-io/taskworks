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

/* Thread safe List */

#include "twi_list.h"

#include <stdint.h>

#include "taskworks_internal.h"

//#define DEL_FLAG  0x01ULL
#define FLAG_MASK 0xfffffffffffffff4ULL

//#define MARK_DEL(A)	 ((TW_Handle_t) ((uint64_t) (A) | DEL_FLAG))
#define GET_PTR(A) ((TW_Handle_t) ((uint64_t) (A)&FLAG_MASK))
//#define IS_SET(A, B) ((uint64_t) (A)&B)

terr_t TWI_List_create (TWI_List_handle_t *l) {
	terr_t err = TW_SUCCESS;

	*l = (TWI_List_handle_t)TWI_Malloc (sizeof (TWI_List_t));
	CHECK_PTR (*l);

	OPA_store_ptr (&((*l)->head), NULL);

err_out:;
	return err;
}

terr_t TWI_List_free (TWI_List_handle_t l) {
	TWI_List_itr_t i, j;

	i = OPA_load_ptr (&(l->head));
	while (i) {
		j = i;
		i = OPA_load_ptr (&(i->next));
		TWI_Free (j);
	}

	TWI_Free (l);

	return TW_SUCCESS;
}

terr_t TWI_List_insert_front (TWI_List_handle_t l, void *data) {
	terr_t err = TW_SUCCESS;
	TWI_List_itr_t new_node;
	TWI_List_itr_t old_head, cur_head;

	// Create new node
	new_node = (TWI_List_itr_t)TWI_Malloc (sizeof (TWI_List_node_t));
	CHECK_PTR (new_node);
	new_node->data = data;

	// Try insert until we success
	cur_head = OPA_load_ptr (&(l->head));
	do {
		old_head = cur_head;
		OPA_store_ptr (&(new_node->next), old_head);
		cur_head = OPA_cas_ptr (&(l->head), old_head, new_node);
	} while (cur_head != old_head);

err_out:;
	return err;
}
/*
terr_t TWI_List_insert (TWI_List_handle_t l, TWI_List_itr_t pos, void *data, TWI_Bool_t *success) {
	terr_t err = TW_SUCCESS;
	OPA_ptr_t *next;
	TWI_Bool_t succ = TWI_TRUE;
	TWI_List_itr_t new_node;
	TWI_List_itr_t old_next, cur_next, pre;

	new_node = (TWI_List_itr_t)TWI_Malloc (sizeof (TWI_List_node_t));
	CHECK_PTR (new_node);
	new_node->data = data;

	// The next pointer of previous node
	if (pos) {
		next = &(pos->next);
		pre	 = pos;
		OPA_store_ptr (&(new_node->pre), pos);
	} else {
		next = &(l->head);
		pre	 = NULL;
		OPA_store_ptr (&(new_node->pre), NULL);
	}

	// Try update the next of prev node
	cur_next = OPA_load_ptr (next);
	do {
		if (IS_SET (cur_next, DEL_FLAG)) {
			succ = TWI_FALSE;
			break;
		}
		old_next = cur_next;
		OPA_store_ptr (&(new_node->next), old_next);
		cur_next = OPA_cas_ptr (next, old_next, new_node);
	} while (cur_next != old_next);

	if (succ) {
		// Try update the prev of the next node
		// pre are not guaranteed to be up to date, so its ok to fail
		OPA_cas_ptr (&(old_next->pre), pre, new_node);
	} else {
		TWI_Free (new_node);
	}

	if (success) { *success = succ; }

err_out:;
	return err;
}

terr_t TWI_List_del (TWI_List_handle_t l, TWI_List_itr_t pos, int *success) {
	terr_t err		= TW_SUCCESS;
	TWI_Bool_t succ = TWI_TRUE;
	TWI_List_itr_t pre, next, new_next, new_prev;
	OPA_ptr_t *prep;

	// Mark the next pointer for deletion
	new_next = OPA_load_ptr (&(pos->next));
	do {
		if (IS_SET (new_next, DEL_FLAG)) {
			break;
			succ = TWI_FALSE;
		}

		next	 = new_next;
		new_next = OPA_cas_ptr (&(pos->next), next, MARK_DEL (next));
	} while (new_next != next);

	if (succ) {
		// Mark the pre pointer for deletion
		// Since we won the previous CAS, it will not fail
		new_prev = OPA_load_ptr (&(pos->pre));
		do {
			pre		 = new_prev;
			new_prev = OPA_cas_ptr (&(pos->next), pre, MARK_DEL (pre));
		} while (new_prev != pre);

		// Case the next ptr of the pree node
		new_next = OPA_load_ptr (&(pos->next));
		do {
			if (IS_SET (new_next, DEL_FLAG)) {
				break;
				succ = TWI_FALSE;
			}

			next	 = new_next;
			new_next = OPA_cas_ptr (&(pos->next), next, MARK_DEL (next));
		} while (new_next != next);

		//
	}

	// TODO: How to safely free the node, other thread may still be in

	return err;
}
*/
terr_t TWI_List_erase (TWI_List_handle_t l, void *data) {
	terr_t err = TW_SUCCESS;
	TWI_List_itr_t i, nxt;
	OPA_ptr_t *prep;

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

	// TODO: How to safely free the node, other thread may still be in

	return err;
}

TWI_List_itr_t TWI_List_begin (TWI_List_handle_t l) {
	return (TWI_List_itr_t)OPA_load_ptr (&(l->head));
}
TWI_List_itr_t TWI_List_next (TWI_List_itr_t itr) {
	void *tmp = OPA_load_ptr (&(itr->next));

	return ((TWI_List_itr_t)GET_PTR (tmp));
}
TWI_List_itr_t TWI_List_pre (TWI_List_itr_t itr) {
	TWI_List_itr_t old_pre, new_pre, pre, cur;

	new_pre = OPA_load_ptr (&(itr->pre));
	do {
		old_pre = new_pre;
		cur		= old_pre;
		do {  // Iterate until meet current node
			pre = cur;
			cur = TWI_List_next (pre);
		} while (cur != itr);
		// Try CAS if outdated
		if (old_pre != pre) { new_pre = OPA_cas_ptr (&(itr->pre), old_pre, pre); }
	} while (new_pre != old_pre);

	return (TWI_List_itr_t)GET_PTR (pre);
}