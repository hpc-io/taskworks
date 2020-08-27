/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Argobot driver functions */

#pragma once

#include <time.h>
// Time.h must go first to eliminate abt.h warning

//
#define _BSD_SOURCE
#define __USE_MISC
#include <sys/time.h>

// libevent headers
#include <event2/event.h>
#include <event2/util.h>

#include "eal.h"
#include "taskworks_internal.h"

#ifdef TWI_DEBUG

#else
#endif

#define CHECK_LIBEVTPTR(R)                                                                   \
	{                                                                                        \
		if (!(R)) {                                                                          \
			err = -1;                                                                        \
			printf ("Error at line %d in %s: fail to initizlie libevent object\n", __LINE__, \
					__FILE__);                                                               \
			goto err_out;                                                                    \
		}                                                                                    \
	}

#define CHECK_LIBEVTRET(R)                                             \
	{                                                                  \
		if (R != 0) {                                                  \
			err = TWLIBEVT_Err_to_tw_err (R);                          \
			printf ("Error at line %d in %s: \n", __LINE__, __FILE__); \
			goto err_out;                                              \
		}                                                              \
	}

#define CHECK_LIBEVTERR CHECK_LIBEVTRET (evterr)

#define EVT_STATUS_PENDING	0
#define EVT_STATUS_COMMITED 1
#define EVT_STATUS_RUNNING	2

typedef struct TWLIBEVT_Loop_t {
	struct event_base *base;
	TWI_Ts_vector_handle_t poll_events;	 // events that requires polling
} TWLIBEVT_Loop_t;

typedef struct TWLIBEVT_Event_t {
	TW_Event_args_imp_t args;
	TW_Event_driver_handler_t handler;
	void *data;
	struct event *event;
	TWLIBEVT_Loop_t *lp;
	OPA_int_t status;
	OPA_ptr_t nlp;	// Pending commits, we allow one more commit while event is still running so the
					// handler can recommit the event
} TWLIBEVT_Event_t;

extern TWI_Ts_vector_handle_t TWLIBEVTI_Loops;
extern TWI_Ts_vector_handle_t TWLIBEVTI_Events;

/* Init callbacks */
terr_t TWLIBEVT_Init (int *argc, char ***argv);
terr_t TWLIBEVT_Finalize (void);

/* Loop callbacks */
terr_t TWLIBEVT_Loop_create (TW_Handle_t *loop);  // Initialize an event loop
terr_t TWLIBEVT_Loop_free (TW_Handle_t engine);	  // Finalize the event loop
terr_t TWLIBEVT_Loop_check_events (TW_Handle_t engine,
								   ttime_t timeout);  // Check for a single event

/* Event callbacks */
terr_t TWLIBEVT_Event_create (TW_Event_driver_handler_t evt_cb,
							  void *evt_data,
							  TW_Event_args_imp_t arg,
							  TW_Handle_t *event);					 // Create a new event
terr_t TWLIBEVT_Event_free (TW_Handle_t event);						 // Free up an event
terr_t TWLIBEVT_Event_commit (TW_Handle_t event, TW_Handle_t loop);	 // Commit an event
terr_t TWLIBEVT_Event_retract (TW_Handle_t htask);					 // Remove event from the loop

/* Loop internal functions */
terr_t TWLIBEVTI_Check_for_single_unmanaged_event (TWLIBEVT_Loop_t *lp, TWI_Bool_t *success);
terr_t TWLIBEVTI_Check_for_single_event (TW_Handle_t loop, TWI_Bool_t *success);

/* Event internal functions */
void TWLIBEVTI_Evt_file_cb (evutil_socket_t socket, short flags, void *data);
void TWLIBEVTI_Evt_socket_cb (evutil_socket_t socket, short flags, void *data);
void TWLIBEVTI_Evt_timer_cb (evutil_socket_t socket, short flags, void *data);
void TWLIBEVTI_Evt_mpi_cb (TWLIBEVT_Event_t *ep, int flag, MPI_Status stat);
void TWLIBEVTI_Evt_poll_cb (TWLIBEVT_Event_t *ep);

/* Misc */
terr_t TWLIBEVT_Err_to_tw_err (int TWI_UNUSED abterr);