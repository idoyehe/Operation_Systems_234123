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


int enable_disable_test(){
	struct forbidden_activity_info buff[10];
	int pid = getpid();
	int result = 1;
	int i=0;
	int res;
	
	// enable after already invoking enable 
	res = enable_policy(pid, 10, 234123);; 
	assert(res==0);
	res = enable_policy(pid, 10, 234123);;
	assert(res==-1);
	assert(errno==EINVAL);
	res = disable_policy(pid, 234123);
	assert(res==0);
	
	// disable after already invoking disable
	res = enable_policy(pid, 10, 234123);; 
	assert(res==0);
	res = disable_policy(pid, 234123);
	assert(res==0);
	res = disable_policy(pid, 234123);
	assert(res==-1);
	assert(errno==EINVAL);
	
	// sys_enable_tests
	res = enable_policy(-1,10, 234123);; //negative pid
	assert(res==-1);
	assert(errno==ESRCH);
	res = enable_policy(33000, 10, 234123); //no such proc
	assert(res==-1);
	assert(errno==ESRCH);
	res = enable_policy(pid, -1, 234123);; //negative size
	assert(res==-1);
	assert(errno==EINVAL);
	res = enable_policy(pid, 10, 234120);; //wrong password
	assert(res==-1);
	assert(errno==EINVAL);
	enable_policy(-1, -1, 234123);; //negative pid and size
	assert(res==-1);
	assert(errno==ESRCH);
	
	//size=0 test
	res = enable_policy(pid, 0, 234123);
	assert(res==0);
	res = get_process_log(pid, 0, buff);
	assert(res==0);
	res = disable_policy(pid, 234123);
	assert(res==0);
	
	//normal size en dis test
	res = enable_policy(pid, 10, 234123);
	assert(res==0);
	res = disable_policy(pid, 234123);
	assert(res==0);
	
	// sys_disable_tests
	res = disable_policy(-1, 234123); // negative pid
	assert(res==-1);
	assert(errno==ESRCH);
	res = disable_policy(33000, 234123); //no such proc
	assert(res==-1);
	assert(errno==ESRCH);
	res = disable_policy(pid, 234120); //wrong password
	assert(res==-1);
	assert(errno==EINVAL);
	res = disable_policy(pid, 234123); //disable after invoking disable
	assert(res==-1);
	assert(errno==EINVAL);
	
}

int simple_get_test(){
	struct forbidden_activity_info buff[10];
	int pid = getpid();
	int result = 1;
	int i=0;
	int res;
	
	res = get_process_log(-1, 5, buff); //negative pid
	assert(res==-1);
	assert(errno==ESRCH);
	res = get_process_log(33000, 5, buff); //no such proc
	assert(res==-1);
	assert(errno==ESRCH);
	res = get_process_log(pid, 11, buff); //size>log_size
	assert(res==-1);
	assert(errno==EINVAL);
	res = get_process_log(pid, -1, buff); //negative size
	assert(res==-1);
	assert(errno==EINVAL);
	res = get_process_log(pid, 5, buff); //logging isn't enabled
	assert(res==-1);
	assert(errno==EINVAL);
	res = get_process_log(-1, -1, buff); //negative pid and size
	assert(res==-1);
	assert(errno==ESRCH);
	res = get_process_log(-1, 11, buff); //negative pid and size bigger than log_size
	assert(res==-1);
	assert(errno==ESRCH);

	//not enough logs yet
	enable_policy(pid, 10, 243123);
	
	res = get_process_log(pid, 7, buff);
	assert(res==-1);
	assert(errno==EINVAL);
	
	res = enable_policy(pid, 10, 234123);
	assert(res==0);
	res = set_process_capabilities(pid, 0, 234123);
	assert(res==0);
	errno = -1;
	res = get_process_log(pid, 1, buff);
	assert(res==-1);
	int fork_res = fork();
	if(fork_res == 0){
		printf("\n fork should never work\n");
	}
	assert(fork_res==-1);
	assert(errno==EINVAL);
	res = get_process_log(pid, 1, buff);
	//printf("\n req_level = %d proc_level = %d\n", buff[0].syscall_req_level, buff[0].proc_level);
	assert(res==0);
	assert(buff[0].syscall_req_level == 2);
	assert(buff[0].proc_level == 0);
	res = get_process_log(pid, 1, buff);
	assert(res==-1);
	
	int wait_res = wait(NULL);
	assert(wait_res==-1);
	assert(errno==EINVAL);
	res = get_process_log(pid, 1, buff);
	assert(res==0);
	assert(buff[0].syscall_req_level == 1);
	assert(buff[0].proc_level == 0);
	
	res = get_process_log(pid, 1, buff);
	assert(res==-1);
	
	wait_res = wait(NULL);
	assert(wait_res==-1);
	assert(errno==EINVAL);
	
	fork_res = fork();
	if(fork_res == 0){
		printf("\n fork should never work\n");
	}
	assert(fork_res==-1);
	assert(errno==EINVAL);
	
	res = get_process_log(pid, 2, buff);
	assert(res==0);
	
	assert(buff[0].syscall_req_level == 1);
	assert(buff[0].proc_level == 0);
	
	assert(buff[1].syscall_req_level == 2);
	assert(buff[1].proc_level == 0);
	
	disable_policy(pid, 234123);
	return result;
}

