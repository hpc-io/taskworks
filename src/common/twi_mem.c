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

/* Memory wrapper implementations */

#include "twi_mem.h"

void *TWI_Malloc (size_t size) { return malloc (size); }

void *TWI_Realloc (void *old_ptr, size_t size) { return realloc (old_ptr, size); }

void TWI_Free (void *ptr) { free (ptr); }
