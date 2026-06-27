#include "deterministic_pthread.h"

#include <errno.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static pthread_once_t init_once = PTHREAD_ONCE_INIT;

static pthread_setschedparam_fn original_setschedparam = NULL;
static pthread_setaffinity_np_fn original_setaffinity_np = NULL;

static int DET_PTHREAD_PRIORITY;
static cpu_set_t DET_PTHREAD_CPU_SET;

static void init(void)
{
    original_setschedparam = (pthread_setschedparam_fn)dlsym(RTLD_NEXT, "pthread_setschedparam");
    original_setaffinity_np = (pthread_setaffinity_np_fn)dlsym(RTLD_NEXT, "pthread_setaffinity_np");

    DET_PTHREAD_PRIORITY = -1;
    const char * env_priority = getenv("DET_PTHREAD_PRIORITY");
    if (env_priority != NULL) {
        char * end = NULL;
        errno = 0;
        long priority = strtol(env_priority, &end, 10);

        if (errno == 0 && end != env_priority && *end == '\0' &&
            priority >= INT_MIN && priority <= INT_MAX) {
            DET_PTHREAD_PRIORITY = (int)priority;
        }
    }

    CPU_ZERO(&DET_PTHREAD_CPU_SET);

    const char * env_cpu_set = getenv("DET_PTHREAD_CPU_SET");
    if (env_cpu_set != NULL && *env_cpu_set != '\0') {
        char * env_cpu_set_copy = strdup(env_cpu_set);
        if (env_cpu_set_copy != NULL) {
            char * saveptr = NULL;
            char * token = strtok_r(env_cpu_set_copy, ",", &saveptr);

            while (token != NULL) {
                char * end = NULL;
                errno = 0;
                long cpu = strtol(token, &end, 10);

                if (errno == 0 && end != token && *end == '\0' &&
                    cpu >= 0 && cpu < CPU_SETSIZE) {
                    CPU_SET((int)cpu, &DET_PTHREAD_CPU_SET);
                }

                token = strtok_r(NULL, ",", &saveptr);
            }

            free(env_cpu_set_copy);
        }
    }
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param * param)
{
    pthread_once(&init_once, init);

    if (original_setschedparam == NULL) {
        return ENOSYS;
    }

    if (param == NULL) {
        return EINVAL;
    }

    struct sched_param modified_param = *param;

    if (policy == SCHED_FIFO || policy == SCHED_RR) {
        if (DET_PTHREAD_PRIORITY != -1) {
            int min_priority = sched_get_priority_min(policy);
            int max_priority = sched_get_priority_max(policy);
            if (min_priority != -1 && max_priority != -1 &&
                min_priority <= DET_PTHREAD_PRIORITY && DET_PTHREAD_PRIORITY <= max_priority) {
                modified_param.sched_priority = DET_PTHREAD_PRIORITY;
            }
        }

        if (original_setaffinity_np && CPU_COUNT(&DET_PTHREAD_CPU_SET) > 0) {
            original_setaffinity_np(thread, sizeof(cpu_set_t), &DET_PTHREAD_CPU_SET);
        }
    }

    return original_setschedparam(thread, policy, &modified_param);
}
