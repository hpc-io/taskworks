/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @file
 * @brief  Engine related APIs
 */

/// \cond
// Prevent doxygen from leaking our internal headers
#include "dispatcher.h"
/// \endcond

TW_Event_poll_handler_t TW_Event_poll_mpi;
TW_Event_poll_handler_t TW_Event_poll_mpi_req;

TW_Event_poll_response_t TWI_Event_poll_mpi (void *data) {
	int mpierr;
	TW_Event_poll_mpi_data *dp = (TW_Event_poll_mpi_data *)data;

	mpierr = MPI_Iprobe (dp->src, dp->tag, dp->comm, &(dp->flag), &(dp->status));
	if (mpierr != MPI_SUCCESS) return TW_Event_poll_response_err;

	if (dp->flag) { return TW_Event_poll_response_trigger; }

	return TW_Event_poll_response_pending;
}

TW_Event_poll_response_t TWI_Event_poll_mpi_req (void *data) {
	int mpierr;
	TW_Event_poll_mpi_req_data *dp = (TW_Event_poll_mpi_req_data *)data;

	mpierr = MPI_Test (&(dp->req), &(dp->flag), &(dp->status));
	if (mpierr != MPI_SUCCESS) return TW_Event_poll_response_err;

	if (dp->flag) { return TW_Event_poll_response_trigger; }

	return TW_Event_poll_response_pending;
}

terr_t TW_Event_poll_mpi_data_create (MPI_Comm comm, int src, int tag, void **data) {
	terr_t err = TW_SUCCESS;
	TW_Event_poll_mpi_data *dp;

	dp = (TW_Event_poll_mpi_data *)TWI_Malloc (sizeof (TW_Event_poll_mpi_data));
	CHECK_PTR (dp);

	dp->comm = comm;
	dp->src	 = src;
	dp->tag	 = tag;

	*data = dp;

err_out:;
	return err;
}

terr_t TW_Event_poll_mpi_req_data_create (MPI_Request req, void **data) {
	terr_t err = TW_SUCCESS;
	TW_Event_poll_mpi_req_data *dp;

	dp = (TW_Event_poll_mpi_req_data *)TWI_Malloc (sizeof (TW_Event_poll_mpi_req_data));
	CHECK_PTR (dp);

	dp->req = req;

	*data = dp;

err_out:;
	return err;
}

/*
int TWI_Event_poll_mpi_init (void *in, void **data);
int TWI_Event_poll_mpi_check (void *data);
int TWI_Event_poll_mpi_finalize (void *data);

int TWI_Event_poll_mpi_req_init (void *in, void **data);
int TWI_Event_poll_mpi_req_check (void *data);
int TWI_Event_poll_mpi_req_finalize (void *data);

TW_Event_poll_handler_t TWI_Event_poll_mpi_req;
TW_Event_poll_handler_t TWI_Event_poll_mpi_req = {TWI_Event_poll_mpi_req_init,
												  TWI_Event_poll_mpi_req_finalize,
												  TWI_Event_poll_mpi_req_check, NULL};
TW_Event_poll_handler_t TWI_Event_poll_mpi;
TW_Event_poll_handler_t TWI_Event_poll_mpi = {TWI_Event_poll_mpi_init, TWI_Event_poll_mpi_finalize,
											  TWI_Event_poll_mpi_check, NULL};

int TWI_Event_poll_mpi_init (void *in, void **data) {
	TW_Event_poll_mpi_data *dp;

	dp = (TW_Event_poll_mpi_data *)malloc (sizeof (TW_Event_poll_mpi_data));
	if (!dp) return -1;

	*dp	  = *((TW_Event_poll_mpi_data *)in);
	*data = dp;

	return 0;
}
int TWI_Event_poll_mpi_check (void *data) {
	int mpierr;
	TW_Event_poll_mpi_data *dp = (TW_Event_poll_mpi_data *)data;

	mpierr = MPI_Iprobe (dp->src, dp->tag, dp->comm, &(dp->flag), &(dp->status));
	if (mpierr != MPI_SUCCESS) return -1;

	if (dp->flag) { return 1; }

	return 0;
}
int TWI_Event_poll_mpi_finalize (void *data) { free (data); }

int TWI_Event_poll_mpi_req_init (void *in, void **data) {
	TW_Event_poll_mpi_req_data *dp;

	dp = (TW_Event_poll_mpi_req_data *)malloc (sizeof (TW_Event_poll_mpi_req_data));
	if (!dp) return -1;

	*dp	  = *((TW_Event_poll_mpi_req_data *)in);
	*data = dp;

	return 0;
}
int TWI_Event_poll_mpi_req_check (void *data) {
	int mpierr;
	TW_Event_poll_mpi_req_data *dp = (TW_Event_poll_mpi_req_data *)data;

	mpierr = MPI_Test (&(dp->req), &(dp->flag), &(dp->status));
	if (mpierr != MPI_SUCCESS) return -1;

	if (dp->flag) { return 1; }

	return 0;
}
int TWI_Event_poll_mpi_req_finalize (void *data) {
	free (data);
	return 0;
}
*/