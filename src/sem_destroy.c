#include "musl-sem.h"

int musl_sem_destroy(musl_sem_t *sem)
{
	(void) sem;
	return 0;
}
