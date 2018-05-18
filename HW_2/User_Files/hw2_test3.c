#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>


#include "hw2_syscalls.h"
#include "test_utilities.h"

#define LOG_SIZE 2500
#define LENGTH 120
#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_LOTTERY	3 // WET_2 new policy


#define USER_LIMIT 40
#define USER_LIMIT_ON 1

#define MAX_TICKETS ((USER_LIMIT)*(USER_LIMIT_ON))

bool test_enable_lottery_for_duration(){
    set_max_tickets(MAX_TICKETS);
    ASSERT_TEST(enable_logging(0) == 0);
    ASSERT_TEST(disable_logging() == 0);//make sure Logger is OFF
    start_orig_scheduler();//make sure Lottery is OFF
    printf("PID of test3 is: %d\n",getpid());
    ASSERT_TEST(start_lottery_scheduler () == 0);
    printf("LOTTERY is ON!!\n");
    ASSERT_TEST(enable_logging(LOG_SIZE) == 0);
    printf("enable_logging is ON!!\n");


    sleep(LENGTH);

    cs_log log_output[LOG_SIZE];
    int acctual_log = get_logger_records(log_output);
    ASSERT_TEST(acctual_log !=-1);

    ASSERT_TEST(disable_logging() == 0);
    ASSERT_TEST(enable_logging(0) == 0);
    ASSERT_TEST(disable_logging() == 0);
    ASSERT_TEST(start_orig_scheduler () == 0);
    printf("LOTTERY is OFF!!\n");


    int j;
    for( j=0 ; j<acctual_log;j++){
        printf("\nlog_output[%d] is:\n\n",j+1);
        printf("\tlog_output[%d].prev_pid is: %d\n",j+1,log_output[j].prev);
        printf("\tlog_output[%d].next_pid is: %d\n",j+1,log_output[j].next);
        printf("\tlog_output[%d].prev_priority is: %d\n",j+1,log_output[j].prev_priority);
        printf("\tlog_output[%d].next_priority is: %d\n",j+1,log_output[j].next_priority);

        printf("\tlog_output[%d].prev_policy is: %d\n",j+1,log_output[j].prev_policy);
        printf("\tlog_output[%d].next_policy is: %d\n",j+1,log_output[j].next_policy);

        ASSERT_TEST(log_output[j].prev_policy == SCHED_LOTTERY);
        ASSERT_TEST(log_output[j].next_policy == SCHED_LOTTERY);

        printf("\tlog_output[%d].switch_time is: %d\n",j+1,log_output[j].switch_time);
        printf("\tlog_output[%d].NT is: %d\n",j+1,log_output[j].n_tickets);
        ASSERT_TEST(log_output[j].n_tickets <= USER_LIMIT);


//        if (log_output[j].prev != 0 && log_output[j].next) {
//            printf("\tlog_output[%d].Random_ticket is: %u\n", j + 1,
//                   log_output[j].random_number);
//            printf("\tlog_output[%d].next_number_tickets is: %u\n", j + 1,
//                   log_output[j].next_n_tickets);
//            printf("\tlog_output[%d].before_tickets is: %u\n", j + 1,
//                   log_output[j].all_prev_tickts);
//            unsigned int number_tickets_before = log_output[j].all_prev_tickts;
//            unsigned int random_ticket = log_output[j].random_number;
//            unsigned int next_number_tickts = log_output[j].next_n_tickets;
//            unsigned int start_interval = number_tickets_before + 1;
//            unsigned int end_interval =
//                    number_tickets_before + next_number_tickts;
//            ASSERT_TEST(random_ticket >= start_interval);
//            ASSERT_TEST(random_ticket <= end_interval);
//            ASSERT_TEST(random_ticket > 0 && random_ticket <= log_output[j].n_tickets);
//        }
    }
    return true;
}


int main() {
    disable_logging();//make sure Logger is OFF
    start_orig_scheduler();//make sure Lottery is OFF

    RUN_TEST(test_enable_lottery_for_duration);

    ASSERT_TEST(enable_logging(0) == 0);
    ASSERT_TEST(disable_logging() == 0);//make sure Logger is OFF
    ASSERT_TEST(start_orig_scheduler () == -1);//make sure Lottery is OFF
    printf("LOTTERY is OFF!!\n");

    return 0;
}
