#include "internal/threading.h"

#include <stdint.h>

#include "internal/atomic_arch.h"

void __wait(volatile int *addr, volatile int *waiters, int val, int priv)
{
	int spins=100;
	if (priv) priv = 128;
	while (spins-- && (!waiters || !*waiters)) {
		if (*addr==val) a_spin();
		else return;
	}
	if (waiters) a_inc(waiters);
	while (*addr==val) {
		syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, 0) != -ENOSYS
		|| syscall(SYS_futex, addr, FUTEX_WAIT, val, 0);
	}
	if (waiters) a_dec(waiters);
}
