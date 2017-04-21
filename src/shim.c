#include "musl-sem.h"

MUSL_SEM_EXPORT int sem_close(musl_sem_t *sem)
{
	return musl_sem_close(sem);
}

MUSL_SEM_EXPORT int sem_destroy(musl_sem_t *sem)
{
	return musl_sem_destroy(sem);
}

MUSL_SEM_EXPORT int sem_getvalue(musl_sem_t *__restrict sem, int *__restrict valp)
{
	return musl_sem_getvalue(sem, valp);
}

MUSL_SEM_EXPORT int sem_init(musl_sem_t *sem, int pshared, unsigned value)
{
	return musl_sem_init(sem, pshared, value);
}

MUSL_SEM_EXPORT musl_sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value)
{
	return musl_sem_open(name, oflag, mode, value);
}

MUSL_SEM_EXPORT int sem_post(musl_sem_t *sem)
{
	return musl_sem_post(sem);
}

MUSL_SEM_EXPORT int sem_timedwait(musl_sem_t *__restrict sem, const struct timespec *__restrict at)
{
	return musl_sem_timedwait(sem, at);
}

MUSL_SEM_EXPORT int sem_trywait(musl_sem_t *sem)
{
	return musl_sem_trywait(sem);
}

MUSL_SEM_EXPORT int sem_unlink(const char *name)
{
	return musl_sem_unlink(name);
}

MUSL_SEM_EXPORT int sem_wait(musl_sem_t *sem)
{
	return musl_sem_wait(sem);
}
