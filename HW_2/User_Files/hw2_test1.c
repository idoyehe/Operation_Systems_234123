#include <limits.h>
#include <stdlib.h>
#include <sched.h>

#include "hw2_syscalls.h"
#include "test_utilities.h"

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2

#define TEST_SIZE 1000
#define LOG_SIZE 4000

bool test_enable_logging() {
	// size < 0
	ASSERT_TEST(enable_logging(-3) == -1 && errno == EINVAL);
	
	// success
	ASSERT_TEST(enable_logging(0) == 0);

	// already enabled
	ASSERT_TEST(enable_logging(1) == -1 && errno == EINVAL);

	return true;
}


bool test_disable_logging() {
	// success
	ASSERT_TEST(disable_logging() == 0);
	
	// already disabled
	ASSERT_TEST(disable_logging() == -1 && errno == EINVAL);
	
	// success
	ASSERT_TEST(enable_logging(0) == 0);
	ASSERT_TEST(disable_logging() == 0);
	
	// test correct size
	
	return true;
}

bool test_get_logger_records() {
	struct sched_param param;
	param.sched_priority = 20;
	sched_setscheduler(getpid(), SCHED_FIFO, &param);
	
	param.sched_priority = 30;
	int parent_pid = getpid();
	int first_child_pid;
	int second_child_pid;
	cs_log log[10];

	// zero size success
	disable_logging();
	ASSERT_TEST(enable_logging(0) == 0);

	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(get_logger_records(log) == 0);

	// simple success
	ASSERT_TEST(enable_logging(3) == 0);

	first_child_pid = fork();
	sched_setscheduler(first_child_pid, SCHED_FIFO, &param);
	if (first_child_pid == 0) exit(0);
	wait(NULL);

	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(get_logger_records(log) == 3);
	ASSERT_TEST(log[0].prev == parent_pid);
	ASSERT_TEST(log[0].next == parent_pid);
	ASSERT_TEST(log[0].prev_priority == 79);
	ASSERT_TEST(log[0].next_priority == 79);
	ASSERT_TEST(log[0].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[0].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].prev == parent_pid);
	ASSERT_TEST(log[1].next == first_child_pid);
	ASSERT_TEST(log[1].prev_priority == 79);
	ASSERT_TEST(log[1].next_priority == 69);
	ASSERT_TEST(log[1].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[2].prev == first_child_pid);
	ASSERT_TEST(log[2].next == parent_pid);
	ASSERT_TEST(log[2].prev_priority == 69);
	ASSERT_TEST(log[2].next_priority == 79);
	ASSERT_TEST(log[2].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[2].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[0].switch_time <= log[1].switch_time);
	ASSERT_TEST(log[1].switch_time <= log[2].switch_time);

	// success of size 2 after size 3
	param.sched_priority = 40;
	ASSERT_TEST(enable_logging(2) == 0);

	second_child_pid = fork();
	sched_setscheduler(second_child_pid, SCHED_RR, &param);
	if (second_child_pid == 0) exit(0);
	wait(NULL);

	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(get_logger_records(log) == 2);
	ASSERT_TEST(log[0].prev == parent_pid);
	ASSERT_TEST(log[0].next == parent_pid);
	ASSERT_TEST(log[0].prev_priority == 79);
	ASSERT_TEST(log[0].next_priority == 79);
	ASSERT_TEST(log[0].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[0].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].prev == parent_pid);
	ASSERT_TEST(log[1].next == second_child_pid);
	ASSERT_TEST(log[1].prev_priority == 79);
	ASSERT_TEST(log[1].next_priority == 59);
	ASSERT_TEST(log[1].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].next_policy == SCHED_RR);
	ASSERT_TEST(log[0].switch_time <= log[1].switch_time);

	// if you don't pass this section, don't worry.
	ASSERT_TEST(log[2].prev == first_child_pid);
	ASSERT_TEST(log[2].next == parent_pid);
	ASSERT_TEST(log[2].prev_priority == 69);
	ASSERT_TEST(log[2].next_priority == 79);
	ASSERT_TEST(log[2].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[2].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[2].switch_time <= log[0].switch_time);
	// section ends here

	// NULL user_mem, when enable is of size 0
	ASSERT_TEST(enable_logging(0) == 0);
	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(get_logger_records(NULL) == -1 && errno == ENOMEM);

	// NULL user_mem - failure -> success
	param.sched_priority = 30;
	ASSERT_TEST(enable_logging(3) == 0);

	first_child_pid = fork();
	if (first_child_pid == 0) exit(0);
	wait(NULL);

	ASSERT_TEST(get_logger_records(NULL) == -1 && errno == ENOMEM);
	
	second_child_pid = fork();
	sched_setscheduler(second_child_pid, SCHED_FIFO, &param);
	if (second_child_pid == 0) exit(0);
	wait(NULL);

	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(get_logger_records(log) == 3);
	ASSERT_TEST(log[0].prev == parent_pid);
	ASSERT_TEST(log[0].next == parent_pid);
	ASSERT_TEST(log[0].prev_priority == 79);
	ASSERT_TEST(log[0].next_priority == 79);
	ASSERT_TEST(log[0].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[0].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].prev == parent_pid);
	ASSERT_TEST(log[1].next == second_child_pid);
	ASSERT_TEST(log[1].prev_priority == 79);
	ASSERT_TEST(log[1].next_priority == 69);
	ASSERT_TEST(log[1].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[2].prev == second_child_pid);
	ASSERT_TEST(log[2].next == parent_pid);
	ASSERT_TEST(log[2].prev_priority == 69);
	ASSERT_TEST(log[2].next_priority == 79);
	ASSERT_TEST(log[2].prev_policy == SCHED_FIFO);
	ASSERT_TEST(log[2].next_policy == SCHED_FIFO);
	ASSERT_TEST(log[1].switch_time <= log[2].switch_time);


	// Should result in copy_to_user failing
	ASSERT_TEST(enable_logging(LOG_SIZE) == 0);
	int i;
	for (i = 0; i < LOG_SIZE; ++i) {
		first_child_pid = fork();
		if (first_child_pid == 0) exit(0);
		wait(NULL);
	}
	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(get_logger_records(log) == -1 && errno == ENOMEM);

	// Should work, because last call erases the log
	ASSERT_TEST(get_logger_records(log) == 0);

	return true;
}

bool enable_disable_stress_test() {
	disable_logging();
	int i;
	for (i = 0; i < TEST_SIZE; ++i) {

		// print every 1%. used as an indicator that the kernel did not freeze
		if (i % (TEST_SIZE / 100) == 0) {
			printf(".");
			fflush(stdout);
		}

		// in case of memory leaks, enable_policy will eventually fail, or the kernel will freeze/crash
		if (enable_logging(LOG_SIZE) != 0) exit(-1);

		// add entries to log
		int j;
		int child;
		
		for (j = 0; j < LOG_SIZE; ++j) {
			child = fork();
			if (child == 0) exit(0);
			wait(NULL);
		}
		// kill process without disabling the policy

		if (disable_logging() != 0) exit(-1);
	}
	printf("\n");
	return true;
}

int main() {
	disable_logging();
	
	int test_child;
	RUN_TEST_CHILD(test_child, test_enable_logging);
	RUN_TEST_CHILD(test_child, test_disable_logging);
	RUN_TEST_CHILD(test_child, test_get_logger_records);
	RUN_TEST_CHILD(test_child, enable_disable_stress_test);
	return 0;
}
