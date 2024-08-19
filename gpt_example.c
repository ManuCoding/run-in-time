#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

// Global variables
int a = 5;
int b = 10;
int result = 0;

int main() {
    // Size of the memory to allocate
    size_t size = 4096;

    // Allocate memory with RWX permissions
    void *mem = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (mem == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // Machine code to add two integers
    unsigned char code[] = {
        0x8B, 0x05, 0x00, 0x00, 0x00, 0x00,   // mov eax, [rip+a]
        0x03, 0x05, 0x00, 0x00, 0x00, 0x00,   // add eax, [rip+b]
        0x89, 0x05, 0x00, 0x00, 0x00, 0x00    // mov [rip+result], eax
    };

    // Correctly calculate the relative offsets
    *(unsigned int *)(code + 2) = (unsigned char *)&a - (unsigned char *)mem - 6;
    *(unsigned int *)(code + 8) = (unsigned char *)&b - (unsigned char *)mem - 12;
    *(unsigned int *)(code + 14) = (unsigned char *)&result - (unsigned char *)mem - 18;

    // Copy the machine code to the executable memory
    memcpy(mem, code, sizeof(code));

    // Execute the machine code
    void (*func)() = mem;
    func();

    // Output the result
    printf("a = %d, b = %d, result = %d\n", a, b, result);

    // Clean up by unmapping the memory
    if (munmap(mem, size) == -1) {
        perror("munmap");
        exit(1);
    }

    return 0;
}

