
#include "Interrupts.h"
#include "../Panic.h"
#include "../IO.h"
#include "../userinput/Keyboard.h"
#include "../userinput/Mouse.h"

__attribute__((interrupt)) void PageFault_Handler(interrupt_frame* frame){
    Panic("Page Fault Detected");
    while(true);
}

__attribute__((interrupt)) void DoubleFault_Handler(interrupt_frame* frame){
    Panic("Double Fault Detected");
    while(true);
}

__attribute__((interrupt)) void GPFault_Handler(interrupt_frame* frame){
    Panic("General Protection Fault Detected");
    while(true);
}

__attribute__((interrupt)) void KeyboardInt_Handler(interrupt_frame* frame){
    uint8_t scancode = inb(0x60);
    HandleKeyboard(scancode);
    PIC_EndMaster();
}

__attribute__((interrupt)) void MouseInt_Handler(interrupt_frame* frame){

    uint8_t mouseData = inb(0x60);

    HandlePS2Mouse(mouseData);

    PIC_EndSlave();
}

// End the interrupts for PIC chip
void PIC_EndMaster(){
    outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_EndSlave(){
    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void RemapPIC(){
    uint8_t a1, a2; 

    // Save current bitmasks
    a1 = inb(PIC1_DATA);
    io_wait();
    a2 = inb(PIC2_DATA);
    io_wait();

    // Initialize Master PIC
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    // Initialize Slave PIC
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // Master PIC offset
    outb(PIC1_DATA, 0x20);
    io_wait();
    // Slave PIC offset
    outb(PIC2_DATA, 0x28);
    io_wait();

    // Tell PICs how they correspond/know of existence
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    // Tell PICs to operate in 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore bitmasks
    outb(PIC1_DATA, a1);
    io_wait();
    outb(PIC2_DATA, a2);

}