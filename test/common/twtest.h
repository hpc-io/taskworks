#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#else
#endif
#include <semaphore.h>
typedef sem_t *TWT_Semaphore;
#include "taskworks.h"

#define CHECK_RET(E)                                                                             \
	{                                                                                            \
		if (E != TW_SUCCESS) {                                                                   \
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

#define EXP_VAL(V, A, T)                                                                        \
	{                                                                                           \
		if (V != A) {                                                                           \
			printf ("Error at line %d in %s: Expect " #V " = " T ", but got " T "\n", __LINE__, \
					__FILE__, A, V);                                                            \
			nerr++;                                                                             \
		}                                                                                       \
	}

#define UNEXP_VAL(V, A, T)                                                                       \
	{                                                                                            \
		if (V == A) {                                                                            \
			printf ("Error at line %d in %s: Expect " #V " != " T ", but got " T "\n", __LINE__, \
					__FILE__, A, V);                                                             \
			nerr++;                                                                              \
		}                                                                                        \
	}

#define EXP_COND(C)                                                                              \
	{                                                                                            \
		if (!(C)) {                                                                              \
			printf ("Error at line %d in %s: Expect (" #C ") = true, but got false\n", __LINE__, \
					__FILE__);                                                                   \
			nerr++;                                                                              \
		}                                                                                        \
	}

#define RAISE_ERR(M)                                                    \
	{                                                                   \
		printf ("Error at line %d in %s: %s\n", __LINE__, __FILE__, M); \
		nerr++;                                                         \
	}

terr_t TWT_Sem_create (TWT_Semaphore *sem);
terr_t TWT_Sem_dec (TWT_Semaphore sem);
terr_t TWT_Sem_inc (TWT_Semaphore sem);
terr_t TWT_Sem_free (TWT_Semaphore sem);