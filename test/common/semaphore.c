#include "twtest.h"

terr_t TWT_Sem_create (TWT_Semaphore *sem) {
	int ret;
	TWT_Semaphore s;
	s = (TWT_Semaphore)malloc (sizeof (sem_t));

	char sem_name[128] = {};
    sprintf(sem_name, "sem-%d", pthread_self());

    s = sem_open(sem_name, O_CREAT, 0644, 0);
    sem_unlink(sem_name);

	if(s == SEM_FAILED) ret = -1;
	else ret = 0;

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
	ret = sem_close(sem);
	if (ret != 0)
	    return TW_ERR_OS;

	return TW_SUCCESS;
}

void SEMAPHORE_BUG_WARNING(){
#ifdef __APPLE__
  assert(0 && "Taskworks has a known bug with semaphore on OSX that may lead to a deadlock, not fixed yet.");
#endif
}
