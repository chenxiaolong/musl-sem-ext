#include "musl-sem.h"

int musl_sem_wait(musl_sem_t *sem)
{
	return musl_sem_timedwait(sem, 0);
}
