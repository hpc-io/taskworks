/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2020, Lawrence Berkeley National Laboratory.                *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Taskworks. The full Taskworks copyright notice,      *
 * including terms governing use, modification, and redistribution, is       *
 * contained in the file COPYING at the root of the source code distribution *
 * tree.                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Test task APIs */

#include <errno.h>
#include <stdatomic.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <twtest.h>

#define NUM_WORKERS 4

#define PORT	16385
#define BUFSIZE 2048

int create_receiver (int *nerrp, TW_Socket_t *fd) {
	int nerr = 0;
	struct sockaddr_in addr;

#ifdef _WIN32
	if ((*fd = socket (AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		RAISE_ERR ("cannot create socket")
		goto fn_exit;
	}

	// Prepare the sockaddr_in structure
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port		 = htons (PORT);

	// Bind
	if (bind (*fd, (struct sockaddr *)&addr, sizeof (addr)) == SOCKET_ERROR) {
		RAISE_ERR ("bind failed");
		goto fn_exit;
	}

#else
	*fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (*fd < 0) {
		RAISE_ERR ("cannot create socket")
		goto fn_exit;
	}

	memset ((char *)&addr, 0, sizeof (addr));
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = htonl (INADDR_ANY);
	addr.sin_port		 = htons (PORT);

	if (bind (*fd, (struct sockaddr *)&addr, sizeof (addr)) < 0) {
		RAISE_ERR ("bind failed");
		goto fn_exit;
	}
#endif

fn_exit:;
	*nerrp += nerr;
	return nerr > 0;
}

int send_msg (int *nerrp, char *msg) {
	int nerr = 0;
	TW_Socket_t fd;
	struct sockaddr_in addr;

#ifdef _WIN32
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		RAISE_ERR ("cannot create socket")
		goto fn_exit;
	}
	// Prepare the sockaddr_in structure
	memset ((char *)&addr, 0, sizeof (addr));
	addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = inet_addr ("127.0.0.1");
	addr.sin_port		 = htons (PORT);

	// now reply the client with the same data
	if (sendto (fd, msg, strlen (msg), 0, (struct sockaddr *)&addr, sizeof (addr)) ==
		SOCKET_ERROR) {
		RAISE_ERR ("sendto failed")
		goto fn_exit;
	}
#else
	fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		RAISE_ERR ("cannot create socket")
		goto fn_exit;
	}
	char *my_messsage = "this is a test message";

	/* fill in the server's address and data */
	memset ((char *)&addr, 0, sizeof (addr));
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons (PORT);
	addr.sin_addr.s_addr = inet_addr ("127.0.0.1");

	if (sendto (fd, msg, strlen (msg), 0, (struct sockaddr *)&addr, sizeof (addr)) < 0) {
		RAISE_ERR ("sendto failed")
		goto fn_exit;
	}
#endif

	close (fd);

fn_exit:;
	*nerrp += nerr;
	return nerr > 0;
}

TWT_Semaphore sem;
char buf[255];
char *bufp = buf;
int event_cb (TW_Event_handle_t __attribute__ ((unused)) evt, TW_Event_args_t *arg, void *data) {
	terr_t err;
	int nerr   = 0;
	int *nerrp = (int *)data;
	int len;
#ifdef _WIN32
	DWORD dwBytesRead;
#else
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof (addr);
#endif

	memset (buf, 0, sizeof (buf));

#ifdef _WIN32
	if ((len = recvfrom (arg->args.socket.socket, bufp, sizeof (buf) - 1 - (size_t) (bufp - buf), 0,
						 (struct sockaddr *)&addr, &addrlen)) == SOCKET_ERROR) {
		RAISE_ERR ("recvfrom failed");
		goto fn_exit;
	}

	err = TW_Event_retract (evt);
	CHECK_ERR
	err = TWT_Sem_inc (sem);
	CHECK_ERR
#else
	len = recvfrom (arg->args.socket.socket, bufp, sizeof (buf) - 1 - (size_t) (bufp - buf), 0,
					(struct sockaddr *)&addr, &addrlen);
	if (len > 0) {
		bufp += len;
		if (bufp - buf >= 8) {
			err = TW_Event_retract (evt);
			CHECK_ERR
			err = TWT_Sem_inc (sem);
			CHECK_ERR
		}
	} else if (len < 0) {
		RAISE_ERR ("recvfrom failed");
	}
#endif

fn_exit:;
	*nerrp += nerr;

	return 0;
}

int main (int argc, char **argv) {
	terr_t err = TW_SUCCESS;
	int nerr   = 0;
	int ret;
	int nworker		   = NUM_WORKERS;
	atomic_int evtnerr = 0;
	TW_Socket_t frecv, fsend;
	char msg[] = "test_msg";
	TW_Event_args_t arg;
	TW_Event_handle_t evt;
	TW_Engine_handle_t eng;
#ifdef _WIN32
	WSADATA wsa;
#endif

	PRINT_TEST_MSG ("Check if file event triggers correctly");

	if (argc > 1) { nworker = atoi (argv[1]); }

#ifdef _WIN32
	if (WSAStartup (MAKEWORD (2, 2), &wsa) != 0) {
		RAISE_ERR ("WSAStartup failed");
		goto fn_exit;
	}
#endif

	err = TWT_Sem_create (&sem);
	CHECK_ERR

	ret = create_receiver (&nerr, &frecv);
	if (ret != 0) goto fn_exit;

	err = TW_Init (TW_Backend_argobots, TW_Event_backend_libevent, &argc, &argv);
	CHECK_ERR

	err = TW_Engine_create (nworker, &eng);
	CHECK_ERR

	err = TW_Event_arg_set_socket (&arg, frecv, TW_EVENT_SOCKET_READY_FOR_READ);
	CHECK_ERR
	err = TW_Event_create (event_cb, &evtnerr, arg, &evt);
	CHECK_ERR
	err = TW_Event_commit (evt, eng);
	CHECK_ERR

	ret = send_msg (&nerr, msg);

	err = TWT_Sem_dec (sem);
	CHECK_ERR
	err = TWT_Sem_free (sem);
	CHECK_ERR

	nerr += evtnerr;

	// Avoid comparing '\n' by setting len as strlen(msg)
	ret = strncmp (buf, msg, strlen (msg));
	EXP_VAL (ret, 0, "%d");

	err = TW_Engine_free (eng);
	CHECK_ERR

	err = TW_Finalize ();
	CHECK_ERR

#ifdef _WIN32
	WSACleanup ();
	closesocket (frecv);
#else
	close (frecv);
#endif

fn_exit:;
	PRINT_TEST_RESULT
	return nerr;
}
