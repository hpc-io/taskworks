/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Function that does not involve drivers */

#pragma once

#include "taskworks.h"

#define TW_TIMEOUT_NEVER -1LL

typedef int terr_t;

terr_t TW_Init (int *argc, char ***argv);
terr_t TW_Finalize ();

const char *TW_Get_err_msg (terr_t err);