#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "hw1_syscalls.h"
#include "test_utilities.h"


bool test_enable_policy() {
	// test pid < 0
	ASSERT_TEST(enable_policy(-5, 10, 234123) == -1 && errno == ESRCH);
	// test (hopefully) pid does not exist
	ASSERT_TEST(enable_policy(INT_MAX - 2, 10, 234123) == -1 && errno == ESRCH);
	// test incorrect password
	ASSERT_TEST(enable_policy(getpid(), 10, 234122) == -1 && errno == EINVAL);
	// test negative size
	ASSERT_TEST(enable_policy(getpid(), -2, 234123) == -1 && errno == EINVAL);
	// success
	ASSERT_TEST(enable_policy(getpid(), 10, 234123) == 0);
	// test policy already on
	ASSERT_TEST(enable_policy(getpid(), 10, 234123) == -1 && errno == EINVAL);
	// test pid does not exist
	int child_pid = fork();
	if (child_pid == 0) {
		exit(0);
		return true;
	}
	wait(NULL);
	ASSERT_TEST(enable_policy(/*dead*/child_pid, 10, 234123) == -1 && errno == ESRCH);
	return true;
	// log should be destroyed automatically!
}


bool test_disable_policy() {
	// test pid < 0
	ASSERT_TEST(disable_policy(-1, 234123) == -1 && errno == ESRCH);
	// test (hopefully) pid does not exist
	ASSERT_TEST(disable_policy(INT_MAX -2, 234123) == -1 && errno == ESRCH);
	// test policy already off
	ASSERT_TEST(disable_policy(getpid(), 234123) == -1 && errno == EINVAL);
	// enabled policy
	enable_policy(getpid(), 10, 234123);
	// test incorrect password
	ASSERT_TEST(disable_policy(getpid(), 1) == -1 && errno == EINVAL);
	// test policy defaults to off on new process
	int child_pid = fork();
	if (child_pid == 0) {
		ASSERT_TEST(disable_policy(getpid(), 234123) == -1 && errno == EINVAL);
		exit(0);
		return true;
	}
	wait(NULL);
	// check success
	ASSERT_TEST(disable_policy(getpid(), 234123) == 0);
	ASSERT_TEST(enable_policy(getpid(),10,234123) == 0);
	ASSERT_TEST(disable_policy(getpid(),234123) == 0);
	// test policy already off
	ASSERT_TEST(disable_policy(getpid(), 234123) == -1 && errno == EINVAL);
	return true;
}

bool test_set_process_capabilities() {
	int father_pid = getpid();
	// test pid < 0
	ASSERT_TEST(set_process_capabilities(-1, 1, 234123) == -1 && errno == ESRCH);
	// test (hopefully) pid does not exist
	ASSERT_TEST(set_process_capabilities(INT_MAX - 2, 1, 234123) == -1 && errno == ESRCH);
	// test size new_level != 0, 1, 2
	ASSERT_TEST(set_process_capabilities(father_pid, -1, 234123) == -1 && errno == EINVAL);
	ASSERT_TEST(set_process_capabilities(father_pid, 7, 234123) == -1 && errno == EINVAL);
	// test incorrect password
	ASSERT_TEST(set_process_capabilities(father_pid, 1, 0) == -1 && errno == EINVAL);
	// test policy feature is off
	ASSERT_TEST(set_process_capabilities(father_pid, 1, 234123) == -1 && errno == EINVAL);	
	// check success
	enable_policy(father_pid, 10, 234123);
	int child_pid = fork();
	if(child_pid == 0){
		int i;
		for (i = 0; i <= 2; ++i) {
			ASSERT_TEST(set_process_capabilities(father_pid, i, 234123) == 0);
		}
		exit(0);
	}
	wait(NULL);
	return true;
}

