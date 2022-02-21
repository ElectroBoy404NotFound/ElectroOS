#include "pci.h"

namespace PCI {
    void EnumerateFunction(uint64_t deviceAddress, uint64_t function, UtilClasses utils, PageTableManager ptm){
        uint64_t offset = function << 12;

        uint64_t functionAddress = deviceAddress + offset;
        ptm.MapMemory((void*)functionAddress, (void*)functionAddress, utils);

        PCIDeviceHeader* pciDeviceHeader = (PCIDeviceHeader*)functionAddress;

        if (pciDeviceHeader->DeviceID == 0) return;
        if (pciDeviceHeader->DeviceID == 0xFFFF) return;

        utils.print->print(hexToString(pciDeviceHeader->VendorID));
        utils.print->print(" ");
        utils.print->println(hexToString(pciDeviceHeader->DeviceID));
    }

    void EnumerateDevice(uint64_t busAddress, uint64_t device, UtilClasses utils, PageTableManager ptm){
        uint64_t offset = device << 15;

        uint64_t deviceAddress = busAddress + offset;
        ptm.MapMemory((void*)deviceAddress, (void*)deviceAddress, utils);

        PCIDeviceHeader* pciDeviceHeader = (PCIDeviceHeader*)deviceAddress;

        if (pciDeviceHeader->DeviceID == 0) return;
        if (pciDeviceHeader->DeviceID == 0xFFFF) return;

        for (uint64_t function = 0; function < 8; function++){
            EnumerateFunction(deviceAddress, function, utils, ptm);
        }
    }

    void EnumerateBus(uint64_t baseAddress, uint64_t bus, UtilClasses utils, PageTableManager ptm){
        uint64_t offset = bus << 20;

        uint64_t busAddress = baseAddress + offset;
        ptm.MapMemory((void*)busAddress, (void*)busAddress, utils);

        PCIDeviceHeader* pciDeviceHeader = (PCIDeviceHeader*)busAddress;

        if (pciDeviceHeader->DeviceID == 0) return;
        if (pciDeviceHeader->DeviceID == 0xFFFF) return;

        for (uint64_t device = 0; device < 32; device++){
            EnumerateDevice(busAddress, device, utils, ptm);
        }
    }

    void EnumeratePCI(ACPI::MCFGHeader* mcfg, UtilClasses utils, PageTableManager ptm){
        int entries = ((mcfg->Header.Length) - sizeof(ACPI::MCFGHeader)) / sizeof(ACPI::DeviceConfig);
        for (int t = 0; t < entries; t++){
            ACPI::DeviceConfig* newDeviceConfig = (ACPI::DeviceConfig*)((uint64_t)mcfg + sizeof(ACPI::MCFGHeader) + (sizeof(ACPI::DeviceConfig) * t));
            for (uint64_t bus = newDeviceConfig->StartBus; bus < newDeviceConfig->EndBus; bus++){
                EnumerateBus(newDeviceConfig->BaseAddress, bus, utils, ptm);
            }
        }
    }
}