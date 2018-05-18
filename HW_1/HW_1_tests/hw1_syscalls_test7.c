#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "hw1_syscalls.h"
#include "test_utilities.h"

#define PASSWORD 234123
#define TEST_SIZE 10000
#define LOG_SIZE 5000

bool memory_strees_test() {
    int i;
    for (i = 0; i < TEST_SIZE; ++i) {
        // print every 1%. used as an indicator that the kernel did not freeze
        if (i % (TEST_SIZE / 100) == 0) {
            printf(".");
            fflush(stdout);
        }

        // in case of memory leaks, enable_policy will eventually fail, or the kernel will freeze/crash
        if (enable_policy(getpid(), LOG_SIZE, PASSWORD) != 0) {
            exit(-1);
        }
        set_process_capabilities(getpid(), 0, PASSWORD);
        // add entries to log
        int j;
        for (j = 0; j < LOG_SIZE; ++j) {
            wait(NULL);
        }
        // kill process without disabling the policy

        if (disable_policy(getpid(), PASSWORD) != 0) {
            exit(-1);
        }
    }
    return true;
}

int main() {
    RUN_TEST(memory_strees_test);
    return 0;
}
