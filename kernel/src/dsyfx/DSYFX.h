#pragma once

#include "../Basic_Renderer.h"
#include "../../../gnu-efi/bootloader/bootloader.h" // Can't compile without our bootloader source code
#include "../Panic.h"

void _DSYFX_FAULT_DETECTED() {
    DSYFX_Fault("DSYFX Verification Failed. Key Mismatch!");
    while(true);
}

// Compare bootloader Magic Value with the one provided by the kernel and return either a 0 or 1 to be used by future functions.
int _DSYFX(int _DSYFX_CHALLENGE_INPUT) {

    int (BOOTLOADER)(_BOOTLOADER());

    int _DSYFX_BOOTLOADER_VALUE = BOOTLOADER;
    int _DSYFX_CHALLENGE_VALUE = _DSYFX_CHALLENGE_INPUT;
    
    if (_DSYFX_BOOTLOADER_VALUE != _DSYFX_CHALLENGE_VALUE) {
        return 0;
    } else {
        return 1;
    }
}

// Run the check at boot time and output to the console if the bootloader is valid. If not, the kernel will not boot.
int _DSYFX_BOOT_CHECK(int _DSYFX) {

    int (DSYFX_RETURN_VALUE)(_DSYFX);

    if (DSYFX_RETURN_VALUE == 1) {
        GlobalRenderer->Print("DSYFX Has Arrived");
        return 1;
    } else {
        _DSYFX_FAULT_DETECTED();
        return 0;
    }
}

// Randomly check the bootloader for validity by comparing the Magic Value with value supplied by kernel.
int _DSYFX_RANDOM_CHECK(int _DSYFX) {

    int (DSYFX_RETURN_VALUE)(_DSYFX);

    if (DSYFX_RETURN_VALUE == 1) {
        return 1;
    } else {
        _DSYFX_FAULT_DETECTED();
        return 0;
    }
}

/* int _DSYFX(int _DSYFX_CHALLENGE_INPUT) {

    int (BOOTLOADER)(_BOOTLOADER());

    int _DSYFX_BOOTLOADER_VALUE = BOOTLOADER;
    int _DSYFX_CHALLENGE_INPUT_VALUE = _DSYFX_CHALLENGE_INPUT;

    if (&_DSYFX_CHALLENGE_INPUT_VALUE != &_DSYFX_BOOTLOADER_VALUE) {

        if (_DSYFX_BOOTLOADER_VALUE == NULL) {
            _DSYFX_MISSING_MV_BOOTLOADER = true;
            _DSYFX_Fault_Detected();
            return _DSYFX_MISSING_MV_BOOTLOADER;
        } else {
            if (_DSYFX_CHALLENGE_INPUT_VALUE == NULL) {
                _DSYFX_MISSING_CI_VALUE = true;
                _DSYFX_Fault_Detected();
                return _DSYFX_MISSING_CI_VALUE;
            } else {
                if (_DSYFX_BOOTLOADER_VALUE == _DSYFX_CHALLENGE_INPUT_VALUE) {
                    _DSYFX_VALID_BOOTLOADER = true;
                    _DSYFX_Fault_Detected();
                    GlobalRenderer->Print("DSYFX Has Arrived");
                    return _DSYFX_VALID_BOOTLOADER;
            } else {
                _DSYFX_VALID_BOOTLOADER = false;
                return _DSYFX_VALID_BOOTLOADER;
            }
        }
    }
}
} */

/* void _DSYFX_Fault_Detected() {
    if (_DSYFX_VALID_BOOTLOADER == false) {
        DSYFX_Fault("DSYFX Verification Failed. Bootloader Is Not Valid.");
        while(true);
    } else if (_DSYFX_MISSING_MV_BOOTLOADER == true) {
        DSYFX_Fault("Magic Value Not Found In Memory. This is a fatal error and a memory diagnostic is recommended.");
        while(true);
    } else if (_DSYFX_MISSING_CI_VALUE == true) {
        DSYFX_Fault("Challenge Value Not Found In Memory. This is a fatal error and a memory diagnostic is recommended.");
        while(true);
    }
} */








