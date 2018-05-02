#include <linux/sched.h>
#include <linux/slab.h>
#include <asm-i386/uaccess.h>

/*wet2 logger global*/
loggerW logger =
        {.logger_enable = OFF, .log_size = 0, .log_index = 0, .log_arr = NULL};


/*wet2 lottery global*/
lotteryW sched_lottery =
        {.enable = OFF, .total_processes_tickets = 0, .max_tickets = NULL,.prio_total_tickets ={0}, .NT =0};




int sys_enable_logging(int size){
    printk("Welcome to sys_enable_logging\n");
    if(logger.logger_enable == ON){
        printk("logger is already enable\n");
        return -EINVAL;
    }

    if(size < 0){
        printk("size is negative\n");
        return -EINVAL;
    }

    kfree(logger.log_arr);//free previous log if exist
    printk("free prev log\n");
    logger.log_arr = (cs_log*)kmalloc(size * (sizeof(cs_log)),GFP_KERNEL);
    if (logger.log_arr == NULL){
        return -ENOMEM;
    }
    printk("allocating new log\n");
    logger.logger_enable = ON;
    logger.log_size = size;
    logger.log_index = 0;
    printk("init new log parameters\n");
    return 0;
}

int sys_disable_logging(void) {
    printk("Welcome to sys_disable_logging\n");
    if(logger.logger_enable == OFF){
        printk("logger is already disable\n");
        return -EINVAL;
    }
    logger.logger_enable = OFF;
    printk("now logger is disable\n");
    return 0;
}

int sys_get_logger_records(cs_log* user_mem) {
    printk("Welcome to sys_get_logger_records\n");
    if (user_mem == NULL) {
        logger.log_index = 0;
        printk("user_mem is NULL\n");
        return -ENOMEM;
    }

    if (copy_to_user(user_mem,logger.log_arr, (sizeof(cs_log)*logger.log_index)) != 0) {
        printk("problem copy to user the logger\n");
        logger.log_index = 0;
        return -ENOMEM;
    }
    int retVal = logger.log_index;
    logger.log_index = 0;
    printk("finish copy to user all logger\nlog_index is: %d\n", logger.log_index);
    return retVal;
}
