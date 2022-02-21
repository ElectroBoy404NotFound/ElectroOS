#include "io.h"

void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" :: "a"(value), "Nd"(port));
}
uint8_t inb(uint16_t port) {
    uint8_t returnval;
    asm volatile ("inb %1, %0" : "=a"(returnval) : "Nd"(port));
    return returnval;
}
void ioWait() {
    asm volatile ("outb %%al, $0x80" :: "a"(0));
}

void PIC_EndMaster(){
    outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_EndSlave(){
    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void RemapPIC(){
    uint8_t a1, a2; 

    a1 = inb(PIC1_DATA);
    ioWait();
    a2 = inb(PIC2_DATA);
    ioWait();

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    ioWait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    ioWait();

    outb(PIC1_DATA, 0x20);
    ioWait();
    outb(PIC2_DATA, 0x28);
    ioWait();

    outb(PIC1_DATA, 4);
    ioWait();
    outb(PIC2_DATA, 2);
    ioWait();

    outb(PIC1_DATA, ICW4_8086);
    ioWait();
    outb(PIC2_DATA, ICW4_8086);
    ioWait();

    outb(PIC1_DATA, a1);
    ioWait();
    outb(PIC2_DATA, a2);
}