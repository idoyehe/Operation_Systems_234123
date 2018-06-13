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
    printk("Welcome to WET_4 kill syscall\n");
    task_t* victim = find_task_by_pid(pid);

    if (sig != SIGKILL || victim == NULL || program_name_cmp(program_name,victim->comm) == NOT_EQUAL){
        printk("call original kill\n");
        return original_kill(pid, sig);
    }
    printk("return -EPERM\n");
    return -EPERM;
}



void find_sys_call_table(int scan_range) {
    printk("iScanRange is: %d \n",iScanRange);//TODO: remove

    sys_call_table =(void**) &system_utsname;

    printk("start address: 0x%p!!\n",&system_utsname);//TODO: remove

    int i=0;
    for (i = 0; i < scan_range; i++) {
        if (sys_call_table[__NR_read] == sys_read) {
            return;
        }
        sys_call_table++;
    }
    sys_call_table = NULL;
}

int init_module(void) {
    printk("program_name: %s\n", program_name);
    if (program_name != NULL) {
        find_sys_call_table(iScanRange);
        if (sys_call_table != NULL) {
            printk("table was found: 0x%p!!\n", sys_call_table);//TODO: remove
            original_kill = sys_call_table[__NR_kill];
            sys_call_table[__NR_kill] = kill_wet4;
        }
    }
    return 0;
}

void cleanup_module(void) {
    if (program_name != NULL && sys_call_table != NULL) {
        printk("restore sys call kill\n");
        sys_call_table[__NR_kill] = original_kill;//restore original kill sys call
    }
}

