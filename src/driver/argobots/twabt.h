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

#include <abt.h>

#include "taskworks_internal.h"

#ifdef ENABLE_DEBUG
#define TWABTI_PRINT_ERR(E)                                                                      \
	{                                                                                            \
		char errmsg[256];                                                                        \
		size_t len;                                                                              \
		int ret;                                                                                 \
                                                                                                 \
		ret = ABT_error_get_str (E, NULL, &len);                                                 \
		if (ret == ABT_SUCCESS && len < 256) {                                                   \
			ret = ABT_error_get_str (E, errmsg, NULL);                                           \
			if (ret == ABT_SUCCESS) {                                                            \
				printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, errmsg, E);     \
			} else {                                                                             \
				printf ("Error at line %d in %s: Unknown argobots error (%d)\n", __LINE__,       \
						__FILE__, E);                                                            \
			}                                                                                    \
		} else {                                                                                 \
			printf ("Error at line %d in %s: Unknown argobots error (%d)\n", __LINE__, __FILE__, \
					E);                                                                          \
		}                                                                                        \
	}
#else
#define TWABTI_PRINT_ERR(E)
#endif

#define CHK_ABTRET(R)                      \
	{                                      \
		if (R != ABT_SUCCESS) {            \
			TWABTI_PRINT_ERR (R)           \
			err = TWABT_Err_to_tw_err (R); \
			PRINT_ERR (err)                \
			goto err_out;                  \
		}                                  \
	}

#define CHECK_ABTERR CHK_ABTRET (abterr)

typedef struct TWABT_Engine_t {
	int ness;									  // Number of threads (ES)
	int ness_alloc;								  // Size of schedulers and ess
	ABT_pool pools[TWI_TASK_NUM_PRIORITY_LEVEL];  // Task pool of th engine
	ABT_sched *schedulers;						  // Task scheduler
	ABT_xstream *ess;							  // Threads (ES)
	void *dispatcher_obj;						  // Corresponding structure at dispatcher level
	TWI_Nb_list_handle_t tasks;
	TW_Event_driver_handle_t evt_driver;
	void *evt_loop;
	TWI_Mutex_t evt_lock;
} TWABT_Engine_t;

struct TWABT_Task_monitor_t;
typedef struct TWABT_Event_t {
	TW_Event_handler_t handler;
	void *data;
	TW_Event_driver_handle_t driver;
	OPA_int_t status;  // Status of the task
	int status_flag;
	struct TWABT_Task_monitor_t *mp;
	void *driver_obj;
	void *dispatcher_obj;
	TWABT_Engine_t *eng;
	ABT_task abt_task;	// Argobot task handle
} TWABT_Event_t;

typedef struct TWABT_Task_t {
	TWI_Rwlock_t lock;
	TW_Task_handler_t handler;			// Task handler
	void *data;							// Task handler data
	TW_Task_dep_handler_t dep_handler;	// Dependency handler
	int tag;							// Tag for the user
	ABT_task abt_task;					// Argobot task handle
	OPA_int_t status;					// Status of the task
	int priority;						// Priority, currently not used
	TWI_Nb_list_handle_t parents;		// Tasks it depends on
	TWI_Nb_list_handle_t childs;		// Tasks depend on it
	TWABT_Engine_t *ep;					// Engine it is commited to
	void *dispatcher_obj;				// Corresponding structure at dispatcher level
	TWI_Nb_list_handle_t events;		// Events monitoring to its status
} TWABT_Task_t;

typedef struct TWABT_Task_dep_t {
	TWABT_Task_t *parent;  // parent task
	TWABT_Task_t *child;   // child task
	OPA_int_t status;	   // Last status of the parent known to the child
	OPA_int_t ref;		   // Reference count
} TWABT_Task_dep_t;

typedef struct TWABT_Task_monitor_t {
	TWABT_Event_t *evt;
	OPA_int_t ref;
} TWABT_Task_monitor_t;

extern TWI_Ts_vector_handle_t TWABTI_Tasks;
extern TWI_Ts_vector_handle_t TWABTI_Engines;
extern TWI_Ts_vector_handle_t TWABTI_Events;

