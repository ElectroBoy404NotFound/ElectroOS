#include "kernel.h"

extern "C" void _start(BootInfo* bootinfo){
    UtilClasses utils = initlize(bootinfo);
    
    Print* print = utils.print;
    PageFrameAllocator* pageFrameAloc = utils.pageFrameAlloc;

    print->println(hexToString((uint64_t) bootinfo->rsdp));

    while(1) mouse.process();
}