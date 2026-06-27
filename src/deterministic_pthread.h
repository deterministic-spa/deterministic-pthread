#define _GNU_SOURCE

#ifndef DETERMINISTIC_PTHREAD_H
#define DETERMINISTIC_PTHREAD_H

#include <pthread.h>
#include <sched.h>

typedef int (* pthread_setschedparam_fn) (pthread_t, int, const struct sched_param *);
typedef int (* pthread_setaffinity_np_fn) (pthread_t, size_t, const cpu_set_t *);

#endif /* DETERMINISTIC_PTHREAD_H */
