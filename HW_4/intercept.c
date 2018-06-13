#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/unistd.h>
#include <linux/syscall.h>
#include <linux/sched.h>

#define RANGE_INIT 200
#define MAX_NAME_LENGTH 16
#define EQUAL 1
#define NOT_EQUAL 0

MODULE_LICENSE("GPL");

int iScanRange = RANGE_INIT;
char *program_name;

MODULE_PARM(iScanRange , "i");
MODULE_PARM(program_name , "s");

int program_name_cmp(const char* s1, const char* s2){
    if(s1 == NULL || s2 == NULL){
        return NOT_EQUAL;
    }
    int i;
    for(i = 0; i < MAX_NAME_LENGTH; i++){
        if(s1[i] != s2[i]){
            return NOT_EQUAL;
        }
        if(s1[i] == '\0' && s2[i] == '\0'){
            break;
        }
    }
    return EQUAL;
}

void** sys_call_table = NULL;

asmlinkage long(*original_kill)(int, int);

asmlinkage long kill_wet4(int pid, int sig) {
    if (sig != SIGKILL){
        return original_kill(pid, sig);
    }
    task_t* victim = find_task_by_pid(pid);
    if (victim == NULL){
        return original_kill(pid, sig);
    }
    if(program_name_cmp(program_name,victim->comm) == NOT_EQUAL) {
        return original_kill(pid, sig);
    }
    return -EPERM;
}

void find_sys_call_table(int scan_range) {
    sys_call_table =(void**) &system_utsname;

    int i=0;
    for (i = 0; i < scan_range; i++) {
        if (sys_call_table[__NR_read] == sys_read) {//sys_call_table found
            return;
        }
        sys_call_table++;
    }
    sys_call_table = NULL;//re init sys_call_table pointer
}

int init_module(void) {
    if (program_name != NULL) {//without program name module do nothing
        find_sys_call_table(iScanRange);
        if (sys_call_table != NULL) {
            original_kill = sys_call_table[__NR_kill];//save orig sys call kill
            sys_call_table[__NR_kill] = kill_wet4;//put our sys call kill
        }
    }
    return 0;
}

void cleanup_module(void) {
    if (program_name != NULL && sys_call_table != NULL) {//restore to original ONLY if changed
        sys_call_table[__NR_kill] = original_kill;//restore original kill sys call
    }
}

