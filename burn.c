// burn by Angrave - a simple test program for shell testing that burns up CPU resources for a limited number of seconds
// NCSA Open Source (<a href="http://opensource.org/licenses/NCSA">http://opensource.org/licenses/NCSA</a>)  Copyright L Angrave 2015
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
// gcc -Wall burn.c -o burn
int stop;
char MESG[100];

void done(int signal) {
    write(1, MESG, strlen(MESG));
    stop = 1;
}

int main(int argc, char**argv) {
    int seconds = 2;
    if (argc > 1) 
        sscanf(argv[1], "%d", &seconds);
    if(seconds < 1 || seconds > 9) 
        seconds = 2;

    int quitsignal = 0;
    if(argc > 2) 
        sscanf(argv[2], "%d", &quitsignal);
    
    printf("%d: Running for %d seconds\n", getpid(), seconds);
    sprintf(MESG, "%d: Signalled\n", getpid());
    signal(SIGALRM, done);
    alarm(seconds);

    while(!stop) {  }
    signal(SIGALRM, SIG_DFL); // reset, in case user wants to exit with SIGALRM
    printf("%d: Sleeping for a bit...\n",getpid());
    sleep(5);
    printf("%d: Parent process is %d\n",getpid(), getppid());
    printf("%d: Quitting\n",getpid());

    // could also use raise() to send a signal to yourself
    if(quitsignal) {
        printf("%d: ... with signal %d\n", getpid(), quitsignal);
        kill(getpid(),quitsignal);
    }

    return 0;
}