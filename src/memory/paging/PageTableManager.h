#pragma once

#include <stdint.h>

#include "Paging.h"
#include "PageMapIndex.h"
#include "PageFrameAllocator.h"

#include "../memory.h"

#include "../../util/utils.h"

class PageTableManager {
    public:
        PageTableManager(PageTable* PML4Address);
        PageTable* PML4;
        void MapMemory(void* virtualMemory, void* physicalMemory, UtilClasses utils);
};