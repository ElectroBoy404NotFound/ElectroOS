#pragma once

#include <stdint.h>

#include "acpi.h"

#include "../util/utils.h"
#include "../memory/paging/PageTableManager.h"

namespace PCI{
    struct PCIDeviceHeader{
        uint16_t VendorID;
        uint16_t DeviceID;
        uint16_t Command;
        uint16_t Status;
        uint8_t RevisionID;
        uint8_t ProgIF;
        uint8_t Subclass;
        uint8_t Class;
        uint8_t CacheLineSize;
        uint8_t LatencyTimer;
        uint8_t HeaderType;
        uint8_t BIST;
    };

    void EnumeratePCI(ACPI::MCFGHeader* mcfg, UtilClasses utils, PageTableManager ptm);
}
