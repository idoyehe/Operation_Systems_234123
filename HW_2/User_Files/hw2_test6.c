#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include "hw2_syscalls.h"
#include "test_utilities.h"

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_LOTTERY   3

#define LOG_SIZE 500

int main(int argc, char const *argv[]) {
    ASSERT_TEST(argc == 2);
    int temp = atoi(argv[1]);
    double duration = ((double)temp);
    printf("runtime is: %f\n",duration);
    int master_pid = getpid();
    printf("Master Pid is: %d\n", master_pid);
    set_max_tickets(278);
    int first_child_pid, second_child_pid;
    cs_log log_output[LOG_SIZE];
    disable_logging();
    start_orig_scheduler();
    ASSERT_TEST(enable_logging(0) == 0);//make sure Logger is OFF
    ASSERT_TEST(disable_logging() == 0);//make sure Logger is OFF
    ASSERT_TEST(disable_logging() == -1);//make sure Logger is OFF

    time_t start_t;

    struct sched_param param;
    param.sched_priority = 98;
    first_child_pid = fork();
    if (first_child_pid == 0) {
        sched_setscheduler(getpid(), SCHED_RR, &param);
        ASSERT_TEST(start_lottery_scheduler() == 0);
        ASSERT_TEST(enable_logging(LOG_SIZE) == 0);
        second_child_pid = fork();
        start_t = clock();
        while (((double)((clock() - start_t))/ CLOCKS_PER_SEC )< duration);
        set_max_tickets(139);
        exit(0);
    }
else {
        wait(NULL);
        ASSERT_TEST(disable_logging() == 0);//make sure Logger is OFF
        ASSERT_TEST(start_orig_scheduler() == 0);
        ASSERT_TEST(disable_logging() == -1);
        ASSERT_TEST(start_orig_scheduler() == -1);

        int acctual_log = get_logger_records(log_output);
        ASSERT_TEST(acctual_log <= LOG_SIZE);

        int j;
        for(j = 0; j < acctual_log ;j++){
            if(log_output[j].next != first_child_pid && log_output[j].next_priority == 1){
                second_child_pid = log_output[j].next;
                break;
            }
        }

        printf("first_child_pid is: %d\n", first_child_pid);
        printf("second_child_pid is: %d\n", second_child_pid);

        double first_end_time=-1,second_end_time=-1;
        for(j=acctual_log-1; j>=0 && ((first_end_time == -1)||(second_end_time == -1));j--){
            if(log_output[j].prev == first_child_pid && first_end_time == -1){
                first_end_time = log_output[j].switch_time;
            }
            if(log_output[j].prev == second_child_pid && second_end_time == -1){
                second_end_time = log_output[j].switch_time;
            }
        }
        printf("first_end_time is:%f\n",first_end_time);
        printf("second_end_time is:%f\n",second_end_time);
        printf("first_end_time/second_end_time is:%f\n",(first_end_time/second_end_time));

        printf("\n--------------\n");

        for (j = 0; j < acctual_log; j++) {
            printf("\nlog_output[%d] is:\n\n", j + 1);
            printf("\tlog_output[%d].prev_pid is: %d\n", j + 1, log_output[j].prev);
            printf("\tlog_output[%d].next_pid is: %d\n", j + 1, log_output[j].next);
            printf("\tlog_output[%d].prev_priority is: %d\n", j + 1,
                   log_output[j].prev_priority);
            printf("\tlog_output[%d].next_priority is: %d\n", j + 1,
                   log_output[j].next_priority);

            printf("\tlog_output[%d].prev_policy is: %d\n", j + 1,
                   log_output[j].prev_policy);
            printf("\tlog_output[%d].next_policy is: %d\n", j + 1,
                   log_output[j].next_policy);

            ASSERT_TEST(log_output[j].prev_policy == SCHED_LOTTERY);
            ASSERT_TEST(log_output[j].next_policy == SCHED_LOTTERY);

            printf("\tlog_output[%d].switch_time is: %d\n", j + 1,
                   log_output[j].switch_time);
            printf("\tlog_output[%d].NT is: %d\n", j + 1, log_output[j].n_tickets);

        }
    }
    return 0;
}
