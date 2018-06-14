#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>

#define NO_FILE 33

int main(int argc, char* argv[]) {
    pid_t son1 = fork();
    if(son1==0)
    {
        execv("killed1G.out",NULL);
    	return NO_FILE;
	}
	pid_t son2 = fork();
	if(son2==0)
    {
        execv("killed2G.out",NULL);
		return NO_FILE;
    }
    sleep(2);
    int res1,res2;
    if(argc>=2 && argv[1][0]=='T')
        res1 = kill(son1,SIGTERM);
	else{
        res1 = kill(son1,SIGKILL);
		if(argv[1][1] == 'N' && (!(res1<0 && errno==EPERM )))
		printf("Wrong error code");
    }
    if(argc>=3 && argv[2][0]=='T')
        res2 = kill(son2,SIGTERM);
    else{
        res2 = kill(son2,SIGKILL);
		if(argv[2][1] == 'N' && (!(res2<0 && errno==EPERM )))
		printf("Wrong error code");
    }
    waitpid(son1, &res1, 0);
	waitpid(son2, &res2, 0);
	if(WIFEXITED(res1))
		res1 =WEXITSTATUS(res1); //fixing NO_FILE error code
	if(WIFEXITED(res2))
		res2 =WEXITSTATUS(res2); //fixing NO_FILE error code
    printf("%d%d", res1, res2);
    return 0;
}
