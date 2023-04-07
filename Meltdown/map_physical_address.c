#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <physical_address>\n", argv[0]);
        return 1;
    }

    unsigned long long physical_address = strtoull(argv[1], NULL, 0);
    size_t map_size = sysconf(_SC_PAGESIZE);

    // Open the /dev/mem device file
    int fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd == -1)
    {
        perror("Error opening /dev/mem");
        return 1;
    }

    // Use mmap to map the physical address to a virtual address
    void *mapped_address = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd, physical_address & ~(map_size - 1));
    if (mapped_address == MAP_FAILED)
    {
        perror("Error mapping memory haha");
        close(fd);
        return 1;
    }

    // Access the mapped memory
    unsigned char value = *((unsigned char *)(mapped_address + (physical_address & (map_size - 1))));
    printf("Value at physical address 0x%llx: 0x%x\n", physical_address, value);

    // Clean up
    munmap(mapped_address, map_size);
    close(fd);

    return 0;
}
