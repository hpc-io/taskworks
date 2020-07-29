/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Libevent driver implementation */

#include "twlibevt.h"

terr_t TWLIBEVT_Loop_create (TW_Handle_t TWI_UNUSED *loop) { return TW_SUCCESS; }
terr_t TWLIBEVT_Loop_free (TW_Handle_t TWI_UNUSED engine) { return TW_SUCCESS; }
terr_t TWLIBEVT_Loop_do_work (TW_Handle_t TWI_UNUSED engine, ttime_t TWI_UNUSED timeout) {
	return TW_SUCCESS;
}