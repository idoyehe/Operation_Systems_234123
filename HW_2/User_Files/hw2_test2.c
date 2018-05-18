#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>


#include "hw2_syscalls.h"
#include "test_utilities.h"

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_LOTTERY	3 // WET_2 new policy

#define TEST_SIZE 100
#define LOG_SIZE 200
#define NICE_RANGE 40

#define USER_LIMIT 40
#define USER_LIMIT_ON 0

#define MAX_TICKETS ((USER_LIMIT)*(USER_LIMIT_ON))

bool test_syscall_setschduler() {
	start_orig_scheduler();//make sure Lottery is OFF
	struct sched_param param ={.sched_priority = 0};
	ASSERT_TEST(sched_setscheduler(getpid(),SCHED_LOTTERY,&param) == -1 && errno == EINVAL);
	ASSERT_TEST(sched_setscheduler(getpid(),SCHED_OTHER,&param) == 0);
	ASSERT_TEST(start_lottery_scheduler() == 0);
	ASSERT_TEST(sched_setscheduler(getpid(),SCHED_LOTTERY,&param) == -1 && errno == EINVAL);
	ASSERT_TEST(sched_setscheduler(getpid(),SCHED_OTHER,&param) == -1 && errno == EINVAL);
	ASSERT_TEST(start_orig_scheduler() == 0);
}

bool test_enable_and_disable_logging() {
	// size < 0
	ASSERT_TEST(enable_logging(-3) == -1 && errno == EINVAL);

	// success
	ASSERT_TEST(enable_logging(LOG_SIZE) == 0);

	// already enabled
	ASSERT_TEST(enable_logging(1) == -1 && errno == EINVAL);

	// success
	ASSERT_TEST(disable_logging() == 0);

	// already disabled
	ASSERT_TEST(disable_logging() == -1 && errno == EINVAL);

	// success
	ASSERT_TEST(enable_logging(0) == 0);
	ASSERT_TEST(disable_logging() == 0);

	return true;
}

bool test_enable_and_disable_lottery() {

	ASSERT_TEST(start_lottery_scheduler() == 0);

	ASSERT_TEST(start_lottery_scheduler() == -1 && errno == EINVAL);

	//because last test : test_enable_lottery () lottery is ON
	ASSERT_TEST(start_orig_scheduler() == 0);

	ASSERT_TEST(start_orig_scheduler() == -1 && errno == EINVAL);

	return true;
}

bool test_schedualer_lottery_behavior() {

	ASSERT_TEST(start_lottery_scheduler() == 0);
	ASSERT_TEST(enable_logging(LOG_SIZE) == 0);
	set_max_tickets(MAX_TICKETS);

	int child;
	int j;
	for (j = 0; j < TEST_SIZE; ++j) {
		child = fork();
		if (child == 0) {
			ASSERT_TEST(sched_yield() == 0);
			exit(0);
		}
		wait(NULL);
	}

	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(start_orig_scheduler() == 0);

	cs_log log_output[LOG_SIZE];
	ASSERT_TEST(get_logger_records(log_output));

	for( j=0 ; j<LOG_SIZE;j++) {
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

//		if (log_output[j].prev != 0 && log_output[j].next) {
//			printf("\tlog_output[%d].Random_ticket is: %u\n", j + 1,
//				   log_output[j].random_number);
//			printf("\tlog_output[%d].next_number_tickets is: %u\n", j + 1,
//				   log_output[j].next_n_tickets);
//			printf("\tlog_output[%d].before_tickets is: %u\n", j + 1,
//				   log_output[j].all_prev_tickts);
//			unsigned int number_tickets_before = log_output[j].all_prev_tickts;
//			unsigned int random_ticket = log_output[j].random_number;
//			unsigned int next_number_tickts = log_output[j].next_n_tickets;
//			unsigned int start_interval = number_tickets_before + 1;
//			unsigned int end_interval =
//					number_tickets_before + next_number_tickts;
//			ASSERT_TEST(random_ticket >= start_interval);
//			ASSERT_TEST(random_ticket <= end_interval);
//			ASSERT_TEST(random_ticket > 0 &&
//						random_ticket <= log_output[j].n_tickets);
//		}
	}


	ASSERT_TEST(enable_logging(0) == 0);
	ASSERT_TEST(disable_logging() == 0);

	return true;
}


int main() {
	disable_logging();//make sure Logger is OFF
	start_orig_scheduler();//make sure Lottery is OFF

	int test_child;
	RUN_TEST_CHILD(test_child, test_syscall_setschduler);
	RUN_TEST_CHILD(test_child, test_enable_and_disable_logging);
	RUN_TEST_CHILD(test_child, test_enable_and_disable_lottery);
	RUN_TEST_CHILD(test_child,test_schedualer_lottery_behavior);

	start_orig_scheduler();//make sure Lottery is OFF
	disable_logging();//make sure Logger is OFF
	ASSERT_TEST(enable_logging(0) == 0);
	ASSERT_TEST(disable_logging() == 0);
	return 0;
}
