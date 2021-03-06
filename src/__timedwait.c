#include "internal/threading.h"

#include <pthread.h>
#include <sys/syscall.h>

int __timedwait_cp(volatile int *addr, int val,
	clockid_t clk, const struct timespec *at, int priv)
{
	int r;
	struct timespec to, *top=0;

	if (priv) priv = 128;

	if (at) {
		if (at->tv_nsec >= 1000000000UL) return EINVAL;
		if (clock_gettime(clk, &to)) return EINVAL;
		to.tv_sec = at->tv_sec - to.tv_sec;
		if ((to.tv_nsec = at->tv_nsec - to.tv_nsec) < 0) {
			to.tv_sec--;
			to.tv_nsec += 1000000000;
		}
		if (to.tv_sec < 0) return ETIMEDOUT;
		top = &to;
	}

	r = -syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, top);
	if (r == ENOSYS) r = -syscall(SYS_futex, addr, FUTEX_WAIT, val, top);
	if (r != EINTR && r != ETIMEDOUT && r != ECANCELED) r = 0;

	return r;
}

int __timedwait(volatile int *addr, int val,
	clockid_t clk, const struct timespec *at, int priv)
{
	int cs, r;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	r = __timedwait_cp(addr, val, clk, at, priv);
	pthread_setcancelstate(cs, 0);
	return r;
}
