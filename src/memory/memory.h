#pragma once 

#include <stdint.h>

struct EFI_MEMORY_DESCRIPTOR {
    uint32_t type;
    void* physAddr;
    void* virtAddr; 
    uint64_t numPages;
    uint64_t attribs;
};
void memoryset(void*, uint8_t, uint64_t);
uint64_t getMemorySize(EFI_MEMORY_DESCRIPTOR*, uint64_t, uint64_t);