#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include "hw2_syscalls.h"
#include "test_utilities.h"
#include <stdlib.h>
#include <string.h>

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_LOTTERY		3

#define MAXSTRING 255
#define TEST_SIZE 12500
#define LOG_SIZE 500
#define MAX_TICKETS 100

bool test_start_lottery_orig_scheduler() {

  	disable_logging();
   	start_orig_scheduler();

	ASSERT_TEST(start_lottery_scheduler() == 0);

	struct sched_param param;
	param.sched_priority = 30;
	ASSERT_TEST(sched_setscheduler(getpid(), SCHED_OTHER, &param) == -1);
	ASSERT_TEST(sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1);
	ASSERT_TEST(sched_setscheduler(getpid(), SCHED_RR, &param) == -1);
	ASSERT_TEST(sched_setscheduler(getpid(), SCHED_LOTTERY, &param) == -1);

	ASSERT_TEST(start_lottery_scheduler() == -1 && errno == EINVAL);

	ASSERT_TEST(start_orig_scheduler() == 0);

	ASSERT_TEST(sched_setscheduler(getpid(), SCHED_LOTTERY, &param) == -1);
	ASSERT_TEST(start_orig_scheduler() == -1 && errno == EINVAL);

	int i = 0;
	int user_mem[3] = {0, 0, 0};
	for (i = 0; i < TEST_SIZE; i++) {
		ASSERT_TEST(start_lottery_scheduler() == 0);
		ASSERT_TEST(start_lottery_scheduler() == -1 && errno == EINVAL);
		ASSERT_TEST(start_orig_scheduler() == 0);
		ASSERT_TEST(start_orig_scheduler() == -1 && errno == EINVAL);
	}
	return true;
}

bool test_log_lottery() {
   	disable_logging();
	start_orig_scheduler();
	cs_log log[LOG_SIZE];
	int parent_pid = getpid();
	struct sched_param param;
	param.sched_priority = 29; //prio = 70, num_of_tickets = 70
	sched_setscheduler(parent_pid, SCHED_FIFO, &param);
	set_max_tickets(MAX_TICKETS);
	int child_pid = fork();
	param.sched_priority = 9; //prio = 90, num_of_tickets = 50
	sched_setscheduler(child_pid, SCHED_FIFO, &param);
	enable_logging(LOG_SIZE);
	start_lottery_scheduler();
	//just a triple loop to make the processes run and perform context switches between them :)
	int i, j, k;
	for (i = 0; i < TEST_SIZE; ++i) {
	   for (j = 0; j < TEST_SIZE; ++j) {
	      for (k = 0; k < LOG_SIZE; ++k) {

	      }
	   }
	}
	if (child_pid == 0) {
	   exit(0);
	}
	wait(NULL);
	int count70=0;
	int count90=0;
	int total = 0;
	ASSERT_TEST(disable_logging() == 0);
	ASSERT_TEST(start_orig_scheduler() == 0);
	int logger_records_size = get_logger_records(log);
	FILE *stream = fopen("stream.txt", "w");
	if (!stream) {
	   printf("Cannot open stream.txt!\n");
	   return false;
	}
	FILE *stream_log = fopen("stream_log.txt", "w");
	if (!stream_log) {
		printf("Cannot open stream_log.txt!");
	}
	for (i = 0; i < logger_records_size; i++) {
	  fprintf(stream_log, "Log size: %d\n", logger_records_size);
	  fprintf(stream_log, "log[%d]: prev = %d, next = %d, prev_priority = %d, next_priority ~%d~, prev_policy = %d, next_policy = %d, n_tickets = %d, switch_time = %d \n\n", i, log[i].prev, log[i].next, log[i].prev_priority, log[i].next_priority, log[i].prev_policy, log[i].next_policy, log[i].n_tickets, log[i].switch_time);
	  if(log[i].prev_priority <=99 && log[i].next_priority <= 99){
	  	count70 += (log[i].next_priority == 70);
	  	count90 += (log[i].next_priority == 90);
	  	total++;
	  }
	}
	fclose(stream);
	fclose(stream_log);
	printf("Count90 is: %d\n",count90);
	printf("Count70 is: %d\n",count70);
	printf("Total is: %d\n",total);
	float final = ((float)count90 / (float)(total));
	printf("final is:%f\n",final);
	if (final <= 0.31 && final >= 0.29){
	   return true;
	}
	return false;
}

int main() {
	disable_logging();
	start_orig_scheduler();
	RUN_TEST(test_start_lottery_orig_scheduler);
	RUN_TEST(test_log_lottery);
	return 0;
}
