#include<linux/sched.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm-i386/uaccess.h>

#define PSW 234123

typedef struct forbidden_activity_info{
    int syscall_req_level;
    int proc_level;
    int time;
}forbidden_activity_info;

int sys_enable_policy (pid_t pid ,int size, int password){
    if(pid < 0){
        return -ESRCH;
    }
    task_t* prc = find_task_by_pid(pid);
    if(prc  == NULL){
        return -ESRCH;
    }
    if(password != PSW){
        return -EINVAL;
    }
    if(prc->pclSw == ENABLE_PCL){
        return -EINVAL;
    }
    if(size < 0){
        return -EINVAL;
    }
    //allocating new log head with dummy head
    prc->log_head = (list_t*)kmalloc(sizeof(*(prc->log_head)),GFP_KERNEL);
    if(prc->log_head == NULL){
        return -ENOMEM;
        //kmalloc failed
    }
    INIT_LIST_HEAD(prc->log_head);//initializing cyclic list
    prc->log_size = 0;//initializing forbidden activity log list size 0
    prc->pclSw = ENABLE_PCL;//mark process policy is ENABLE
    prc->prvl_lvl = HIGH_PRVL;//mark process privilege to HIGH [2]
    return 0;
}

int sys_disable_policy (pid_t pid ,int password) {
    if(pid < 0){
        return -ESRCH;
    }
    task_t* prc = find_task_by_pid(pid);
    if(prc  == NULL){
        return -ESRCH;
    }
    if(prc->pclSw == DISABLE_PCL){
        return -EINVAL;
    }
    if(password != PSW){
        return -EINVAL;
    }

    prc->pclSw = DISABLE_PCL;//mark process policy is DISABLE
    forbidden_activity_info_list * entry_temp = NULL;
    list_t *node_temp = NULL;
    list_for_each(node_temp,prc->log_head){
        entry_temp = list_entry(node_temp, forbidden_activity_info_list,my_ptr);
        list_del(node_temp);
        kfree(entry_temp);
    }
    kfree(prc->log_head);//free dummy log list's head
    prc->log_head = NULL;//mark log list pointer to NULL
    prc->log_size = 0;//mark no log for this process
    return 0;
}

int sys_set_process_capabilities(pid_t pid, int new_level, int password) {
    if(pid < 0){
        return -ESRCH;
    }
    task_t* prc = find_task_by_pid(pid);
    if(prc  == NULL){
        return -ESRCH;
    }
    if(new_level != HIGH_PRVL && new_level != MID_PRVL && new_level != LOW_PRVL){
        return -EINVAL;
    }
    if(password != PSW){
        return -EINVAL;
    }
    if(prc->pclSw == DISABLE_PCL){
        return -EINVAL;
    }

    prc->prvl_lvl = new_level;//change process privilege to given
    return 0;
}

int sys_get_process_log(pid_t pid,int size,struct forbidden_activity_info* user_mem){
    if(pid < 0){
        return -ESRCH;
    }
    task_t* prc = find_task_by_pid(pid);
    if(prc  == NULL){
        return -ESRCH;
    }
    if(size < 0 || size > prc->log_size){
        return -EINVAL;
    }
    if(prc->pclSw == DISABLE_PCL){
        return -EINVAL;
    }

    forbidden_activity_info_list *entry_temp_list = NULL;
    list_t * node_temp = NULL;
    int usr_index = 0;
    list_for_each(node_temp,prc->log_head){
        if(usr_index == size){
            break;
            //break from loop for when copy to user the number of requested log
        }
        entry_temp_list = list_entry(node_temp, struct forbidden_activity_info_list,my_ptr);
        copy_to_user(&(user_mem[usr_index++]),entry_temp_list, sizeof(forbidden_activity_info));// exporting to user memory
        list_del(node_temp);//remove from list the activity
        kfree(entry_temp_list);//free the activity
        prc->log_size--;//decrease log list size
    }
    return 0;
}
