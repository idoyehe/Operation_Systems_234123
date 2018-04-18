#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include "../User_Files/hw1_syscalls.h"


int main(void) {

    int pid = fork();
    if (pid == 0) {
        printf("Im the son of The main process, fork Succseed\n");
    } else {
        wait(NULL);
        printf("First Son is DEAD\n");


        if (enable_policy(getpid(), 10, 1234) == -1) {
            assert(errno == EINVAL);
            printf("Wrong password!!\n");
        }

        if (enable_policy(getpid(), 10, 234123) == 0) {
            printf("HW1 Policiy is active!!\n");
        } else {
            printf("ERROR: errno is %d", errno);
        }

        pid = fork();
        if (pid == 0) {
            printf("Im the son of The main process, fork Succseed\n");
        } else {
            wait(NULL);
            printf("Second Son is DEAD\n");

            if (set_process_capabilities(getpid(), 1, 1234) == -1) {
                assert(errno == EINVAL);
                printf("Wrong password!!\n");
            }

            if (set_process_capabilities(getpid(), 3, 234123) == -1) {
                assert(errno == EINVAL);
                printf("Wrong Privilige!!\n");
            }

            if (set_process_capabilities(getpid(), 1, 234123) == 0) {
                printf("This process can't do fork!!\n");
            }

            pid = fork();
            if (pid == 0) {
                printf("Im the son of The main process, fork Succseed, fork Should be failed\n");
            } else {
                assert(errno == EINVAL);
                printf("Second Son is DEAD\n");
                struct forbidden_activity_info arr[1];
                assert(get_process_log(getpid(), 1, arr) == 0);
                assert(arr->proc_level == 1 && arr->syscall_req_level == 2);
                printf("time of bad act: %d\n", arr->time);
                assert(get_process_log(getpid(), 1, arr) == -1);
                assert(errno == EINVAL);

                if (set_process_capabilities(getpid(), 0, 234123) == 0) {
                    printf("This process can't do wait!!\n");
                }
                wait(NULL);

                pid = fork();
                if (pid == 0) {
                    printf("Im the son of The main process, fork Succseed, fork Should be failed\n");
                } else {
                    assert(disable_policy(getpid(), 234123) == 0);
                    pid = fork();
                    if (pid == 0) {
                        printf("Im the son of The main process, fork Succseed, fork Should be good\n");
                    }

                }
            }
        }
    }
    return 0;
}
