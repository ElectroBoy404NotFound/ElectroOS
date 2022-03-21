
#include "Kernel_Util.h"
#include "memory/Heap.h"


// We use extern "C" to tell the file that it is being called from an external C program file
extern "C" void _start(BootInfo* bootInfo){

	KernelInfo kernelInfo = InitializeKernel(bootInfo);
	PageTableManager* pageTableManager = kernelInfo.pageTableManager;

	// We are using GlobalRenderer to output all text from the kernel.

	// Console output must be directed to begin after the kernelLogo function output.
	// We use GlobalRenderer->CursorPosition.Y + 32 to provide a space between the
	// previous output and new output.
	CURSOR_DOUBLE;

	GlobalRenderer->Print("Kernel Initialized Successfully. Awaiting Instructions.");
	CURSOR_DOUBLE;
	GlobalRenderer->Print("ElectroOS #: ");
	
	// We can't return from this function or kernel panic ensues. Should be obvious, but
	// given the different levels of developer skillsets, we include the comment for clarity.
    while(true){
		// Send a CPU instruction to halt the processor when the kernel is not actively
		// working. Increases power efficiency of the processor.
		asm ("hlt");
	}
}