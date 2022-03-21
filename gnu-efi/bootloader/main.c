#include <efi.h> // These files are contained in other build folders and compile correctly.
#include <efilib.h>
#include <elf.h>
//#include "bootloader.h"

// Magic numbers to determine valid PSF font format
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

// Bootloader information strings. Change as necessary for your system.
#define VERSION "ElectroOS Bootloader Release 1.0.0a Version Generic_12292021-01_i386_amd64 (64-Bit) \n\r"
#define COPYRIGHT_STRING "Copyright 2020 - 2022 ElectroBoy404NotFound. All rights reserved. \n\r"
#define DEVELOPER_INFO "Developed by ElectroBoy404NotFound. \n\r"
#define LICENSE_INFO "Use is subject to license terms. \n\r"
#define VERSION_UNFORMATTED "100"

// Framebuffer typedefs 
typedef unsigned long long size_t;
typedef struct {
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
} Framebuffer;

// PSF font typedefs
typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER* psf1_Header;
	void* glyphBuffer;
} PSF1_FONT;

// Determines whether EFI Graphics Output Protocol (GOP) is present on system. Returns NULL
// if not present, otherwise displays framebuffer information in boot output.
Framebuffer framebuffer;
Framebuffer* InitializeGOP(){
	EFI_GUID gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGUID, NULL, (void**)&gop);
	if(EFI_ERROR(status)){
		Print(L"Unable to locate GOP.\n\r");
		return NULL;
	}
	else
	{
		Print(L"GOP has arrived. Let the party begin...\n\r");
	}

	framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
	framebuffer.BufferSize = gop->Mode->FrameBufferSize;
	framebuffer.Width = gop->Mode->Info->HorizontalResolution;
	framebuffer.Height = gop->Mode->Info->VerticalResolution;
	framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

	return &framebuffer;
	
}

// EFI file handling procedures
EFI_FILE* loadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
	EFI_FILE* LoadedFile;

	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

	if (Directory == NULL){
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	if (s != EFI_SUCCESS){
		return NULL;
	}
	return LoadedFile;
}

// PSF font handling procedures. Verifies font header is valid by comparing magic number
// values to the #define statements at top of file.
PSF1_FONT* loadPSF1Font(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* font = loadFile(Directory, Path, ImageHandle, SystemTable);
	if (font == NULL) return NULL;

	PSF1_HEADER* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&fontHeader);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader);

	if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1){
		return NULL;
	}

	UINTN glyphBufferSize = fontHeader->charsize * 256;
	if (fontHeader->mode == 1) {
		glyphBufferSize = fontHeader->charsize * 512;
	}

	void* glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT* finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->psf1_Header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;
	return finishedFont;

}

int memcmp(const void* aptr, const void* bptr, size_t n){
		const unsigned char* a = aptr, *b = bptr;
		for (size_t i =0; i < n; i++){
			if (a[i] < b[i]) return -1;
			else if (a[i] > b[i]) return 1;
		} 
		return 0;
	}

// Memory management typedef
typedef struct {
	Framebuffer* framebuffer;
	PSF1_FONT* psf1_Font;
	EFI_MEMORY_DESCRIPTOR* mMap;
	UINTN mMapSize;
	UINTN mMapDescSize;
	void* rsdp;
	UINTN dsyfx;
} BootInfo;

// String compare function for PCI device enumeration
UINTN strcmp(CHAR8* a, CHAR8* b, UINTN length){
	for (UINTN i = 0; i < length; i++){
		if (*a != *b) return 0;
	}
	return 1;
}


