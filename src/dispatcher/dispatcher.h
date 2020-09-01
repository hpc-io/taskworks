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

#ifdef HAVE_MPI

typedef struct TW_Event_poll_mpi_data {
	MPI_Comm comm;
	int src;
	int tag;
	int flag;
	MPI_Status status;
} TW_Event_poll_mpi_data;

typedef struct TW_Event_poll_mpi_req_data {
	MPI_Request req;
	int flag;
	MPI_Status status;
} TW_Event_poll_mpi_req_data;

#endif

extern TW_Task_dep_handler_t TWI_Task_dep_null;
extern TW_Task_dep_handler_t TWI_Task_dep_all_complete;

extern TW_Event_poll_response_t TWI_Event_poll_mpi (void *data);
extern TW_Event_poll_response_t TWI_Event_poll_mpi_req (void *data);