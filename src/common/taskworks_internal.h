/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Common internal routines */

#pragma once

#include <config.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include <opa_primitives.h>
#include <taskworks.h>

#include "common.h"
#include "debug.h"
#include "twi_disposer.h"
#include "twi_err.h"
#include "twi_hash.h"
#include "twi_mem.h"
#include "twi_mutex.h"
#include "twi_nb_list.h"
#include "twi_nb_queue.h"
#include "twi_rwlock.h"
#include "twi_time.h"
#include "twi_tls.h"
#include "twi_ts_vector.h"
#include "twi_vector.h"

#define TWI_TASK_NUM_PRIORITY_LEVEL (TW_TASK_PRIORITY_LAZY + 1)
#define TW_TASK_PRIORITY_RESERVED	0

typedef void *TW_handle_t;

typedef enum TW_Event_type_t {
	TW_Event_type_timer,  // The task hasn't been commited, user can modify the
						  // task
	// File related event
	TW_Event_type_file,
	TW_Event_type_socket,  // Internet socket related events
	TW_Event_type_poll,
	/*
#ifdef HAVE_MPI
	TW_Event_type_mpi,	// MPI related event
#endif
*/
} TW_Event_type_t;

typedef struct TW_File_event_args_t {
	TW_Fd_t fd;
	int events;
} TW_File_event_args_t;
typedef struct TW_Socket_event_args_t {
	TW_Socket_t socket;
	int events;
} TW_Socket_event_args_t;

typedef struct TW_Timer_event_args_t {
	int64_t micro_sec;
	int repeat_count;
} TW_Timer_event_args_t;

typedef struct TW_Poll_event_args_t {
	TW_Event_poll_handler_t poll;
	void *data;
	// void *init_data;
} TW_Poll_event_args_t;

/*
#ifdef HAVE_MPI
typedef struct TW_Mpi_event_args_t {
	MPI_Request req;
	int flag;
	MPI_Status stat;
} TW_Mpi_event_args_t;
#endif
*/

typedef struct TW_Event_args_imp_t {
	TW_Event_type_t type;
	union args {
		TW_File_event_args_t file;
		TW_Socket_event_args_t socket;
		TW_Timer_event_args_t timer;
		TW_Poll_event_args_t poll;
	} args;
} TW_Event_args_imp_t;