// Main EFI function. Searches for kernel file named "kernel.elf" and loads into memory.
// Several checks are present to determine if kernel file is a valid kernel image and
// of the correct architecture for the machine running code. 
EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	
	InitializeLib(ImageHandle, SystemTable);
	//Print(L(VERSION));
	//Print(L(COPYRIGHT_STRING));
	//Print(L(DEVELOPER_INFO));
	//Print(L(LICENSE_INFO));
	Print(L"ElecroOS Bootloader Release 1.0.0a Version Generic_12292021-01_i386_amd64 \n\r");
	Print(L"Copyright 2020 - 2022 ElectroBoy404NotFound. All rights reserved.\n\r");
	Print(L"Developed by ElectroBoy404NotFound.\n\r");
	Print(L"Use is subject to license terms.\n\r");
	Print(L" \n\r");

	EFI_FILE* Kernel = loadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
	if ( Kernel == NULL){
		Print(L"Error locating kernel within filesystem.\n\r");
		Print(L"FATAL ERROR: 0xB9BAD3\n\r"); // Rachel's birthday
	}
	else {
		Print(L"Kernel located.\n\r");
	}

	// Check to determine if kernel header is of the correct format.
	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	)
	{
		Print(L"Kernel format is incompatible with this hardware.\n\r");
	}
	else 
	{
		Print(L"Kernel header successfully verified.\n\r");
	}

	Elf64_Phdr* phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for (
		Elf64_Phdr* phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type){
			case PT_LOAD:
			{
				int pages = (phdr->p_memsz + 0x1000 -1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void*)segment);
				break;
			}
		}
	}

	Print(L"Kernel loaded into memory.\n\r");
	Print(L"Waiting for GOP...\n\r");

	// Load the PSF font into memory from root of filesystem. PSF must be type 1 font.
	PSF1_FONT* newFont = loadPSF1Font(NULL, L"zap-ext-light16.psf", ImageHandle, SystemTable);
	if (newFont == NULL){
		Print(L"System PSF invalid..\n\r");
	}
	else
	{
		Print(L"System PSF found. Char size = %d\n\r", newFont->psf1_Header->charsize);
	}

	// Initialize the framebuffer and pass to kernel. Display output of data being passed
	// to kernel for diagnostic reasons.
	Framebuffer* newBuffer = InitializeGOP();

	Print(L"Base: 0x%x\n\rSize: 0x%x\n\rWidth: %d\n\rHeight: %d\n\rPixelsPerScanline: %d\n\r", 
	newBuffer->BaseAddress, 
	newBuffer->BufferSize, 
	newBuffer->Width, 
	newBuffer->Height, 
	newBuffer->PixelsPerScanLine);

	EFI_MEMORY_DESCRIPTOR* Map = NULL;
	UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	{
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void**)&Map);
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
	}

	// Find RSDP for PCI devices
	EFI_CONFIGURATION_TABLE* configTable = SystemTable->ConfigurationTable;
	void* rsdp = NULL; 
	EFI_GUID Acpi2TableGuid = ACPI_20_TABLE_GUID;

	for (UINTN index = 0; index < SystemTable->NumberOfTableEntries; index++){
		if (CompareGuid(&configTable[index].VendorGuid, &Acpi2TableGuid)){
			if (strcmp((CHAR8*)"RSD PTR ", (CHAR8*)configTable->VendorTable, 8)){
				rsdp = (void*)configTable->VendorTable;
				//break;
			}
		}
		configTable++;
	}

	// Return DSYFX Magic Value to kernel
	Print(L"Returning Magic Value to kernel...\n\r");
	//_BOOTLOADER();
	Print(L"Returned Magic Value.\n\r");

	// DSYFX Magic Number
	int _BOOTLOADER() {
    	int _BOOTLOADER_RETURN_VALUE;

    	// Can be any arbitrary value set by the developer so long as that number is not easily guessed.
    	_BOOTLOADER_RETURN_VALUE = 0xB9BAD3;

    	return _BOOTLOADER_RETURN_VALUE;
	}

	// Begin the process of booting the kernel
	void (*KernelStart)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*) )header.e_entry);

	BootInfo bootInfo;
	bootInfo.framebuffer = newBuffer;
	bootInfo.psf1_Font = newFont;
	bootInfo.mMap = Map;
	bootInfo.mMapSize = MapSize;
	bootInfo.mMapDescSize = DescriptorSize;
	bootInfo.rsdp = rsdp;
	bootInfo.dsyfx = _BOOTLOADER(); // Return DSYFX Magic Value to kernel using BootInfo struct. This passes(?) the magic value to the kernel for use by DSYFX functions.

	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

	KernelStart(&bootInfo);
	

	return EFI_SUCCESS; // Exit the UEFI application
}
