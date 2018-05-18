#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>


#include "hw2_syscalls.h"
#include "test_utilities.h"

#define LOG_SIZE 100

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_LOTTERY	3 // WET_2 new policy

#define USER_LIMIT 20
#define USER_LIMIT_ON 1

#define MAX_TICKETS ((USER_LIMIT)*(USER_LIMIT_ON))


int main() {
    ASSERT_TEST(start_orig_scheduler() == -1);//make sure Lottery is OFF
    ASSERT_TEST(disable_logging() == 0);//make sure Logger is OFF
    cs_log log_output[LOG_SIZE];

    int acctual_log = get_logger_records(log_output);
    ASSERT_TEST(acctual_log!=-1);
    printf("log size is: %d\n",acctual_log);

    int j;
    for( j=0 ; j<acctual_log; j++) {
        printf("\nlog_output[%d] is:\n\n", j + 1);
        printf("\tlog_output[%d].prev_pid is: %d\n", j + 1, log_output[j].prev);
        printf("\tlog_output[%d].next_pid is: %d\n", j + 1, log_output[j].next);
        printf("\tlog_output[%d].prev_priority is: %d\n", j + 1,
               log_output[j].prev_priority);
        printf("\tlog_output[%d].next_priority is: %d\n", j + 1,
               log_output[j].next_priority);
        printf("\tlog_output[%d].prev_policy is: %d\n", j + 1,
               log_output[j].prev_policy);
        ASSERT_TEST(log_output[j].prev_policy != SCHED_LOTTERY);

        printf("\tlog_output[%d].next_policy is: %d\n", j + 1,
               log_output[j].next_policy);
        ASSERT_TEST(log_output[j].next_policy != SCHED_LOTTERY);

        printf("\tlog_output[%d].switch_time is: %d\n", j + 1,
               log_output[j].switch_time);
        printf("\tlog_output[%d].NT is: %d\n", j + 1, log_output[j].n_tickets);
        ASSERT_TEST(log_output[j].n_tickets == -1);
    }
    return 0;
}
