#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <time.h>

#ifdef MUSL_SEM_BUILD
#  define MUSL_SEM_EXPORT __attribute__((visibility ("default")))
#else
#  define MUSL_SEM_EXPORT
#endif

#define MUSL_SEM_FAILED ((musl_sem_t *)0)

typedef struct {
	volatile int __val[4*sizeof(long)/sizeof(int)];
} musl_sem_t;

MUSL_SEM_EXPORT int         musl_sem_close(musl_sem_t *);
MUSL_SEM_EXPORT int         musl_sem_destroy(musl_sem_t *);
MUSL_SEM_EXPORT int         musl_sem_getvalue(musl_sem_t *__restrict, int *__restrict);
MUSL_SEM_EXPORT int         musl_sem_init(musl_sem_t *, int, unsigned);
MUSL_SEM_EXPORT musl_sem_t *musl_sem_open(const char *, int, ...);
MUSL_SEM_EXPORT int         musl_sem_post(musl_sem_t *);
MUSL_SEM_EXPORT int         musl_sem_timedwait(musl_sem_t *__restrict, const struct timespec *__restrict);
MUSL_SEM_EXPORT int         musl_sem_trywait(musl_sem_t *);
MUSL_SEM_EXPORT int         musl_sem_unlink(const char *);
MUSL_SEM_EXPORT int         musl_sem_wait(musl_sem_t *);

#ifdef __cplusplus
}
#endif