/* Init callbacks */
terr_t TWABT_Init (int *argc, char ***argv);
terr_t TWABT_Finalize (void);

/* Engine callbacks */
terr_t TWABT_Engine_create (int num_worker,
							void *dispatcher_obj,
							TW_Handle_t *engine);  // Initialize the task engine
												   // with num_worker workers
terr_t TWABT_Engine_free (TW_Handle_t engine);	   // Finalize the task engine
terr_t TWABT_Engine_do_work (TW_Handle_t engine,
							 ttime_t timeout);	// Run a single task using the calling thread
terr_t TWABT_Engine_set_num_workers (
	TW_Handle_t engine,
	int num_worker);  // Increase the number of worker by num_worker

/* Task callbacks */
terr_t TWABT_Task_create (TW_Task_handler_t task_cb,
						  void *task_data,
						  TW_Task_dep_handler_t dep_handler,
						  int tag,
						  int priority,
						  void *dispatcher_obj,
						  TW_Handle_t *task);		   // Create a new task
terr_t TWABT_Task_free (TW_Handle_t task);			   // Free up a task
terr_t TWABT_Task_create_barrier (TW_Handle_t engine,  // Must have option of global
								  int dep_tag,
								  int tag,
								  void *dispatcher_obj,
								  TW_Handle_t *task);  // Create a new barrier task
terr_t TWABT_Task_commit (TW_Handle_t task,
						  TW_Handle_t engine);	// Put the task into the dag
terr_t TWABT_Task_retract (TW_Handle_t task);	// Remove task form the dag
terr_t TWABT_Task_wait_single (TW_Handle_t task,
							   ttime_t timeout);  // Wait for a single task to complete. The
												  // calling thread joins the worker on the
												  // job being waited and all its parents.
terr_t TWABT_Task_wait (TW_Handle_t *tasks,
						int num_tasks,
						ttime_t timeout);  // Wait for a multiple task to complete.
terr_t TWABT_Task_add_dep (TW_Handle_t child, TW_Handle_t parent);
terr_t TWABT_Task_rm_dep (TW_Handle_t child, TW_Handle_t parent);
terr_t TWABT_Task_inq (TW_Handle_t task, TW_Task_inq_type_t inqtype, void *ret);

/* Event callbacks */
terr_t TWABT_Event_create (TW_Event_handler_t evt_cb,
						   void *evt_data,
						   TW_Event_args_t args,
						   void *dispatcher_obj,
						   TW_Handle_t *hevt);	// Create a new event
terr_t TWABT_Event_free (TW_Handle_t hevt);
terr_t TWABT_Event_commit (TW_Handle_t hevt, TW_Handle_t engine);  // Commit event, start watching
terr_t TWABT_Event_retract (TW_Handle_t hevt);					   // Stop watching

/* Internal functions */
terr_t TWABTI_Engine_free (TWABT_Engine_t *engine);
terr_t TWABTI_Task_free (TWABT_Task_t *task);
// terr_t TWABTI_Event_free (TWABT_Event_t *hevt);

/* Error handling */
terr_t TWABT_Err_to_tw_err (int abterr);
/* Schedulers */
int TWABTI_Sched_init (ABT_sched sched, ABT_sched_config config);
void TWABTI_Sched_run (ABT_sched sched);
int TWABTI_Sched_finalize (ABT_sched sched);

/* Task functions */
void TWABTI_Task_abttask_cb (void *task);
terr_t TWABTI_Task_run (TWABT_Task_t *tp, TWI_Bool_t *success);
terr_t TWABTI_Sched_run_single (TWABT_Engine_t *ep, ABT_pool *pools, int *success);
terr_t TWABTI_Task_run_dep (TWABT_Task_t *tp, TWI_Bool_t *successp);
terr_t TWABTI_Task_update_status (TWABT_Task_t *tp, int old_stat, int new_stat, int *success);
terr_t TWABTI_Task_queue (TWABT_Task_t *tp);
terr_t TWABTI_Task_notify_parent_status (TWABT_Task_t *tp, int old_stat, int new_stat);

/* Event internal functions */
terr_t TWABTI_Event_cb (TW_Event_args_t *arg, void *data);