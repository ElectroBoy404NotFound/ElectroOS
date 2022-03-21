
#pragma once

#include <stdint.h>
#include "Basic_Renderer.h"
#include "C_String.h"
#include "EFI_Memory.h"
#include "System_Memory.h"
#include "Memory_Bitmap.h"
#include "paging/PageFrameAllocator.h"
#include "paging/PageMapIndexer.h"
#include "paging/Paging.h"
#include "paging/PageTableManager.h"
#include "userinput/Mouse.h"
#include "ACPI.h"
#include "PCI.h"

// Kernel information strings. Change as necessary for your system.
#define VERSION "ElectroOS Kernel Release 1.0.0a Version Generic_12292021-01_i386_amd64 (64-bit)"
#define COPYRIGHT_STRING "Copyright 2020 - 2022 ElectroBoy404NotFound. All rights reserved."
#define DEVELOPER_INFO "Developed by ElectroBoy404NotFound."
#define LICENSE_INFO "Use is subject to license terms."
#define VERSION_UNFORMATTED "100"

// Used by DSYFX
#define RANDOM_KEY int rand(void)

// Memory management typedef
struct BootInfo {
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_Font;
	EFI_MEMORY_DESCRIPTOR* mMap;
	uint64_t mMapSize;
	uint64_t mMapDescSize;
	ACPI::RSDP2* rsdp;
	uint64_t dsyfx; // DSYFX
};

// Variables used to determine start and end of kernel memory space
extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

struct KernelInfo {
    PageTableManager* pageTableManager;
};


KernelInfo InitializeKernel(BootInfo* BootInfo);