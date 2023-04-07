#include <memory.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static jmp_buf jbuf;
static void catch_segv()
{
    longjmp(jbuf, 1);
}

// ---------------------------------------------------------------------------
static void unblock_signal(int signum __attribute__((__unused__)))
{
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, signum);
    sigprocmask(1, &sigs, NULL);
}

// ---------------------------------------------------------------------------
static void segfault_handler(int signum)
{
    (void)signum;
    unblock_signal(SIGSEGV);
    longjmp(jbuf, 1);
}

int main()
{
    int *p = 0xffff000000000000;

    signal(SIGSEGV, segfault_handler);
    for (size_t i = 0; i < 10; i++)
    {
        if (setjmp(jbuf) == 0)
        {
            printf("%d\n", *(p + i));
        }
        else
        {
            printf("Ouch! I crashed!\n");
        }
    }

    return 0;
}