/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of TaskEngine. The full TaskEngine copyright notice,    *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Usser level common API */

#include "taskworks_internal.h"

/**
 * @brief  Initialize the TaskWorks library.
 * @note The library may remove TaskWorks related command line arguments.
 * @param  *argc: Command line argument count
 * @param  ***argv: Command line arguments
 * @retval TW_ERR_SUCCESS on success or error code on failure
 */
terr_t TW_Init (int *argc, char ***argv) {
	terr_t err = TW_ERR_SUCCESS;
	return err;
}

/**
 * @brief  Finalize the TaskWorks library.
 * @note   All resource will be freed. All TaskWorks objects will be closed.
 * @retval TW_ERR_SUCCESS on success or error code on failure
 */
terr_t TW_Finalize () {
	terr_t err = TW_ERR_SUCCESS;
	return err;
}
