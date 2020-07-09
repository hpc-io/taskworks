/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Dispatcher related functiona and type */

#include "taskworks_internal.h"

#define CHK_HANDLE(H, T)                                                             \
	{                                                                                \
		if (!H || H->objtype != T) { RET_ERR (TW_ERR_INVAL_HANDLE) }                 \
		if (H->driver != TWI_Active_driver) { RET_ERR (TW_ERR_INCOMPATIBLE_OBJECT) } \
	}

typedef enum TW_Obj_type {
	TW_Obj_type_engine,
	TW_Obj_type_task,
	TW_Obj_type_event,
	TW_Obj_type_null
} TW_Obj_type;

typedef struct TW_Obj_t {
	TW_Obj_type objtype;
	union {
		TW_Driver_handle_t driver;			  // Driver associated with the object
		TW_Event_driver_handle_t evt_driver;  // Driver associated with the object
	};
	TW_Handle_t driver_obj;	 // Driver specific object
} TW_Obj_t;

typedef TW_Obj_t *TW_Obj_handle_t;

extern TW_Event_driver_handle_t TWI_Active_evt_driver;
extern TW_Driver_handle_t TWI_Active_driver;