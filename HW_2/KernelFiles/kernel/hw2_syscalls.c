#include <linux/sched.h>
#include <linux/slab.h>
#include <asm-i386/uaccess.h>

int sys_enable_logging (int size){
    if(logger_enable == true){
        return -EINVAL;
    }

    if(size < 0){
        return -EINVAL;
    }

    kfree(log_arr);//free previous log if exist

    log_arr = (cs_log*)kmalloc(size * (sizeof(cs_log)),GFP_KERNEL);
    if (log_arr == NULL){
        return -ENOMEM;
    }
    logger_enable = true;
    log_size = size;
    log_index = 0;
    return 0;
}

int sys_disable_logging() {
    if(logger_enable == false){
        return -EINVAL;
    }
    logger_enable = false;
    return 0;
}

int sys_get_logger_records(cs_log* user_mem) {
    if (user_mem == NULL) {
        return -ENOMEM;
    }
    int i;
    for (i = 0; i < log_index; i++) {
        if (copy_to_user(&(user_mem[i]), &(log_arr[i]), sizeof(cs_log)) != 0) {
            return -ENOMEM;
        }
    }
    log_index = 0;
    return 0;
}
