#include <limits.h>
#include <stdlib.h>
#include <sched.h>

#include "hw2_syscalls.h"
#include "test_utilities.h"

int main() {

	start_orig_scheduler();
	set_max_tickets(0);
	ASSERT_TEST(start_lottery_scheduler() == 0);
	ASSERT_TEST(start_lottery_scheduler() == -1 && errno == EINVAL);
	ASSERT_TEST(start_orig_scheduler() == 0);
	ASSERT_TEST(start_orig_scheduler() == -1 && errno == EINVAL);



	return 0;
}
