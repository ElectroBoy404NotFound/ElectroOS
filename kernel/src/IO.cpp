
#include "IO.h"

// Outbound. ASM code must be volatile so compiler does not change.
void outb(uint16_t port, uint8_t value){
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Inbound. ASM code must be volatile so compiler does not change.
uint8_t inb(uint16_t port){
    uint8_t returnVal;
    asm volatile ("inb %1, %0"
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

// Wait function. Sends "0" to unused IO port 0x80 to waste one cycle when IO is slow to allow
// devices to catch up.
void io_wait(){
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}