#include "intrruptes.h"

__attribute__((interrupt)) void PageFault_Handler(interrupt_frame* frame) {
    panic.panic("PAGE_FAULT");
}
__attribute__((interrupt)) void DoubleFault_Handler(interrupt_frame* frame) {
    panic.panic("DOUBLE_FAULT");
}
__attribute__((interrupt)) void GPFault_Handler(interrupt_frame* frame) {
    panic.panic("GENERAL_PROTECTION_FAULT");
}

__attribute__((interrupt)) void KB_int_Handler(interrupt_frame* frame) {
    keyboard.onKey();
}
__attribute__((interrupt)) void Mouse_int_Handler(interrupt_frame* frame) {
    mouse.onIntrrupt();
}