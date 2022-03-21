
#include "Kernel_Util.h"
#include "gdt/GDT.h"
#include "interrupts/IDT.h"
#include "interrupts/Interrupts.h"
#include "IO.h"
#include "PCI.h"
#include "memory/Heap.h"
#include "dsyfx/DSYFX.h"

KernelInfo kernelInfo;
PageTableManager pageTableManager = NULL; 

// We are declaring a basic renderer for all kernel output.
Basic_Renderer r = Basic_Renderer(NULL, NULL);

void PrepareMemory(BootInfo* bootInfo)
{
	// Determine memory map information
	uint64_t mMapEntries = bootInfo->mMapSize / bootInfo->mMapDescSize;

	GlobalAllocator = PageFrameAllocator();
	GlobalAllocator.ReadEFIMemoryMap(bootInfo->mMap, bootInfo->mMapSize, bootInfo->mMapDescSize);
	
	// Determine kernel memory size and lock away to prevent conflict
	uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
	uint64_t kernelPages = (uint64_t)kernelSize / 4096 + 1;

	GlobalAllocator.LockPages(&_KernelStart, kernelPages);

	// Enabling virtual memory paging
	PageTable* PML4 = (PageTable*)GlobalAllocator.RequestPage();
	memset(PML4, 0, 0x1000);

	g_PageTableManager = PageTableManager(PML4);

	for (uint64_t t = 0; t < GetMemorySize(bootInfo->mMap, mMapEntries, bootInfo->mMapDescSize); t+= 0x1000){

		g_PageTableManager.MapMemory((void*)t, (void*)t);
		
	}

	uint64_t fbBase = (uint64_t)bootInfo->framebuffer->BaseAddress;
	uint64_t fbSize = (uint64_t)bootInfo->framebuffer->BufferSize + 0x1000;
	GlobalAllocator.LockPages((void*)fbBase, fbSize / 0x1000 + 1);

	for (uint64_t t = fbBase; t < fbBase + fbSize; t += 4096){
		g_PageTableManager.MapMemory((void*)t, (void*)t);
	}

	asm ("mov %0, %%cr3" : : "r" (PML4));

    kernelInfo.pageTableManager = &g_PageTableManager;
}

void PrepareACPI(BootInfo* bootInfo){
    ACPI::SDTHeader* xsdt = (ACPI::SDTHeader*)(bootInfo->rsdp->XSDTAddress);
    
    ACPI::MCFGHeader* mcfg = (ACPI::MCFGHeader*)ACPI::FindTable(xsdt, (char*)"MCFG");

    PCI::EnumeratePCI(mcfg);
}


void KernelLogo(BootInfo* bootInfo)
{

	// We are using GlobalRenderer to output from the kernel.

	// Kernel logo information can be changed to reflect updated version information.
	// Change these definitions in Kernel_Util.h header file.
	// In future, these should be stored as variables passed to the compiler and auto-populated
	// but for now, this works.
	GlobalRenderer->Print(VERSION);
	CURSOR_SINGLE;
	GlobalRenderer->Print(COPYRIGHT_STRING);
	CURSOR_SINGLE;
	GlobalRenderer->Print(DEVELOPER_INFO);
	CURSOR_SINGLE;
	GlobalRenderer->Print(LICENSE_INFO);
	CURSOR_DOUBLE;

	// Display system memory information. Very DOS-like but I think it looks cool. Does not
	// include virtual memory information.
	GlobalRenderer->Print("Free Physical RAM: ");
	GlobalRenderer->Print(to_string(GlobalAllocator.GetFreeRAM() / 1024));
	GlobalRenderer->Print(" KB");

	CURSOR_SINGLE;
	GlobalRenderer->Print("Used Physical RAM: ");
	GlobalRenderer->Print(to_string(GlobalAllocator.GetUsedRAM() / 1024));
	GlobalRenderer->Print(" KB");

	CURSOR_SINGLE;
	GlobalRenderer->Print("Reserved Physical RAM: ");
	GlobalRenderer->Print(to_string(GlobalAllocator.GetReservedRAM() / 1024));
	GlobalRenderer->Print(" KB");
	CURSOR_DOUBLE;

	// System status messages... no real use but looks cool. Probably a memory drain.
	GlobalRenderer->Print("MEMSET Function Passed");
	CURSOR_SINGLE;
	GlobalRenderer->Print("Memory Sized");
	CURSOR_SINGLE;
	GlobalRenderer->Print("PTM Variable Set");
	CURSOR_SINGLE;

	// DSYFX Implementation
	int (VERIFIER)(bootInfo->dsyfx);
	_DSYFX(VERIFIER);

	int (VALID_INVALID)(_DSYFX(VERIFIER));
	_DSYFX_BOOT_CHECK(VALID_INVALID);

	CURSOR_SINGLE;
	GlobalRenderer->Print("Virtual Memory Initialized");
	CURSOR_SINGLE;
	GlobalRenderer->Print("PCI Devices Probed");
	CURSOR_DOUBLE;

	PrepareACPI(bootInfo);
	
    return;
}

