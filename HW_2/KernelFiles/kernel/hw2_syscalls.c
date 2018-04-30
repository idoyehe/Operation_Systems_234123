#include <linux/sched.h>
#include <linux/slab.h>
#include <asm-i386/uaccess.h>

/*wet2 logger globals*/
SWITCH logger_enable = OFF;
int log_size = 0;
int log_index = 0;
cs_log* log_arr = NULL;

SWITCH sched_lottery_enable = OFF;
SWITCH chnged_sched = OFF;
int total_processes_tickets = 0;
int max_tickets = 0;
int NT = 0;



int sys_enable_logging(int size){
    printk("Welcome to sys_enable_logging\n");
    if(logger_enable == ON){
        printk("logger is already enable\n");
        return -EINVAL;
    }

    if(size < 0){
        printk("size is negative\n");
        return -EINVAL;
    }

    kfree(log_arr);//free previous log if exist
    printk("free prev log\n");
    log_arr = (cs_log*)kmalloc(size * (sizeof(cs_log)),GFP_KERNEL);
    if (log_arr == NULL){
        return -ENOMEM;
    }
    printk("allocating new log\n");
    logger_enable = ON;
    log_size = size;
    log_index = 0;
    printk("init new log parameters\n");
    return 0;
}

int sys_disable_logging() {
    printk("Welcome to sys_disable_logging\n");
    if(logger_enable == OFF){
        printk("logger is already disable\n");
        return -EINVAL;
    }
    logger_enable = OFF;
    printk("now logger is disable\n");
    return 0;
}

int sys_get_logger_records(cs_log* user_mem) {
    printk("Welcome to sys_get_logger_records\n");
    if (user_mem == NULL) {
        printk("logger arr is NULL\n");
        log_index = 0;
        return -ENOMEM;
    }

    int i;
    for (i = 0; i < log_index; i++) {
        if (copy_to_user(&(user_mem[i]),&(log_arr[i]), sizeof(cs_log)) != 0) {
            log_index = 0;
            return -ENOMEM;
        }
        printk("finish copy to user the index: %d\n", i);

    }

    log_index = 0;
    printk("finish copy to user all logger\nlog_index is: %d\n", log_index);
    return 0;
}

int sys_start_lottery_scheduler(){
    printk("Welcome to sys_start_lottery_scheduler\n");
    if(sched_lottery_enable == ON){
        chnged_sched = OFF;
        printk("lottery_scheduler is already enable\n");
        return -EINVAL;
    }
    sched_lottery_enable = ON;
    chnged_sched = ON;
    printk("lottery_scheduler is enable\n");
    return 0;
}


int sys_start_orig_scheduler(){
    printk("Welcome to sys_start_orig_scheduler\n");
    if(sched_lottery_enable == OFF){
        printk("lottery_scheduler is already disable\n");
        return -EINVAL;
    }
    sched_lottery_enable = OFF;
    chnged_sched = ON;
    printk("lottery_scheduler is disable\n");
    return 0;
}

int sys_set_max_tickets(int max_tickets){
    printk("Welcome to sys_set_max_tickets\n");
    if (max_tickets <= 0 || max_tickets > total_processes_tickets) {
        NT = total_processes_tickets;
        printk("NT is total_processes_tickets, value: %d\n",NT);

    } else {
        NT = max_tickets;
        printk("NT is max_tickets, value: %d\n", NT);
    }
}
