#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "hw1_syscalls.h"
#include "test_utilities.h"

int main() {
    int my_pid = getpid();
    printf("Process PID is: %d\n", my_pid);
    while(1) {
        ASSERT_TEST(enable_policy(my_pid, 1, 234123) == 0);
        ASSERT_TEST(set_process_capabilities(my_pid, 0, 234123) == 0);
		wait(NULL);
        ASSERT_TEST(errno == EINVAL);
		wait(NULL);
		ASSERT_TEST(errno == EINVAL);
		wait(NULL);
		ASSERT_TEST(errno == EINVAL);
        ASSERT_TEST(disable_policy(my_pid,234123) == 0);
    }
    return 0;
}
