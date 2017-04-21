#include "musl-sem.h"

int musl_sem_getvalue(musl_sem_t *restrict sem, int *restrict valp)
{
	int val = sem->__val[0];
	*valp = val < 0 ? 0 : val;
	return 0;
}
