#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <sys/prctl.h>

int main() {
    if (prctl(PR_SET_NAME, "szz_rt", 0, 0, 0) != 0) {
        perror("prctl(PR_SET_NAME)");
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    int ret = sched_setaffinity(0, sizeof(cpuset), &cpuset);

    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 45;  // SCHED_FIFO/SCHED_RR 优先级范围 1-99（99最高）
    ret = sched_setscheduler(0, SCHED_FIFO, &sp);

    struct timespec ts;
    ret = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if (ret == 0) {
        printf("monotonicraw time: %ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
    }

    while(1) { 

    }

    return 0;
}