/*
//enabling logging without disable - exit() check
int enable_no_disable(){
	struct forbidden_activity_info buff[10];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	enable_policy(pid,10);
	return result;
}
*/

int set_capabilities_test(){
	struct forbidden_activity_info buff[10];
	int pid = getpid();
	int result = 1;
	int i=0;
	int res;
	
	res = enable_policy(pid, 10, 234123);
	assert(res==0);
	
	res = set_process_capabilities(-1, 1, 234123);
	assert(res==-1);
	assert(errno==ESRCH);
	res = set_process_capabilities(33000, 1, 234123);
	assert(res==-1);
	assert(errno==ESRCH);
	res = set_process_capabilities(pid, 3, 234123);
	assert(res==-1);
	assert(errno==EINVAL);
	res = set_process_capabilities(pid, -1, 234123);
	assert(res==-1);
	assert(errno==EINVAL);
	res = set_process_capabilities(pid, 1, 234120);
	assert(res==-1);
	assert(errno==EINVAL);
	
	res = disable_policy(pid, 234123);
	assert(res==0);
	res = set_process_capabilities(pid, 1, 234123);
	assert(res==-1);
	assert(errno==EINVAL);
	
	res = enable_policy(pid, 10, 234123);
	assert(res==0);
	
	int wait_res = wait(NULL);
	assert(wait_res==-1);
	//assert(errno==WNOHANG);
	     
	res = get_process_log(pid, 1, buff);
	assert(res==-1);
	assert(errno==EINVAL);
	
	res = disable_policy(pid, 234123);
	assert(res==0);
	
	return result;
	
}

