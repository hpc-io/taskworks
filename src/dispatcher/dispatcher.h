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

#include "driver.h"
#include "eal.h"
#include "tal.h"
#include "taskworks_internal.h"

#define CHK_HANDLE(H, T)                                                                \
	{                                                                                   \
		if (!H || H->objtype != T) { ASSIGN_ERR (TW_ERR_INVAL_HANDLE) }                 \
		if (H->driver != TWI_Active_driver) { ASSIGN_ERR (TW_ERR_INCOMPATIBLE_OBJECT) } \
	}

typedef enum TW_Obj_type {
	TW_Obj_type_engine,
	TW_Obj_type_task,
	TW_Obj_type_event,
	TW_Obj_type_null
} TW_Obj_type;

typedef struct TW_Obj_t {
	TW_Obj_type objtype;
	TW_Driver_handle_t driver;			  // Driver associated with the object
	TW_Event_driver_handle_t evt_driver;  // Driver associated with the object
	TW_Handle_t driver_obj;				  // Driver specific object
} TW_Obj_t;

typedef TW_Obj_t *TW_Obj_handle_t;

extern TW_Task_dep_handler_t TWI_Task_dep_null;
extern TW_Task_dep_handler_t TWI_Task_dep_all_complete;