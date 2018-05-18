#ifndef HW2_SYSCALLS_H
#define HW2_SYSCALLS_H

#include <errno.h>
#include <termios.h>
#include <sys/types.h>


typedef struct {
    pid_t prev; // previous process pid
    pid_t next; // next process pid
    int prev_priority; // previous process priority
    int next_priority;// next process priority
    int prev_policy; // previous process policy
    int next_policy;// next process policy
    long switch_time; // time of context switch in jiffies
    int n_tickets; // global Number of tickets
}cs_log; //WET_2 global logger struct


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