int fork_test(){
	//grandfather
	struct forbidden_activity_info buff[4];
	int pid= getpid(); 
	int result = 1;
	int i=0;
	int res;
	res = enable_policy(pid,4,234123);
	assert(res==0);
	
	int fork_res1 = fork(); //2
	
	if(fork_res1==0){
		//father
		int son_pid = getpid();
		res = disable_policy(son_pid,234123);
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_process_log(son_pid, 4, buff);
			assert(res==-1);
			assert(errno==EINVAL);		
		res = enable_policy(son_pid,4,234123); //243
		assert(res==0);
		int father_pid = getppid();
			
			
			
			res = set_process_capabilities(son_pid, 0, 234123);
			assert(res==0);
			res = get_process_log(son_pid, 3, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			res = wait(NULL);
			assert(res==-1);
			assert(errno==EINVAL);
			res = sched_yield();
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_process_log(son_pid, 2, buff);
			assert(res==0);
			assert(buff[0].syscall_req_level == 1);
			assert(buff[0].proc_level == 0);
			assert(buff[1].syscall_req_level == 1);
			assert(buff[1].proc_level == 0);
			res = set_process_capabilities(son_pid, 1, 234123);
			assert(res==0);
			res = fork();
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_process_log(son_pid, 2, buff);
			assert(res==-1);
			res = get_process_log(son_pid, 1, buff);
			assert(res==0);
			assert(buff[0].syscall_req_level == 2);
			assert(buff[0].proc_level == 1);
			res = get_process_log(son_pid, 3, buff);
			assert(res==-1);
			
			res = set_process_capabilities(son_pid, 0, 234123);
			res = sched_yield();
			assert(res==-1);
			assert(errno==EINVAL);
			res = fork();
			assert(res==-1);
			assert(errno==EINVAL);
			res = get_process_log(son_pid, 2, buff);
			assert(res==0);
			assert(buff[0].syscall_req_level == 1);
			assert(buff[0].proc_level == 0);
			assert(buff[0].syscall_req_level == 1);
			assert(buff[0].proc_level == 0);
			
			
			res = enable_policy(pid,4,234123); //243
			assert(res==-1);
			res = get_process_log(pid, 1, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			
			res = disable_policy(son_pid,234123);
			assert(res==0);
			
			exit(0);
		} else{
			//father
			wait(NULL);
			assert(res==0);
			res = set_process_capabilities(pid,1, 234123);
			res = enable_policy(pid,4,234123); //243
			assert(res==-1);
			res = get_process_log(pid, 1, buff);
			assert(res==-1);
			assert(errno==EINVAL);
			
			res = set_process_capabilities(pid, 0, 234123);
			assert(res==0);
			
			res = waitpid(33000, NULL, 0);
			assert(res==-1);
			assert(errno==EINVAL);
			
			res = get_process_log(pid, 2, buff);
			assert(res==-1);
			res = get_process_log(pid, 1, buff);
			assert(res==0);
			assert(buff[0].syscall_req_level == 1);
			assert(buff[0].proc_level == 0);
			res = disable_policy(pid,234123);
			assert(res==0);
		}
		
	
	return result;
}


int zombie_check(){
	struct forbidden_activity_info buff[4];
	int pid= getpid();
	int result = 1;
	int i=0;
	int res;
	res = enable_policy(pid,4,234123); //243
	assert(res==0);
	res = set_process_capabilities(pid, 0, 234123);
	res = sched_yield();
	assert(res==-1);
	assert(errno==EINVAL);
	res = fork();
	assert(res==-1);
	assert(errno==EINVAL);
	res = set_process_capabilities(pid, 2, 234123);
	int fork_res1 = fork(); //2

	if(fork_res1==0){
		int son_pid = getpid();
		res = enable_policy(son_pid,4,234123); //243
		assert(res==0);
		res = set_process_capabilities(son_pid, 0, 234123);
		assert(res==0);
		res = fork();
		assert(res==-1);
		assert(errno==EINVAL);
		res = sched_yield();
		assert(res==-1);
		assert(errno==EINVAL);
		
	} else{
		sleep(3); // wait till son dies and becomes zombie
		
		res = get_process_log(fork_res1, 2, buff);
		assert(res==0);
		assert(buff[0].syscall_req_level == 2);
		assert(buff[0].proc_level == 0);
		assert(buff[1].syscall_req_level == 1);
		assert(buff[1].proc_level == 0);
		
		res = get_process_log(pid, 2, buff);
		assert(res==0);
		assert(buff[0].syscall_req_level == 1);
		assert(buff[0].proc_level == 0);
		assert(buff[1].syscall_req_level == 2);
		assert(buff[1].proc_level == 0);
		res = disable_policy(pid,234123);
		assert(res==0);
		res = wait(NULL);
		assert(res != -1);

	}
	
	return result;
}

int main(){

	printf("Test begins\n");
	RUN_TEST(enable_disable_test);
	RUN_TEST(simple_get_test);
	RUN_TEST(set_capabilities_test);
	RUN_TEST(fork_test);
	RUN_TEST(zombie_check);
	printf("Test passed! :)\n");
	return 0;
}

