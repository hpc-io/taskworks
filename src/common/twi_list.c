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

terr_t TWI_List_create (TWI_List_handle_t *l) {
	terr_t err = TW_SUCCESS;

	*l = (TWI_List_handle_t)TWI_Malloc (sizeof (TWI_List_t));
	CHK_PTR (*l);

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

terr_t TWI_List_insert (TWI_List_handle_t l, void *data) {
	terr_t err = TW_SUCCESS;
	TWI_List_itr_t new_node;
	TWI_List_itr_t old_head, cur_head;

	new_node = (TWI_List_itr_t)TWI_Malloc (sizeof (TWI_List_node_t));
	CHK_PTR (new_node);

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

terr_t TWI_List_insert_at (TWI_List_itr_t itr, void *data) { return TW_ERR_NOT_SUPPORTED; }

terr_t TWI_List_erase (TWI_List_handle_t l, void *data) {
	terr_t err = TW_SUCCESS;
	TWI_List_itr_t i, nxt, old_next;
	OPA_ptr_t *prep, *new_prep;

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
TWI_List_itr_t TWI_List_head (TWI_List_handle_t l) {
	return (TWI_List_itr_t)OPA_load_ptr (&(l->head));
}
TWI_List_itr_t TWI_List_next (TWI_List_itr_t itr) {
	return (TWI_List_itr_t)OPA_load_ptr (&(itr->next));
}