#include "twtest.h"

terr_t TWT_Sem_create (TWT_Semaphore *sem) {
	int ret;
	TWT_Semaphore s;

	s = (TWT_Semaphore)malloc (sizeof (sem_t));

	//ret = sem_init (s, 0, 0);
    sem_t *semaphore = (sem_t *) malloc(sizeof(sem_t));
    s = sem_open("/semaphore", O_CREAT, 0644, 1);
    sem_unlink("/semaphore");
	if(s == SEM_FAILED)
		ret = -1;

	if (ret != 0) return TW_ERR_OS;

	*sem = s;

	return TW_SUCCESS;
}

terr_t TWT_Sem_dec (TWT_Semaphore sem) {
	int ret;

	ret = sem_wait (sem);
	printf("%s:%d: ret = %d, Errno str = %s\n", __func__, __LINE__, ret, strerror(errno));
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

	ret = sem_close(sem); //sem_destroy (sem);
	if (ret != 0) return TW_ERR_OS;

	//free (sem);

	return TW_SUCCESS;
}
