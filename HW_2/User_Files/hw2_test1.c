#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "hw2_syscalls.h"
#include "test_utilities.h"

#define SCHED_LOTTERY	3


void printLogger(cs_log* log_arr, int size){
    printf("The logger is:\n");
    int i;
    for(i = 0; i < size;i++){
        printf("\n index of log is: %d\n",i);
        printf("prev pid: %d\n", log_arr[i].prev);
        printf("next pid: %d\n", log_arr[i].next);
        printf("prev priority : %d\n", log_arr[i].prev_priority);
        printf("next priority : %d\n", log_arr[i].next_priority);
        printf("prev policy : %d\n", log_arr[i].prev_policy);
        printf("next policy : %d\n", log_arr[i].next_policy);
        printf("switch time is : %d\n", log_arr[i].switch_time);
    }
}


int main(){
    int my_pid = getpid();
    disable_logging();
    ASSERT_TEST(enable_logging(-1) == -1 && errno == EINVAL);
    ASSERT_TEST(disable_logging() == -1 &&  errno == EINVAL );
    ASSERT_TEST(enable_logging(1) == 0);
    ASSERT_TEST(enable_logging(0) == -1 && errno == EINVAL);
    ASSERT_TEST(disable_logging() == 0);
    ASSERT_TEST(get_logger_records(NULL) == -1 && errno == ENOMEM );
    ASSERT_TEST(enable_logging(10) == 0);
    sleep(5);
    ASSERT_TEST(disable_logging() == 0);
    cs_log log_arr10[10];
    ASSERT_TEST(get_logger_records(log_arr10) == 0 );
    printLogger(log_arr10,10);

    ASSERT_TEST(enable_logging(5) == 0);
    sleep(5);
    ASSERT_TEST(get_logger_records(log_arr10) == 0 );
    printLogger(log_arr10,10);

    printf("print the log last time\n");
    ASSERT_TEST(get_logger_records(log_arr10) == 0);
    printLogger(log_arr10,10);

    ASSERT_TEST(disable_logging() == 0);
    ASSERT_TEST(start_lottery_scheduler() == 0);
    ASSERT_TEST(start_lottery_scheduler() == -1 && errno == EINVAL);
    ASSERT_TEST(start_orig_scheduler() == 0);
    ASSERT_TEST(start_orig_scheduler() == -1 && errno == EINVAL);
    ASSERT_TEST(sched_setscheduler(my_pid,SCHED_LOTTERY,NULL) == -1 && errno == EINVAL);
    ASSERT_TEST(start_lottery_scheduler() == 0);
    ASSERT_TEST(sched_setscheduler(my_pid,0,NULL) == -1 && errno == EINVAL);
    ASSERT_TEST(start_orig_scheduler() == 0);

    return 0;
}
