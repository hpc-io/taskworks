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

#include "twi_vector.h"

TWI_Vector_handle_t TWI_Vector_create (void) {
	terr_t err			  = TW_SUCCESS;
	TWI_Vector_handle_t s = NULL;

	s = (TWI_Vector_handle_t)TWI_Malloc (sizeof (TWI_Vector_t));
	CHECK_PTR (s);

	err = TWI_Vector_init (s);
	CHECK_ERR

err_out:;
	if (err) {
		TWI_Free (s);
		s = NULL;
	}
	return s;
}
terr_t TWI_Vector_init (TWI_Vector_handle_t s) {
	terr_t err = TW_SUCCESS;

	DEBUG_ENTER_FUNC (3)

	s->base = (void **)TWI_Malloc (sizeof (void *) * TWI_STACK_INIT_SIZE);
	CHECK_PTR (s->base);
	s->end = s->base + TWI_STACK_INIT_SIZE;
	s->ptr = s->base;

err_out:;
	DEBUG_EXIT_FUNC (3)
	return err;
}
void TWI_Vector_finalize (TWI_Vector_handle_t s) {
	DEBUG_ENTER_FUNC (3)

	TWI_Free (s->base);
	s->end = s->base;
	s->ptr = s->base;

	DEBUG_EXIT_FUNC (3)
}
void TWI_Vector_free (TWI_Vector_handle_t s) {
	DEBUG_ENTER_FUNC (3)

	TWI_Vector_finalize (s);
	TWI_Free (s);

	DEBUG_EXIT_FUNC (3)
}
size_t TWI_Vector_size (TWI_Vector_handle_t s) { return (size_t) (s->ptr - s->base); }
static inline terr_t TWI_Vector_extend (TWI_Vector_handle_t s, size_t size) {
	terr_t err = TW_SUCCESS;
	size_t nalloc;
	void *tmp;

	nalloc = (size_t) (s->end - s->base);
	if (size > nalloc) {
		do { nalloc <<= TWI_STACK_SHIFT_AMOUNT; } while (size > nalloc);

		tmp = TWI_Realloc (s->base, sizeof (void *) * nalloc);
		CHECK_PTR (tmp)
		s->base = tmp;
		s->end	= s->base + nalloc;
	}

err_out:;
	return err;
}
terr_t TWI_Vector_resize (TWI_Vector_handle_t s, size_t size) {
	terr_t err = TW_SUCCESS;

	s->ptr = s->base + size;
	if (s->ptr > s->end) {
		err = TWI_Vector_extend (s, size);
		CHECK_ERR
	}

err_out:;
	return err;
}
terr_t TWI_Vector_push_back (TWI_Vector_handle_t s, void *data) {
	terr_t err = TW_SUCCESS;

	// Increase size if needed
	if (s->ptr == s->end) {
		err = TWI_Vector_extend (s, (size_t) (s->end - s->base + 1));
		CHECK_ERR
	}

	// Push to stack
	*(s->ptr) = data;
	(s->ptr)++;

err_out:;
	return err;
}
terr_t TWI_Vector_pop_back (TWI_Vector_handle_t s, void **data) {
	if (s->ptr == s->base)
		return TW_ERR_INVAL;
	else {
		(s->ptr)--;
		*data = *(s->ptr);
	}
	return TW_SUCCESS;
}
