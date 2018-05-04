#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>


#include "hw2_syscalls.h"
#include "test_utilities.h"

#define LOG_SIZE 200



int main() {
    enable_logging(0);
    disable_logging();//make sure Logger is OFF
    start_orig_scheduler();//make sure Lottery is OFF

    ASSERT_TEST(enable_logging(LOG_SIZE) == 0);
    ASSERT_TEST(start_lottery_scheduler () == 0);

    sleep(300);

    ASSERT_TEST(start_orig_scheduler () == 0);
    ASSERT_TEST(disable_logging() == 0);
    ASSERT_TEST(enable_logging(0) == 0);
    ASSERT_TEST(disable_logging() == 0);
    return 0;
}
