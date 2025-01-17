#pragma once

#include "stdint.h"
#include "stddef.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void ioWait();

void RemapPIC();
void PIC_EndMaster();
void PIC_EndSlave();