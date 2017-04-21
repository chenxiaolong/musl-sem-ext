#include "musl-sem.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "internal/sem-limits.h"
#include "internal/threading.h"

char *__shm_mapname(const char *name, char *buf)
{
	char *p;
	while (*name == '/') name++;
	if (*(p = strchrnul(name, '/')) || p==name ||
	    (p-name <= 2 && name[0]=='.' && p[-1]=='.')) {
		errno = EINVAL;
		return 0;
	}
	if (p-name > NAME_MAX) {
		errno = ENAMETOOLONG;
		return 0;
	}
	memcpy(buf, "/dev/shm/", 9);
	memcpy(buf+9, name, p-name+1);
	return buf;
}

typedef struct {
	ino_t ino;
	musl_sem_t *sem;
	int refcnt;
} semtab_t;
static semtab_t *semtab;
static int semtab_capacity = MUSL_SEM_NSEMS_MAX;
static volatile int lock[2];

#define FLAGS (O_RDWR|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK)

musl_sem_t *musl_sem_open(const char *name, int flags, ...)
{
	va_list ap;
	mode_t mode;
	unsigned value;
	int fd, i, e, slot, first=1, cnt, cs;
	musl_sem_t newsem;
	void *map;
	char tmp[64];
	struct timespec ts;
	struct stat st;
	char buf[NAME_MAX+10];

	if (!(name = __shm_mapname(name, buf)))
		return MUSL_SEM_FAILED;

	LOCK(lock);
	/* Allocate table if we don't have one yet */
	if (!semtab && !(semtab = calloc(sizeof *semtab, semtab_capacity))) {
		UNLOCK(lock);
		return MUSL_SEM_FAILED;
	}

	/* Reserve a slot in case this semaphore is not mapped yet;
	 * this is necessary because there is no way to handle
	 * failures after creation of the file. */
	slot = -1;
	for (cnt=i=0; i<semtab_capacity; i++) {
		cnt += semtab[i].refcnt;
		if (!semtab[i].sem && slot < 0) slot = i;
	}
	/* Enlarge semtab if it's full */
	if (slot < 0 && semtab_capacity < (int) (INT_MAX / 2 / sizeof(*semtab))) {
		int new_capacity = semtab_capacity << 1;
		semtab_t *new_semtab = realloc(semtab, sizeof(*new_semtab) * new_capacity);
		if (!new_semtab) {
			UNLOCK(lock);
			return MUSL_SEM_FAILED;
		}

		/* Zero out new structures */
		memset(new_semtab + semtab_capacity, 0, sizeof(semtab_t) * (new_capacity - semtab_capacity));

		slot = semtab_capacity;
		semtab = new_semtab;
		semtab_capacity = new_capacity;
	}
	/* Avoid possibility of overflow later */
	if (cnt == INT_MAX || slot < 0) {
		errno = EMFILE;
		UNLOCK(lock);
		return MUSL_SEM_FAILED;
	}
	/* Dummy pointer to make a reservation */
	semtab[slot].sem = (musl_sem_t *)-1;
	UNLOCK(lock);

	flags &= (O_CREAT|O_EXCL);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	/* Early failure check for exclusive open; otherwise the case
	 * where the semaphore already exists is expensive. */
	if (flags == (O_CREAT|O_EXCL) && access(name, F_OK) == 0) {
		errno = EEXIST;
		goto fail;
	}

	for (;;) {
		/* If exclusive mode is not requested, try opening an
		 * existing file first and fall back to creation. */
		if (flags != (O_CREAT|O_EXCL)) {
			fd = open(name, FLAGS);
			if (fd >= 0) {
				if (fstat(fd, &st) < 0 ||
				    (map = mmap(0, sizeof(musl_sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
					close(fd);
					goto fail;
				}
				close(fd);
				break;
			}
			if (errno != ENOENT)
				goto fail;
		}
		if (!(flags & O_CREAT))
			goto fail;
		if (first) {
			first = 0;
			va_start(ap, flags);
			mode = va_arg(ap, mode_t) & 0666;
			value = va_arg(ap, unsigned);
			va_end(ap);
			if (value > MUSL_SEM_VALUE_MAX) {
				errno = EINVAL;
				goto fail;
			}
			musl_sem_init(&newsem, 1, value);
		}
		/* Create a temp file with the new semaphore contents
		 * and attempt to atomically link it as the new name */
		clock_gettime(CLOCK_REALTIME, &ts);
		snprintf(tmp, sizeof(tmp), "/dev/shm/tmp-%d", (int)ts.tv_nsec);
		fd = open(tmp, O_CREAT|O_EXCL|FLAGS, mode);
		if (fd < 0) {
			if (errno == EEXIST) continue;
			goto fail;
		}
		if (write(fd, &newsem, sizeof newsem) != sizeof newsem || fstat(fd, &st) < 0 ||
		    (map = mmap(0, sizeof(musl_sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
			close(fd);
			unlink(tmp);
			goto fail;
		}
		close(fd);
		e = link(tmp, name) ? errno : 0;
		unlink(tmp);
		if (!e) break;
		munmap(map, sizeof(musl_sem_t));
		/* Failure is only fatal when doing an exclusive open;
		 * otherwise, next iteration will try to open the
		 * existing file. */
		if (e != EEXIST || flags == (O_CREAT|O_EXCL))
			goto fail;
	}

	/* See if the newly mapped semaphore is already mapped. If
	 * so, unmap the new mapping and use the existing one. Otherwise,
	 * add it to the table of mapped semaphores. */
	LOCK(lock);
	for (i=0; i<semtab_capacity && semtab[i].ino != st.st_ino; i++);
	if (i<semtab_capacity) {
		munmap(map, sizeof(musl_sem_t));
		semtab[slot].sem = 0;
		slot = i;
		map = semtab[i].sem;
	}
	semtab[slot].refcnt++;
	semtab[slot].sem = map;
	semtab[slot].ino = st.st_ino;
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
	return map;

fail:
	pthread_setcancelstate(cs, 0);
	LOCK(lock);
	semtab[slot].sem = 0;
	UNLOCK(lock);
	return MUSL_SEM_FAILED;
}

int musl_sem_close(musl_sem_t *sem)
{
	int i;
	LOCK(lock);
	for (i=0; i<semtab_capacity && semtab[i].sem != sem; i++);
	if (!--semtab[i].refcnt) {
		semtab[i].sem = 0;
		semtab[i].ino = 0;
	}
	UNLOCK(lock);
	munmap(sem, sizeof *sem);
	return 0;
}
