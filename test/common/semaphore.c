#include "twtest.h"

terr_t TWT_Sem_create (TWT_Semaphore *sem) {
	int ret;
	TWT_Semaphore s;

	s = (TWT_Semaphore)malloc (sizeof (sem_t));

    char sem_name[128] = {};
    sprintf(sem_name, "sem-%d", pthread_self());
    s = sem_open(sem_name, O_CREAT, 0644, 0);
    sem_unlink(sem_name);

    DEBUG
	if(s == SEM_FAILED)
		ret = -1;
	else
	    ret = 0;
	if (ret != 0) return TW_ERR_OS;

	*sem = s;
	SEM_VAL(s)
	return TW_SUCCESS;
}

terr_t TWT_Sem_dec (TWT_Semaphore sem) {
	int ret;
	DEBUG
    SEM_VAL(sem)
	errno = 0;

	ret = sem_wait (sem);

	SEM_VAL(sem)
	printf("%s:%d: ret = %d, Errno str = %s\n", __func__, __LINE__, ret, strerror(errno));
	if (ret != 0) return TW_ERR_OS;

	return TW_SUCCESS;
}

terr_t TWT_Sem_inc (TWT_Semaphore sem) {
	int ret;
    DEBUG
    ret = sem_post (sem);

	SEM_VAL(sem)
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

int read_sem_val(TWT_Semaphore s){
    int val;
    sem_getvalue(s, &val);
    return val;
}

