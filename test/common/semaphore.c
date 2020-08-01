#include "twtest.h"

terr_t TWT_Sem_create (TWT_Semaphore *sem) {
	int ret;
	TWT_Semaphore s;

	s = (TWT_Semaphore)malloc (sizeof (sem_t));

	ret = sem_init (s, 0, 0);
	if (ret != 0) return TW_ERR_OS;

	*sem = s;

	return TW_SUCCESS;
}

terr_t TWT_Sem_dec (TWT_Semaphore sem) {
	int ret;

	ret = sem_wait (sem);
	if (ret != 0) return TW_ERR_OS;

	return TW_SUCCESS;
}

terr_t TWT_Sem_inc (TWT_Semaphore sem) {
	int ret;

	ret = sem_post (sem);
	if (ret != 0) return TW_ERR_OS;

	return TW_SUCCESS;
}

terr_t TWT_Sem_free (TWT_Semaphore sem) {
	int ret;

	ret = sem_destroy (sem);
	if (ret != 0) return TW_ERR_OS;

	free (sem);

	return TW_SUCCESS;
}