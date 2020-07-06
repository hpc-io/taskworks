#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include "taskworks.h"

#define CHECK_RET(E)                                                                             \
	{                                                                                            \
		if (E != TW_ERR_SUCCESS) {                                                               \
			printf ("Error at line %d in %s: %s (%d)\n", __LINE__, __FILE__, TW_Get_err_msg (E), \
					E);                                                                          \
			nerr++;                                                                              \
		}                                                                                        \
	}

#define CHECK_ERR CHECK_RET (err)

#define PRINT_TEST_MSG(M) printf ("%s: %s ... ", basename (argv[0]), M);

#define PRINT_TEST_RESULT                           \
	{                                               \
		if (nerr)                                   \
			printf ("fail with %d errors\n", nerr); \
		else                                        \
			printf ("success\n");                   \
	}
