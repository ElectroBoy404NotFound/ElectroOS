
#include "GDT.h"

// Not strictly necessary as the __attribute__ is included in our header file, but better
// safe than sorry. 
__attribute__((aligned(0x1000)))

// Defining the Global Descriptor Table structure...
GDT DefaultGDT = {
    {0, 0, 0, 0x00, 0x00, 0}, // NULL segment
    {0, 0, 0, 0x9a, 0xa0, 0}, // KernelCode segment
    {0, 0, 0, 0x92, 0xa0, 0}, // KernelData segment
    {0, 0, 0, 0x00, 0x00, 0}, // User NULL segment
    {0, 0, 0, 0x9a, 0xa0, 0}, // UserCode segment
    {0, 0, 0, 0x92, 0xa0, 0}, // UserData segment
};