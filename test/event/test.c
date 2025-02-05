/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Test engine APIs */

#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <signal.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <twtest.h>

int event_cb (TW_Event_handle_t evt, TW_Event_args_imp_t *arg, void *data) {
	terr_t err;
	int nerr   = 0;
	int *nerrp = (int *)data;
	char buf[255];
	int len;
#ifdef _WIN32
	DWORD dwBytesRead;
#endif

	return 0;

#ifdef _WIN32
	len = ReadFile ((HANDLE)arg->args.file.fd, buf, sizeof (buf) - 1, &dwBytesRead, NULL);

	/* Check for end of file. */
	if (len && dwBytesRead == 0) {
		err = TW_Event_retract (evt);
		CHECK_ERR
		goto end;
	}

	buf[dwBytesRead] = '\0';
#else
	len = read (arg->args.file.fd, buf, sizeof (buf) - 1);

	if (len <= 0) {
		EXP_VAL (len, 0, "%d")
		err = TW_Event_retract (evt);
		CHECK_ERR
		goto end;
	}

	buf[len] = '\0';
#endif
	fprintf (stdout, "Read: %s\n", buf);

end:;
	*nerrp += nerr;
	return 0;
}

int open_file (char *path, TW_Fd_t *fd) {
#ifdef _WIN32
	HANDLE socket;
	/* Open a file. */
	socket = CreateFileA (path,					 /* open File */
						  GENERIC_READ,			 /* open for reading */
						  0,					 /* do not share */
						  NULL,					 /* no security */
						  OPEN_EXISTING,		 /* existing file only */
						  FILE_ATTRIBUTE_NORMAL, /* normal file */
						  NULL);				 /* no attr. template */

	if (socket == INVALID_HANDLE_VALUE) return -1;

#else
	struct event *signal_int;
	struct stat st;
	const char *fifo = path;
	int socket;

#if defined __USE_MISC || defined __USE_XOPEN
	if (lstat (fifo, &st) == 0) {
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			errno = EEXIST;
			perror ("lstat");
			return -1;
		}
	}
#endif

	unlink (fifo);
	if (mkfifo (fifo, 0600) == -1) {
		perror ("mkfifo");
		return -1;
	}

	socket = open (fifo, O_RDONLY | O_NONBLOCK, 0);

	if (socket == -1) {
		perror ("open");
		return -1;
	}

	fprintf (stderr, "Write data to %s\n", fifo);
#endif

	*fd = socket;

	return 0;
}

int main (int argc, char **argv) {
	terr_t err		   = TW_SUCCESS;
	int nerr		   = 0;
	atomic_int evtnerr = 0;
	int ret;
	TW_Fd_t fd;
	TW_Event_args_imp_t arg;
	TW_Event_handle_t evt;
	TW_Engine_handle_t eng;

	PRINT_TEST_MSG ("Check if TaskWork can create and free engines");

	ret = open_file ("file.txt", &fd);
	EXP_VAL (ret, 0, "%d");

	err = TW_Init (TW_Backend_argobots, TW_Event_backend_libevent, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (4, &eng);
	CHECK_ERR

	err = TW_Event_arg_set_file (&arg, fd, TW_EVENT_FILE_READY_FOR_READ);
	CHECK_ERR
	err = TW_Event_create (event_cb, &evtnerr, arg, &evt);
	CHECK_ERR
	err = TW_Event_commit (evt, eng);
	CHECK_ERR

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

	PRINT_TEST_RESULT
	return nerr;
}
