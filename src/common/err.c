/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Internal error handling functions */

/// \cond
// Prevent doxygen from leaking our internal headers
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#endif

#include "dispatcher.h"
/// \endcond

static char oserr[1024] = "Operating system error:";

#ifdef _WIN32
#else
#endif

const char *TWI_get_os_err_msg (terr_t err) {
	switch (err) {
		/* No Err */
		case TW_SUCCESS:
			return "Operation finished successfully";
		/* Common */
		case TW_ERR_MEM:
			return "Memory allocation fail";
		case TW_ERR_OS:
#ifdef _WIN32
		{
			LPVOID msg;
			DWORD errno;

			errno = GetLastError ();

			FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
							   FORMAT_MESSAGE_IGNORE_INSERTS,
						   NULL, errno, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0,
						   NULL);
			sprintf (oserr, "Win32 error: %s", msg);
		}
#else
			sprintf (oserr, "POSIX error: %s\n", strerror (errno));
#endif

			return "System call fail";
		/* User related */
		case TW_ERR_INVAL_BACKEND:
			return "Unrecognized engine backend";
		case TW_ERR_INVAL_EVT_BACKEND:
			return "Unrecognized event backend";
		case TW_ERR_INVAL_HANDLE:
			return "Invalid handle";
		case TW_ERR_INCOMPATIBLE_OBJECT:
			return "Objected not compatible with the current backend backend";
		/* Engine related */
		case TW_ERR_THREAD_CREATE:
			return "Cannot create thread";
		case TW_ERR_THREAD_SIG:
			return "Cannot send signal to thread";
		case TW_ERR_NOT_SUPPORTED:
			return "The backend does not support such function";
	}

	return "Unknown error";
}