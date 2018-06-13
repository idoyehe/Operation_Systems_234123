#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int main(int argc, char* argv[]) {
    pid_t son1 = fork();
    if(son1==0)
    {
        execv("killed1G.out",NULL);
    }
	pid_t son2 = fork();
	if(son2==0)
    {
        execv("killed2G.out",NULL);
    }
    sleep(2);
    int res1,res2;
    if(argc>=2 && argv[1][0]=='1')
        res1 = kill(son1,SIGTERM);
    else
        res1 = kill(son1,SIGKILL);
    if(argc>=3 && argv[2][0]=='1')
        res2 = kill(son2,SIGTERM);
    else
        res2 = kill(son2,SIGKILL);

    waitpid(son1, &res1, 0);
	waitpid(son2, &res2, 0);
    printf("%d%d", res1, res2);
    return 0;
}
