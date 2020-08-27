/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
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

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#include "taskworks.h"
#include "tw_task.h"

#ifdef _WIN32
#define TW_Socket_t HANDLE
#define TW_Fd_t		HANDLE
#else
#define TW_Socket_t int
#define TW_Fd_t		int
#endif

/* Event status
 * We do not use enum because it is not supported by openpa
 */
#define TW_EVENT_STAT_INVAL	   0x0
#define TW_EVENT_STAT_IDLE	   0x1	 // The event hasn't been commited, user can modify the event
#define TW_EVENT_STAT_WATCHING 0x2	 // Commiting to the engine, watching for events
#define TW_EVENT_STAT_TRIGGER  0x4	 // Even triggered, preparing to run
#define TW_EVENT_STAT_RUNNING  0x10	 // Event handler running
//#define TW_EVENT_STAT_FINAL		0x20  // Finalizing event handler run
//#define TW_EVENT_STAT_COMPLETED 0x40   // The event handler is completed
#define TW_EVENT_STAT_FAILED 0x80	// The event handler returns non-zero
#define TW_EVENT_STAT_TRANS	 0x100	// The event handler returns non-zero
#define TW_EVENT_STAT_ANY	 0x1FF	// The event handler returns non-zero

#define TW_EVENT_FILE_READ			  0x1
#define TW_EVENT_FILE_WRITE			  0x2
#define TW_EVENT_FILE_READY_FOR_READ  0x4
#define TW_EVENT_FILE_READY_FOR_WRITE 0x8
#define TW_EVENT_FILE_ALL			  0xF

#define TW_EVENT_SOCKET_READ			0x1
#define TW_EVENT_SOCKET_WRITE			0x2
#define TW_EVENT_SOCKET_READY_FOR_READ	0x4
#define TW_EVENT_SOCKET_READY_FOR_WRITE 0x8
#define TW_EVENT_SOCKET_ALL				0xF

typedef struct TW_Event_args_t {
	char data[32];
} TW_Event_args_t;

typedef TW_Event_args_t *TW_Event_args_handle_t;

typedef struct TW_Obj_t *TW_Event_handle_t;

typedef int (*TW_Event_handler_t) (TW_Event_handle_t evt, TW_Event_args_t *arg, void *data);
typedef int (*TW_Event_poll_t) (void *data);

// Set event arg
extern terr_t TW_Event_arg_set_file (TW_Event_args_handle_t harg, TW_Fd_t fd, int events);
extern terr_t TW_Event_arg_set_socket (TW_Event_args_handle_t harg, TW_Socket_t socket, int events);
extern terr_t TW_Event_arg_set_timer (TW_Event_args_handle_t harg,
									  int64_t micro_sec,
									  int repeat_count);
extern terr_t TW_Event_arg_set_poll (TW_Event_args_handle_t harg,
									 TW_Event_poll_t poll_fn,
									 void *data);
extern terr_t TW_Event_arg_get_file (TW_Event_args_handle_t harg, TW_Fd_t *fd, int *events);
extern terr_t TW_Event_arg_get_socket (TW_Event_args_handle_t harg,
									   TW_Socket_t *socket,
									   int *events);
extern terr_t TW_Event_arg_get_timer (TW_Event_args_handle_t harg,
									  int64_t *micro_sec,
									  int *repeat_count);
extern terr_t TW_Event_arg_get_poll (TW_Event_args_handle_t harg,
									 TW_Event_poll_t *poll_fn,
									 void **data);
#ifdef HAVE_MPI
extern terr_t TW_Event_arg_set_mpi (TW_Event_args_handle_t harg, MPI_Request req);
extern terr_t TW_Event_arg_get_mpi (TW_Event_args_handle_t harg, MPI_Request *req);
#endif

// Create, free
extern terr_t TW_Event_create (TW_Event_handler_t evt_cb,
							   void *evt_data,
							   TW_Event_args_t arg,
							   TW_Event_handle_t *event);  // Create a new event
extern terr_t TW_Event_free (TW_Event_handle_t event);

// Control
extern terr_t TW_Event_commit (TW_Event_handle_t event,
							   TW_Engine_handle_t engine);	// Commit event, start watching
extern terr_t TW_Event_retract (TW_Event_handle_t event);	// Stop watching

// Misc
extern const char *TW_Event_status_str (int status);