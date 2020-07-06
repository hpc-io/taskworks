/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Memory wrapper implementations */

#include <stdlib.h>

#include "taskworks_internal.h"

void *TWI_Malloc (size_t size) { return malloc (size); }

void *TWI_Realloc (void *old_ptr, size_t size) { return relloc (old_ptr, size); }

void TWI_Free (void *ptr) { free (ptr); }
