#ifndef HW2_SYSCALLS_H
#define HW2_SYSCALLS_H

#include <errno.h>
#include <termios.h>
#include <sys/types.h>


typedef struct {
    pid_t prev;
    pid_t next;
    int prev_priority;
    int next_priority;
    int prev_policy;
    int next_policy;
    long switch_time;
    int n_tickets;
} cs_log;



int enable_logging (int size){
    int __res;
    __asm__(
        "int $0x80;"
        :"=a"(__res)
        :"0"(243), "b" (size)
        :"memory"
    );

    if((__res) < 0){
        errno = (-__res);
        return -1;
    }
    return __res;
}


int disable_logging() {
    int __res;
    __asm__(
        "int $0x80;"
        :"=a"(__res)
        :"0"(244)
        :"memory"
    );

    if((__res) < 0){
        errno = (-__res);
        return -1;
    }
    return __res;
}


 int get_logger_records(cs_log* user_mem) {
     int __res;
     __asm__(
         "int $0x80;"
         :"=a"(__res)
         :"0"(245), "b" (user_mem)
         :"memory"
     );

     if((__res) < 0){
         errno = (-__res);
         return -1;
     }
     return __res;
 }

 int start_lottery_scheduler(){
     int __res;
     __asm__(
     "int $0x80;"
     :"=a"(__res)
     :"0"(246)
     :"memory"
     );

     if((__res) < 0){
         errno = (-__res);
         return -1;
     }
     return __res;
}

int start_orig_scheduler(){
    int __res;
    __asm__(
    "int $0x80;"
    :"=a"(__res)
    :"0"(247)
    :"memory"
    );

    if((__res) < 0){
        errno = (-__res);
        return -1;
    }
    return __res;
}

void set_max_tickets(int max_tickets){
    int __res;
    __asm__(
    "int $0x80;"
    :"=a"(__res)
    :"0"(248), "b" (max_tickets)
    :"memory"
    );

    if((__res) < 0){
        errno = (-__res);
        return;
    }
    return;
}

#endif /* HW2_SYSCALLS_H */
