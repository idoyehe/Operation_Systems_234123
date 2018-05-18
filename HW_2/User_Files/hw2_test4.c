#include <limits.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>


#include "hw2_syscalls.h"
#include "test_utilities.h"

#define LOG_SIZE 100


#define USER_LIMIT 20
#define USER_LIMIT_ON 0

#define MAX_TICKETS ((USER_LIMIT)*(USER_LIMIT_ON))


int main() {
    disable_logging();//make sure Logger is OFF
    ASSERT_TEST(disable_logging() == -1);//make sure Logger is OFF
    ASSERT_TEST(start_orig_scheduler() == -1);//make sure Lottery is OFF
    printf("LOTTERY is OFF!!\n");
    ASSERT_TEST(enable_logging(0) == 0);
    ASSERT_TEST(disable_logging() == 0);//make sure Logger is OFF

    ASSERT_TEST(enable_logging(LOG_SIZE) == 0);
    ASSERT_TEST(enable_logging(LOG_SIZE) == -1);
    printf("LOGGING is ON!!\n");

    return 0;
}