bool test_get_process_log() {
	struct forbidden_activity_info log[10];
	// test pid < 0
	ASSERT_TEST(get_process_log(-1, 2, log) == -1 && errno == ESRCH);
	// test (hopefully) pid does not exist
	ASSERT_TEST(get_process_log(INT_MAX - 2, 1, log) == -1 && errno == ESRCH);
	// test policy is set off
	ASSERT_TEST(get_process_log(getpid(), 10, log) == -1 && errno == EINVAL);
	// enable policy, level = 0
	enable_policy(getpid(), 2, 234123);
	set_process_capabilities(getpid(), 0, 234123);
	// test size < 0
	ASSERT_TEST(get_process_log(getpid(), -1, log) == -1 && errno == EINVAL);
	// test size > number of records
	ASSERT_TEST(get_process_log(getpid(), 1, log) == -1 && errno== EINVAL);
	// forbidden activities
	int child_pid = fork();
	ASSERT_TEST(child_pid == -1 && errno == EINVAL);
	wait(NULL);
	ASSERT_TEST(errno == EINVAL);
	// test size = 0
	ASSERT_TEST(get_process_log(getpid(), 0, log) == 0);
	// test size > number of records (log not affected)
	ASSERT_TEST(get_process_log(getpid(), 5, log) == -1 && errno == EINVAL);
	// check success
	ASSERT_TEST(get_process_log(getpid(), 2, log) == 0);
	ASSERT_TEST(log[0].syscall_req_level == 2 && log[1].syscall_req_level == 1);
	// test size > number of records
	ASSERT_TEST(get_process_log(getpid(), 1, log) == -1 && errno == EINVAL);
	// more forbidden activities
	wait(NULL);
	wait(NULL);
	// check success
	ASSERT_TEST(get_process_log(getpid(), 2, log) == 0);
	ASSERT_TEST(log[0].syscall_req_level == 1 && log[1].syscall_req_level == 1);
	return true;
}

bool test_all() {
	int father_pid = getpid();
	enable_policy(father_pid, 10, 234123);
	// privilege level = 2
	ASSERT_TEST(wait(NULL) == -1 && errno == ECHILD);
	ASSERT_TEST(sched_yield() == 0);
	int child_pid = fork();
	if (child_pid == 0) {
		// policy off for child
		ASSERT_TEST(disable_policy(getpid(), 234123) == -1 && errno == EINVAL);
		set_process_capabilities(father_pid, 1, 234123);
		exit(0);
	}
	ASSERT_TEST(child_pid > 0);
	// privilege level = 1
	ASSERT_TEST(wait(NULL) == child_pid);
	ASSERT_TEST(sched_yield() == 0);
	// forbidden activity #1
	ASSERT_TEST(fork() == -1 && errno == EINVAL);
	set_process_capabilities(father_pid, 2, 234123);
	// privilege level = 2
	child_pid = fork();
	if (child_pid == 0) {
		set_process_capabilities(father_pid, 0, 234123);
		exit(0);
	}
	sleep(2);
	// privilege = 0
	// forbidden activity #2
	ASSERT_TEST(wait(NULL) == -1 && errno == EINVAL);
	// forbidden activity #3
	ASSERT_TEST(sched_yield() == -1 && errno == EINVAL);
	// forbidden activity #4
	ASSERT_TEST(fork() == -1 && errno == EINVAL);
	struct forbidden_activity_info log[10];
	get_process_log(father_pid, 2, log);
	ASSERT_TEST(log[0].syscall_req_level == 2 && log[0].proc_level == 1);
	ASSERT_TEST(log[1].syscall_req_level == 1 && log[1].proc_level == 0);
	ASSERT_TEST(get_process_log(father_pid, 1, log) == 0);
	ASSERT_TEST(log[0].syscall_req_level == 1 && log[0].proc_level == 0);
	// test other sycall not affected
	ASSERT_TEST(getpid() == father_pid);
	disable_policy(father_pid, 234123);
	// policy off
	ASSERT_TEST(wait(NULL) == child_pid);
	ASSERT_TEST(sched_yield() == 0);
	child_pid = fork();
	if (child_pid == 0) {
		// policy off for child
		ASSERT_TEST(disable_policy(getpid(), 234123) == -1 && errno == EINVAL);
		enable_policy(father_pid, 5, 234123);
		exit(0);
	}
	ASSERT_TEST(child_pid > 0);
	ASSERT_TEST(wait(NULL) == child_pid);
	// policy on, privilege level = 2
	set_process_capabilities(father_pid, 0, 234123);
	// check log reset
	ASSERT_TEST(wait(NULL) == -1 && errno == EINVAL);
	get_process_log(father_pid, 1, log);
	ASSERT_TEST(log[0].syscall_req_level == 1 && log[0].proc_level == 0);
	return true;
}


int main() {
	int test_child;
	test_child = fork();
	if (test_child == 0) {
		RUN_TEST(test_enable_policy);
		return 0;
	}
	wait(NULL);
	test_child = fork();
	if (test_child == 0) {
		RUN_TEST(test_disable_policy);
		return 0;
	}
	wait(NULL);
	test_child = fork();
	if (test_child == 0) {
		RUN_TEST(test_set_process_capabilities);
		return 0;
	}
	wait(NULL);
	test_child = fork();
	if (test_child == 0) {
		RUN_TEST(test_get_process_log);
		return 0;
	}
	wait(NULL);
	test_child = fork();
	if (test_child == 0) {
		RUN_TEST(test_all);
		return 0;
	}
	wait(NULL);
	return 0;
}