IDTR idtr;
void SetIDTGate(void* handler, uint8_t entryOffset, uint8_t type_attr, uint8_t selector){

    IDTDescEntry* interrupt = (IDTDescEntry*)(idtr.Offset + entryOffset * sizeof(IDTDescEntry));
    interrupt->SetOffset((uint64_t)handler);
    interrupt->type_attr = type_attr;
    interrupt->selector = selector;
}

void PrepareInterrupts(){
    idtr.Limit = 0x0FFF;
    idtr.Offset = (uint64_t)GlobalAllocator.RequestPage();

	// Handle faults with an interrupt.
	SetIDTGate((void*)PageFault_Handler, 0xE, IDT_TA_InterruptGate, 0x08);
    SetIDTGate((void*)DoubleFault_Handler, 0x8, IDT_TA_InterruptGate, 0x08);
    SetIDTGate((void*)GPFault_Handler, 0xD, IDT_TA_InterruptGate, 0x08);
    SetIDTGate((void*)KeyboardInt_Handler, 0x21, IDT_TA_InterruptGate, 0x08);
    SetIDTGate((void*)MouseInt_Handler, 0x2C, IDT_TA_InterruptGate, 0x08);

    asm ("lidt %0" : : "m" (idtr));

	RemapPIC();

}

KernelInfo InitializeKernel(BootInfo* bootInfo)
{
	// Define a GlobalRenderer function that persists across the system and does not exit
	// after kernel is initialized. USAGE: "GlobalRenderer->xxxx"
	r = Basic_Renderer(bootInfo->framebuffer, bootInfo->psf1_Font);
    GlobalRenderer = &r;

    // Load the Global Descriptor Table for the x64 instruction set.
	GDTDescriptor gdtDescriptor;
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&gdtDescriptor);
	
	PrepareMemory(bootInfo);

    // Set all pixels to black as soon as kernel has initialized. Removes UEFI messages.
    // Has added effect of removing odd color bars within some VMs during boot. Uses memset()
	// function to accomplish this (defined in Basic_Renderer.h)
	CLEAR_SCREEN_MEMSET;

	InitializeHeap((void*)0x0000100000000000, 0x10);

	PrepareInterrupts();

	InitPS2Mouse();

    outb(PIC1_DATA, 0b11111001);
    outb(PIC2_DATA, 0b11101111);

	// Enables maskable interrupts.
    asm ("sti");

	// Display kernel logo information after kernel has initialized and virtual memory
	// has been setup. Note that we are probing the PCI devices during the KernelLogo
	// function to ensure that enumeration of devices remains consistent with existing
	// kernel output. Technically we should probably find a way to move it to this function
	// but it works for now.
	KernelLogo(bootInfo);

    return kernelInfo;
}
