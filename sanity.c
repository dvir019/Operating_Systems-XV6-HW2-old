// test a process scheduling implemention by running user space program.

//https://www.cs.bgu.ac.il/~os162/wiki.files/Assignment1-new.pdf

/*
 * to follow with the debugger into the child process:
 * https://stackoverflow.com/questions/36221038/how-to-debug-a-forked-child-process-using-clion

Set a break point at the beginning of your program (ie. the parent program, not the child program).

Start the program in the debugger.

Go to the debugger console (tab with the label gdb) in clion and enter set follow-fork-mode child and set detach-on-fork off.

Continue debugging.
 */


static int NUM_ITERATIONS = 10000000;
static int NUM_LOOPS=10;


#ifdef __linux__xxx
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
//#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <string.h>
#define EXIT() exit(0)
#define pr printf
static void yield(){ sleep(0);}

/** MOCKUP !
 wait for child process termination
 retime: The aggregated number of clock ticks during which the process waited(was able to run but did not get CPU)
 rutime: The aggregated number of clock ticks during which the process was running
 stime:  The aggregated number of clock ticks during which the process was waiting for I/O (was not able to run).
 return: pid of child  or -1
*/
int wait2(int *retime, int *rutime, int *stime, int* endtime){
    static const int micro_seconds_to_ticks = 10000;
    int status,options =0;
    struct rusage usage;
    int ret = wait3(&status,options, &usage);
    *retime = (usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec)* micro_seconds_to_ticks;
    //*rutime = usage.
    *stime = 0;
    *endtime = 0;//proc->end_time;
    return ret;
}
#else
#include "types.h"
#include "user.h"
#define assert(expr) if(!(expr)){ printf(2,#expr);exit();} //TODO move to general header file
#define EXIT() exit()

#endif

double volatile junk = 0.0;

int run_cpu_bound(){
    for(int i = 0; i < NUM_LOOPS;++i){
        for(int j = 0; j < NUM_ITERATIONS;++j){
            // MUST disable compiler optimization !
            junk += j+i;
        }
    }
    return junk;
}

void run_io_bound(){
    for(int i = 0; i < NUM_LOOPS/2;++i){
        sleep(100); // units: ticks
    }
}

int run_yielding(){
    for(int i = 0; i < NUM_LOOPS;++i){
        for(int j = 0; j < NUM_ITERATIONS;++j){
            // MUST disable compiler optimization !
            junk += j+i;
        }
        yield();
    }
    return junk;
}


void single(){

    int pid = fork();
    if(pid == 0){
        //run_cpu_bound();
        sleep(120);
        exit();
    }
    printf(0,"parent waiting\n");
    int waited, ran, slept, endtime;
    int pid2 = wait2(&waited, &ran, &slept, &endtime);
    assert(pid == pid2);

    if(pid == -1){
        printf(0,"wait error\n");
    }else {
        printf(0,"child %d terminated.  wait: %d,\t runtime: %d,\t sleep: %d, \t endtime: %d\n",
                pid, waited, ran, slept, endtime);
    }
    printf(0,"[%d] parent done\n", getpid());
    exit();
}

int main(int ac, char** av) {
    static char *names[] = {"cpu  ", "IO   ", "yield"};

    if(ac != 2){
        printf(0,"usage:  sanity n \nn is number of processes to spawn\n");
        exit();
    }
    int n = atoi(av[1]);
    if(n < 1 || n > 10){
        printf(0,"use n in [1..10]\n");
        exit();
    }

    // --------
    //single();    exit();
    //---------
    for(int i =0; i<3*n;++i){
        int pid = fork();
        if(pid < 0){
            printf(0,"fork failed");
            return (3);
        }
        if(pid == 0) {
            // in child
            int new_pid = getpid();
            int d = new_pid%3;
            //printf(0,"running with pid=%d, d = %d\n", new_pid,d);
            switch(d){
                case 0: run_cpu_bound(); break;
                case 1: run_io_bound(); break;
                case 2: run_yielding(); break;
                default:
                    assert(!"bad flow");
            }
            printf(0,"\npid %d finished\n", new_pid);
            EXIT();
        }
    }

    // struct to hold average timing for the processes
    struct {
        double waited, ran, slept; // number of ticks in each state
    } averages[3]; // 3 stress types
    memset(averages,0, sizeof(averages));

    /* the Turnaround is the total time the process took from submission to completion.
     * It does not include being Zombie, so cannot take the value from wait().
     *  maybe simply waited+ran+slept? in this case, why do I need ctime?
     */
    // wait for all of them, no order
    for(int i =0; i< 3*n ;++i){
        int waited, ran, slept, elapsed;
        //printf(0,"waited %p, ran %p, slept %p\n", &waited, &ran, &slept);
        int pid = wait2(&waited, &ran, &slept, &elapsed);
        if(pid == -1){
            printf(0,"wait error\n");
        }else {
            int k = pid%3;
            averages[k].waited += waited;
            averages[k].ran += ran;
            averages[k].slept += slept;
            printf(0,"[%s] process %d terminated.  wait: %d,\t runtime: %d,\t sleep: %d, \t dtime: %d\n",
                    names[k], pid, waited, ran, slept, elapsed);
        }
    }
    printf(0,"---------\n");

    for(int k = 0; k <3;++k) {
        // convert from sum to average
        averages[k].waited /= n;
        averages[k].ran /= n;
        averages[k].slept /= n;
        printf(0,"[%s] sleep: %d\t ready: %d,\t running: %d\n",
              names[k], (int)(averages[k].slept) , (int)(averages[k].waited), (int)(averages[k].ran));
    }
    printf(0,"---------\n");
    EXIT(); // must exit in xv6 shell
    return 0;
}
