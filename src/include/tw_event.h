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

#include "taskworks.h"
#include "tw_task.h"

#ifdef _WIN32
#define TW_Socket_t HANDLE
#define TW_Fd_t		HANDLE
#else
#define TW_Socket_t int
#define TW_Fd_t		int
#endif

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

typedef enum TW_Event_type_t {
	TW_Event_type_timer,  // The task hasn't been commited, user can modify the
						  // task
	// File related event
	TW_Event_type_file,
	TW_Event_type_socket,  // Internet socket related events
	TW_Event_type_task,	   // Task related event
	TW_Event_type_mpi,	   // MPI related event
} TW_Event_type_t;

typedef struct TW_File_event_args_t {
	TW_Fd_t fd;
	int events;
} TW_File_event_args_t;
typedef struct TW_Socket_event_args_t {
	TW_Socket_t socket;
	int events;
} TW_Socket_event_args_t;
// typedef struct TW_MPI_event_args_t {
//} TW_MPI_event_args_t;
typedef struct TW_Task_event_args_t {
	TW_Task_handle_t task;
	int status;
} TW_Task_event_args_t;
typedef struct TW_Timer_event_args_t {
	int64_t micro_sec;
	int repeat_count;
} TW_Timer_event_args_t;

typedef struct TW_Event_args_t {
	TW_Event_type_t type;
	union args {
		TW_File_event_args_t file;
		TW_Socket_event_args_t socket;
		// TW_MPI_event_args_t socket;
		TW_Task_event_args_t task;
		TW_Timer_event_args_t timer;
	} args;
} TW_Event_args_t;

typedef TW_Event_args_t *TW_Event_args_handle_t;

typedef struct TW_Obj_t *TW_Event_handle_t;

typedef int (*TW_Event_handler_t) (TW_Event_handle_t evt, TW_Event_args_t *arg, void *data);

typedef enum TW_Event_socket_event_type { TW_Event_socket_event_data } TW_Event_socket_event_type;

// Set event arg
extern terr_t TW_Event_arg_set_file (TW_Event_args_handle_t harg, TW_Fd_t fd, int events);
extern terr_t TW_Event_arg_set_socket (TW_Event_args_handle_t harg, TW_Socket_t socket, int events);
extern terr_t TW_Event_arg_set_timer (TW_Event_args_handle_t harg,
									  int64_t micro_sec,
									  int repeat_count);
extern terr_t TW_Event_arg_set_task (TW_Event_args_handle_t harg,
									 TW_Task_handle_t task,
									 int status);

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