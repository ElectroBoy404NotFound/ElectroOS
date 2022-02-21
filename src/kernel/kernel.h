#pragma once

#include "stddef.h"

#include "../util/utils.h"

#include "../io/io.h"
#include "../io/acpi.h"
#include "../io/pci.h"

#include "../io/keyboard/keyboard.h"
#include "../io/mouse/mouse.h"

#include "../graphics/renderer.h"

#include "../memory/paging/PageFrameAllocator.h"
#include "../memory/paging/PageMapIndex.h"
#include "../memory/paging/Paging.h"
#include "../memory/paging/PageTableManager.h"

#include "../gdt/gdt.h"

#include "../panic/panic.h"
#include "../idt/idt.h"
#include "../idt/intrruptes.h"

struct BootInfo {
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_Font;
	EFI_MEMORY_DESCRIPTOR* mMap;
	uint64_t mMapSize;
	uint64_t mMapDescSize;
	ACPI::RSDP2* rsdp;
};

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

IDTR idtr;
UtilClasses initlize(BootInfo* bootinfo) {
    GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&gdtDescriptor);

    Print print(bootinfo->framebuffer, bootinfo->psf1_Font);
    print.setCursor(0, 0);
    MouseRenderer mr(bootinfo->framebuffer, bootinfo->psf1_Font);

    uint64_t mMapEntries = bootinfo->mMapSize / bootinfo->mMapDescSize;
    PageFrameAllocator pageFrameAlloca = PageFrameAllocator();
    pageFrameAlloca.ReadEFIMemoryMap(bootinfo->mMap, bootinfo->mMapSize, bootinfo->mMapDescSize);

    uint64_t kernelSize  = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernelPages = (uint64_t)kernelSize / 4096 + 1;

    pageFrameAlloca.LockPages(&_KernelStart, kernelPages);

    PageTable* PML4 = (PageTable*)pageFrameAlloca.RequestPage();
    memoryset(PML4, 0, 0x1000);

    PageTableManager pageTableManager = PageTableManager(PML4);
    
    UtilClasses rval = {&print, &mr, &pageFrameAlloca};

    for (uint64_t t = 0; t < getMemorySize(bootinfo->mMap, mMapEntries, bootinfo->mMapDescSize); t+= 0x1000)
        pageTableManager.MapMemory((void*)t, (void*)t, rval);

    uint64_t fbBase = (uint64_t)bootinfo->framebuffer->BaseAddress;
    uint64_t fbSize = (uint64_t)bootinfo->framebuffer->BufferSize + 0x1000;
    pageFrameAlloca.LockPages((void*)fbBase, fbSize/ 0x1000 + 1);
    for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096)
        pageTableManager.MapMemory((void*)t, (void*)t, rval);

    asm ("mov %0, %%cr3" : : "r" (PML4));

    idtr.Limit = 0x0FFF;
    idtr.Offset = (uint64_t)pageFrameAlloca.RequestPage();

    IDTDescEntry* int_PageFault = (IDTDescEntry*)(idtr.Offset + 0xE * sizeof(IDTDescEntry));
    int_PageFault->SetOffset((uint64_t)PageFault_Handler);
    int_PageFault->type_attr = IDT_TA_InterruptGate;
    int_PageFault->selector = 0x08;

    IDTDescEntry* int_DoubleFault = (IDTDescEntry*)(idtr.Offset + 0x8 * sizeof(IDTDescEntry));
    int_DoubleFault->SetOffset((uint64_t)DoubleFault_Handler);
    int_DoubleFault->type_attr = IDT_TA_InterruptGate;
    int_DoubleFault->selector = 0x08;

    IDTDescEntry* int_GPFault = (IDTDescEntry*)(idtr.Offset + 0xD * sizeof(IDTDescEntry));
    int_GPFault->SetOffset((uint64_t)GPFault_Handler);
    int_GPFault->type_attr = IDT_TA_InterruptGate;
    int_GPFault->selector = 0x08;

    IDTDescEntry* int_KB = (IDTDescEntry*)(idtr.Offset + 0x21 * sizeof(IDTDescEntry));
    int_KB->SetOffset((uint64_t)KB_int_Handler);
    int_KB->type_attr = IDT_TA_InterruptGate;
    int_KB->selector = 0x08;

    IDTDescEntry* int_Mouse = (IDTDescEntry*)(idtr.Offset + 0x2C * sizeof(IDTDescEntry));
    int_Mouse->SetOffset((uint64_t)Mouse_int_Handler);
    int_Mouse->type_attr = IDT_TA_InterruptGate;
    int_Mouse->selector = 0x08;

    asm ("lidt %0" : : "m" (idtr));

    RemapPIC();

    mouse.init(&rval);
    keyboard.init(&rval);
    panic.init(&rval);

    ACPI::SDTHeader* xsdt = (ACPI::SDTHeader*)(bootinfo->rsdp->XSDTAddress);
    
    ACPI::MCFGHeader* mcfg = (ACPI::MCFGHeader*)ACPI::FindTable(xsdt, (char*)"MCFG");

    for (int t = 0; t < 4; t++){
        print.putChar(mcfg->Header.Signature[t]);
    }
    print.println("");

    PCI::EnumeratePCI(mcfg, rval, pageTableManager);

    outb(PIC1_DATA, 0b11111001);
    outb(PIC2_DATA, 0b11101111);

    asm ("sti");

    return rval;
}