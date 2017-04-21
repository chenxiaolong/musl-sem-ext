#pragma once

#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <sys/syscall.h>

void __lock(volatile int *);
void __unlock(volatile int *);
#define LOCK(x) __lock(x)
#define UNLOCK(x) __unlock(x)

int __timedwait(volatile int *, int, clockid_t, const struct timespec *, int);
int __timedwait_cp(volatile int *, int, clockid_t, const struct timespec *, int);
void __wait(volatile int *, volatile int *, int, int);
static inline void __wake(volatile void *addr, int cnt, int priv)
{
	if (priv) priv = 128;
	if (cnt<0) cnt = INT_MAX;
	syscall(SYS_futex, addr, FUTEX_WAKE|priv, cnt) != -ENOSYS ||
	syscall(SYS_futex, addr, FUTEX_WAKE, cnt);
}
