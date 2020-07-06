/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Event APIs */

#pragma once

#ifdef _WIN32
#include "windows.h"
#else
#include "unistd.h"
#endif

#include "taskworks.h"
#include "tw_task.h"

#ifdef _WIN32
#define TW_Event_socket_t HANDLE
#else
#define TW_Event_socket_t int
#endif

typedef enum TW_Event_type_t {
	TW_Event_type_time,	 // The task hasn't been commited, user can modify the
						 // task
	// File related event
	TW_Event_type_file,
	TW_Event_type_file_open,

	TW_Event_type_socket,  // Internet socket related events
	TW_Event_type_task,	   // Task related event
	TW_Event_type_mpi,	   // MPI related event
} TW_Event_type_t;

typedef struct TW_Event_t *TW_Event_handle_t;
typedef struct TW_Event_attr_t {
	TW_Event_type_t type;
	union evt_data {
		int fd;
		int socket;
	};
} TW_Event_attr_t;

typedef terr_t (*TW_Event_handler_t) (TW_Event_handle_t hevt, void *data);

typedef enum TW_Event_socket_event_type { TW_Event_socket_event_data } TW_Event_socket_event_type;

// Create, free
extern terr_t TW_Event_create_task (TW_Event_attr_t attr,
									TW_Event_handler_t evt_cb,
									void *evt_data,
									TW_Event_handle_t *hevt);  // Create a new event
extern terr_t TW_Event_free (TW_Event_handle_t hevt);

// Control
extern terr_t TW_Event_commit (TW_Engine_handle_t heng,
							   TW_Event_handle_t hevt);	  // Commit event, start watching
extern terr_t TW_Event_retract (TW_Event_handle_t hevt);  // Stop watching