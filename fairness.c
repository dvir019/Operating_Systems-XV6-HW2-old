#include "types.h"
#include "user.h"

extern int set_priority(int);

static int iteratrions = 10*1000*1000;
static double x=0;

/* for several processes:
2 @ prio 1
then 2 @ prio 0
then 1 at prio 3
and for desert, prio 1 again

The expected pattern is
* high prio processes finish before lower gets cpu.
* round robin at prio 1,2,3
* fifo at prio 0 (i.e. process is run till completion)
*/
static int prio_keys[] = {1,1,0,0,2,3,1};

void check_proc_order(){
    printf(1, "parent run at pid %d\n", getpid());

    // I run at default prio 2
    int num_children = sizeof(prio_keys)/sizeof(int);
    for(int k = 0; k < num_children;k++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "fork failed");
            return;
        }
        if (pid == 0) {
            // in child
            //int new_pid = getpid();
           
            int prio = prio_keys[k]; // index k inherited from parent
		    //printf(1,"Child(%d) received prio: %d\n", getpid(), prio);
            set_priority(prio);
            // do something that takes cpu time
            for(k=0;k< iteratrions;k++){
                    x += k; // x is global so the compiler cannot remove this loop
            }
            //printf(1,"\nChild(%d) DONE\n", getpid());
            exit();
        } else{
            // in parent
        }
    }
    for(int i = 0; i < num_children; i++)
        wait();
    //printf(0,"\nPARENT finished\n");
}
int proces_fairness() {
    int num_children = 6;
    check_proc_order();
    return 0; // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    for(int k = 0; k < num_children;k++) {
        int pid = fork();
        if (pid < 0) {
            printf(0, "fork failed");
            return (3);
        }
        if (pid == 0) {
            // in child
            int new_pid = getpid();
            printf(0,"Setting pid %d to prio %d\n", new_pid, new_pid%3);
            set_priority(new_pid % 3);
            for (int i = 0; i < 200; i++) {
                printf(0,"%d,", new_pid);
                for(k=0;k< iteratrions;k++){
                    x += k;
                }
            }
            printf(0,"child %d finished\n");
            exit();
        }
    }

    for(int i = 0; i < num_children; i++)
        wait();
    printf(0,"PARENT exiting\n");
}
#if 0
int main(){
    proces_fairness();
}
#endif