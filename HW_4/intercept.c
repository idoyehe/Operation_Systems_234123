#include <linux/module.h>
#include <linux/kernel.h>
// TODO: add more #include statements as necessary

MODULE_LICENSE("GPL");

// TODO: add command-line arguments
void** sys_call_table = NULL;

// TODO: import original syscall and write new syscall

void find_sys_call_table(int scan_range) {
   // TODO: complete the function
}

int init_module(void) {
   // TODO: complete the function
}

void cleanup_module(void) {
   // TODO: complete the function
}

