#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <linux/time.h>
#include <sys/syscall.h>

#define _DEOPEN_        328
#define _DECLOSE_       329
#define _DEWAIT_        330
#define _DESIG_         331

typedef __pid_t pid_t;
typedef __uid_t uid_t;
typedef unsigned long cputime_t;

void test_case0()
{
        int     num = 10;
        int     event[num];
        int     i;

        printf("---- test general operations ----\n");
        for (i = 0; i < num; i ++) {
                if ((event[i] = syscall(_DEOPEN_)) == -1)
                        printf("Fail to create event at %d\n", i);
                else
                        printf("Succeed to create event %d\n", event[i]);
        }

        for (i = 0; i < num; i ++) {
                if (syscall(_DESIG_,   event[i]) == -1)
                        printf("Fail to signal event %d\n", event[i]);
                if (syscall(_DECLOSE_, event[i]) == -1)
                        printf("Fail to close event %d\n", event[i]);
        }

        for (i = 0; i < num; i ++) {
                if (syscall(_DEWAIT_,  event[i]) == -1)
                        printf("Fail to wait event %d\n", event[i]);
                if (syscall(_DESIG_,   event[i]) == -1)
                        printf("Fail to signal event %d\n", event[i]);
                if (syscall(_DECLOSE_, event[i]) == -1)
                        printf("Fail to close event %d\n", event[i]);
        }
}

int test_case1() {
        int     child_num = 10;
        pid_t   pid[child_num];
        int     eid;
        int     i;

        printf("\nPlease entry andy key to continue ...\n");
        getchar();

        printf("---- test block and waitup by signal ----\n");
        eid = syscall(_DEOPEN_);
        if (eid == -1) {
                printf("test_case1: fail to open event!\n");
                return(-1);
        }
        else
                printf("Parent open a doevent %d\n", eid);

        for (i = 0; i < child_num; i ++) {
                pid[i] = fork();

                if (pid[i] == 0) {              // child
                        printf("child %d try to get doevent %d\n",getpid(),eid);
                        syscall(_DEWAIT_, eid);
                        printf("child %d get doevent %d\n",getpid(),eid);
                        exit(0);
                }
        }

        sleep(5);
        printf("Parent will signal doevent ...\n");
        printf("signal number: %d\n", syscall(_DESIG_, eid));
        sleep(10);
        syscall(_DECLOSE_, eid);

        return(0);
}

int test_case2() {
        int     child_num = 10;
        pid_t   pid[child_num];
        int     eid;
        int     i;

        printf("\nPlease entry andy key to continue ...\n");
        getchar();
        printf("---- test block and waitup by close ----\n");
        eid = syscall(_DEOPEN_);
        if (eid == -1) {
                printf("test_case1: fail to open event!\n");
                return(-1);
        }
        else
                printf("Parent open a doevent %d\n", eid);

        for (i = 0; i < child_num; i ++) {
                pid[i] = fork();

                if (pid[i] == 0) {              // child
                        printf("child %d try to get doevent %d\n",getpid(),eid);
                        syscall(_DEWAIT_, eid);
                        printf("child %d get doevent %d\n",getpid(),eid);
                        exit(0);
                }
        }

        sleep(5);
        printf("Parent will close doevent ...\n");
        printf("signal number: %d\n", syscall(_DECLOSE_, eid));
        sleep(10);

        return(0);
}

int test_case3() {
        int     child_num = 10;
        pid_t   pid[child_num];
        int     eid;
        int     i;

        printf("\nPlease entry andy key to continue ...\n");
        getchar();

        printf("---- test multi-processes open doevent simutaniously ----\n");

        for (i = 0; i < child_num; i ++) {
                pid[i] = fork();

                if (pid[i] == 0) {              // child
                        eid = syscall(_DEOPEN_);
                        if (eid == -1) {
                                printf("test_case1: fail to open event!\n");
                                return(-1);
                        }
                        else
                                printf("child %d open a de %d\n",getpid(),eid);
                        syscall(_DECLOSE_, eid);
                        exit(0);
                }
        }

        sleep(5);

        return(0);
}

int test_case4() {
        int     child_num = 2;
        pid_t   pid[child_num];
        int     eid;
        int     i;

        printf("\nPlease entry andy key to continue ...\n");
        getchar();

        printf("---- test reblock on the same doevent ----\n");
        eid = syscall(_DEOPEN_);
        if (eid == -1) {
                printf("test_case1: fail to open event!\n");
                return(-1);
        }
        else
                printf("Parent open a doevent %d\n", eid);

        for (i = 0; i < child_num; i ++) {
                pid[i] = fork();

                if (pid[i] == 0) {              // child
                        printf("child %d try to get doevent %d\n",getpid(),eid);
                        syscall(_DEWAIT_, eid);
                        printf("child %d get doevent %d\n",getpid(),eid);
                        exit(0);
                }
                else {                          // parent
                        sleep(5);
                        printf("parent signal doevent %d\n", eid);
                        syscall(_DESIG_, eid);
                        sleep(5);
                }
        }

        sleep(5);
        syscall(_DECLOSE_, eid);

        return(0);
}


int main(int argc, char *argv[])
{
        int     i;
        int     eid;
        pid_t   pid;

        test_case0();
        test_case1();
        test_case2();
        test_case3();
        test_case4();

        return(0);

}

