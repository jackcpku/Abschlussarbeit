#ifndef _PADDR_H_
#define _PADDR_H_

#include <inttypes.h>
#include <stdint.h>
#include "unistd.h"
#include <stdio.h>
#define PAGEMAP_ENTRY 8
#define GET_BIT(X, Y) (X & ((uint64_t)1 << Y)) >> Y
#define GET_PFN(X) X & 0x7FFFFFFFFFFFFF
#define page_mapping_file "/proc/self/pagemap"

const int __endian_bit = 1;
#define is_bigendian() ((*(char *)&__endian_bit) == 0)

size_t libkdump_virt_to_phys(size_t virtual_address)
{
    static int pagemap = -1;
    if (pagemap == -1)
    {
        pagemap = open("/proc/self/pagemap", "rb");
        if (pagemap < 0)
        {
            return 0;
        }
    }
    uint64_t value;
    int got = pread(pagemap, &value, 8, (virtual_address / 0x1000) * 8);
    if (got != 8)
    {
        return 1;
    }
    uint64_t page_frame_number = value & ((1ULL << 54) - 1);
    if (page_frame_number == 0)
    {
        return 2;
    }
    return page_frame_number * 0x1000 + virtual_address % 0x1000;
}

uintptr_t vtop(uintptr_t virt_addr)
{
    uintptr_t file_offset = 0;
    uintptr_t read_val = 0;
    uintptr_t page_number = 0;
    int i = 0;
    int c = 0;
    int pid = 0;
    int status = 0;
    unsigned char c_buf[PAGEMAP_ENTRY];

    FILE *f = fopen(page_mapping_file, "rb");
    if (!f)
    {
        // if this happens run as root
        printf("Error! Cannot open %s. Please, run as root.\n", page_mapping_file);
        return 0;
    }

    file_offset = virt_addr / getpagesize() * PAGEMAP_ENTRY;

    status = fseek(f, file_offset, SEEK_SET);
    if (status)
    {
        printf("Error! Cannot seek in %s.\n", page_mapping_file);
        perror("Failed to do fseek!");
        fclose(f);
        return 0;
    }

    for (i = 0; i < PAGEMAP_ENTRY; i++)
    {
        c = getc(f);
        if (c == EOF)
        {
            fclose(f);
            return 0;
        }

        if (is_bigendian())
        {
            c_buf[i] = c;
        }
        else
        {
            c_buf[PAGEMAP_ENTRY - i - 1] = c;
        }
    }

    for (i = 0; i < PAGEMAP_ENTRY; i++)
    {
        read_val = (read_val << 8) + c_buf[i];
    }

    /*
    if(GET_BIT(read_val, 63))
    {
       page_number = GET_PFN(read_val);
       printf("%d \n", page_number);
    }
    else
    {
      printf("Page not present\n");
    }
    if(GET_BIT(read_val, 62))
    {
       printf("Page swapped\n");
    }
    */
    fclose(f);

    return read_val;
}

#endif
