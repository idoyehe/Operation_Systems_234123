#include <linux/sched.h>
#include <linux/slab.h>
#include <asm-i386/uaccess.h>

/*wet2 logger global*/
loggerW logger = {
        .logger_enable = OFF,
        .log_size = 0,
        .log_index = 0,
        .log_arr = NULL};


/*wet2 lottery global*/
lotteryW sched_lottery = {
        .enable = OFF,
        .processes_all_tickets = 0,
        .user_max_tickets = 0,
        .tickts_per_prio = {0},
        .NT = 0};



int sys_enable_logging(int size){
    if(logger.logger_enable == ON){
        return -EINVAL;
    }

    if(size < 0){
        return -EINVAL;
    }

    kfree(logger.log_arr);//free previous log if exist
    logger.log_arr = (cs_log*)kmalloc(size * (sizeof(cs_log)),GFP_KERNEL);
    if (logger.log_arr == NULL){
        return -ENOMEM;
    }
    logger.logger_enable = ON;
    logger.log_size = size;
    logger.log_index = 0;
    return 0;
}

int sys_disable_logging(void) {
    if(logger.logger_enable == OFF){
        return -EINVAL;
    }
    logger.logger_enable = OFF;
    return 0;
}

int sys_get_logger_records(cs_log* user_mem) {
    if (user_mem == NULL) {
        logger.log_index = 0;
        return -ENOMEM;
    }

    if (copy_to_user(user_mem,logger.log_arr, (sizeof(cs_log)*logger.log_index)) != 0) {
        logger.log_index = 0;
        return -ENOMEM;
    }
    int retVal = logger.log_index;
    logger.log_index = 0;
    return retVal;
}
