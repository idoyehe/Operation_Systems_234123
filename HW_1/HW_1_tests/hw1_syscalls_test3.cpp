#include "hw1_syscalls.h"
#include "staticQueue.hpp"
#include "testsMacros.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/wait.h>

int enable_disable_test() {
    int pid = getpid();
    // Enable after already invoking enable
    assert(enable_policy(pid, 10, 234123) == 0);
    assert(enable_policy(pid, 10, 234123) == -1);
    assert(errno == EINVAL);
    assert(disable_policy(pid, 234123) == 0);
    // Checking enable policy's error messages
    assert(enable_policy(pid, 10, 234122) == -1);
    assert(errno == EINVAL);
    assert(enable_policy(-9, 10, 234123) == -1);
    assert(errno == ESRCH);
    assert(enable_policy(234123, 10, 234123) == -1);
    assert(errno == ESRCH);
    assert(enable_policy(pid, -5, 234123) == -1);
    assert(errno == EINVAL);
    // Checking disable policy's error messages
    assert(disable_policy(pid, 234123) == -1);
    assert(errno == EINVAL);
    assert(disable_policy(-2, 234123) == -1);
    assert(errno == ESRCH);
    assert(disable_policy(234123, 234123) == -1);
    assert(errno == ESRCH);
    assert(disable_policy(pid, 234122) == -1);
    assert(errno == EINVAL);
    return 1;
}

int set_process_capabilities_test()
{
    // Checking set's error messages
    int pid = getpid();
    assert(set_process_capabilities(pid, 2, 234123) == -1);
    assert(errno == EINVAL);
    assert(enable_policy(pid, 10, 234123) == 0);
    assert(set_process_capabilities(-1, 2, 234123) == -1);
    assert(errno == ESRCH);
    assert(set_process_capabilities(234123, 2, 234123) == -1);
    assert(errno == ESRCH);
    assert(set_process_capabilities(pid, -1, 234123) == -1);
    assert(errno == EINVAL);
    assert(set_process_capabilities(pid, 3, 234123) == -1);
    assert(errno == EINVAL);
    assert(set_process_capabilities(pid, 2, 234122) == -1);
    assert(errno == EINVAL);
    // Setting the process capabilities to 0
    assert(set_process_capabilities(pid, 0, 234123) == 0);
    // Adding a forbidden activity
    assert(fork() == -1);
    assert(errno == EINVAL);
    assert(disable_policy(pid, 234123) == 0);
    // Adding another forbidden activity
    assert(enable_policy(pid, 10, 234123) == 0);
    assert(set_process_capabilities(pid, 0, 234123) == 0);
    assert(fork() == -1);
    assert(disable_policy(pid, 234123) == 0);
    return 1;
}

int get_process_log_test() {
    struct forbidden_activity_info buff[4];
    int pid = getpid();
    // Filling the array with forbidden activities
    assert(enable_policy(pid, 10, 234123) == 0);
    assert(set_process_capabilities(pid, 0, 234123) == 0);
    assert(fork() == -1);
	assert(errno == EINVAL);
    assert(sched_yield() == -1);
    assert(errno == EINVAL);
    assert(wait(NULL) == -1);
    assert(errno == EINVAL);
    assert(wait(NULL) == -1);
    assert(errno == EINVAL);
    assert(get_process_log(pid, 3, buff) == 0);
    assert(buff[0].syscall_req_level == 2);
    assert(buff[1].syscall_req_level == 1);
    assert(buff[2].syscall_req_level == 1);
    assert(buff[0].proc_level == 0);
    assert(buff[1].proc_level == 0);
    assert(buff[2].proc_level == 0);
    assert(buff[0].time <= buff[1].time);
    assert(buff[1].time <= buff[2].time);
    // Checking get's error messages
    assert(get_process_log(-1, 1, buff) == -1);
    assert(errno == ESRCH);
    assert(get_process_log(234123, 1, buff) == -1);
    assert(errno == ESRCH);
    assert(get_process_log(pid, -1, buff) == -1);
    assert(errno == EINVAL);
    assert(get_process_log(pid, 2, buff) == -1);
    assert(errno == EINVAL);
    // Making sure that after disabling the process, that it's array/list is empty
    assert(disable_policy(pid, 234123) == 0);
    assert(get_process_log(pid, 1, buff) == -1);
    assert(errno == EINVAL);
    assert(enable_policy(pid, 10, 234123) == 0);
    assert(get_process_log(pid, 1, buff) == -1);
    assert(errno == EINVAL);
    assert(disable_policy(pid, 234123) == 0);
    return 1;
}

int fork_test() {
    struct forbidden_activity_info buff[4];
    int pid = getpid();
    assert(enable_policy(pid, 10, 234123) == 0);
    assert(set_process_capabilities(pid, 0, 234123) == 0);
    assert(wait(NULL) == -1);
    assert(errno == EINVAL);
    assert(set_process_capabilities(pid, 2, 234123) == 0);
    int p = fork();
    if (p == 0)
    {
        int child = getpid();
        // Checking that the child's flag is down by trying to disable it
        assert(disable_policy(child, 234123) == -1);
        assert(errno == EINVAL);
        assert(enable_policy(child, 10, 234123) == 0);
        // Checking that the child's list/array is empty
        assert(get_process_log(child, 1, buff) == -1);
        assert(errno == EINVAL);
        // Checking that fork works to check if the capabilities level is 2
        int res = fork();
        if (res == 0)
            exit(0);
        else if (res > 0)
        {
            wait(NULL);
            assert(disable_policy(child, 234123) == 0);
            exit(0);
        }
        else
            printf("You forgot to reset the childs cap_level to 2");
    }
    else
    {
        wait(NULL);
        assert(disable_policy(pid, 234123) == 0);

    }
    return 1;
}

int main(){
	printf("Test begins\n");
    RUN_TEST(enable_disable_test);
    RUN_TEST(set_process_capabilities_test);
    RUN_TEST(get_process_log_test);
    RUN_TEST(fork_test);
	printf("Test passed! :)\n");
	return 0;
}
