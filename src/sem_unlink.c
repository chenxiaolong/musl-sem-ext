#include "musl-sem.h"

#include <sys/mman.h>

int musl_sem_unlink(const char *name)
{
	return shm_unlink(name);
}
