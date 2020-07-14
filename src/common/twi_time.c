/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,
 *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Monotonic time wrapper implementations */

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
/* Offset beTW_Engine_en 1/1/1601 and 1/1/1970 in microsecond units */
#define _W32_FT_OFFSET (11644473600000000LL)
#else
#include <sys/time.h>
#endif

#include "taskworks_internal.h"

#ifdef _WIN32
ttime_t TWI_Time_now () {
	FILETIME ftnow;
	ttime_t tnow;

	GetSystemTimeAsFileTime (&tnow);

	tnow = (((ttime_t)ftnow.dwHighDateTime) << 32) | ((ttime_t)ftnow.dwLowDateTime));

	return tnow / 10 - _W32_FT_OFFSET;
}
#else
ttime_t TWI_Time_now () {
	int ret;
	struct timeval tnow;

	ret = gettimeofday (&tnow, NULL);
	assert (ret == 0);

	return (ttime_t)tnow.tv_sec * 1000000 + (ttime_t)tnow.tv_usec;
}
#endif

// TODO: Implement timer