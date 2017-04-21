#include "musl-sem.h"

#include "internal/atomic.h"
#include "internal/sem-limits.h"
#include "internal/threading.h"

int musl_sem_post(musl_sem_t *sem)
{
	int val, waiters, priv = sem->__val[2];
	do {
		val = sem->__val[0];
		waiters = sem->__val[1];
		if (val == MUSL_SEM_VALUE_MAX) {
			errno = EOVERFLOW;
			return -1;
		}
	} while (a_cas(sem->__val, val, val+1+(val<0)) != val);
	if (val<0 || waiters) __wake(sem->__val, 1, priv);
	return 0;
}
