#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "hw1_syscalls.h"
#include "test_utilities.h"

int main() {
    int my_pid = getpid();
    int son = fork();
    if (son == 0) {
        printf("SON PID IS %d\n", getpid());
    } else {
        printf("Father PID is: %d\n", my_pid);
        ASSERT_TEST(enable_policy(my_pid, 1000, 234123) == 0);
        ASSERT_TEST(set_process_capabilities(my_pid, 0, 234123) == 0);

        int i=0;
        for (i = 0; i < 4000; i++)
            wait(NULL);
        ASSERT_TEST(disable_policy(my_pid, 234123) == 0);
        ASSERT_TEST(enable_policy(my_pid, 1000, 234123) == 0);
        ASSERT_TEST(set_process_capabilities(my_pid, 0, 234123) == 0);
        ASSERT_TEST(set_process_capabilities(my_pid, 0, 234123) == 0);

        wait(NULL);
    }
    return 0;
}
