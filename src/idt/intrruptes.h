#pragma once

#include "../panic/panic.h"

#include "../io/keyboard/keyboard.h"
#include "../io/mouse/mouse.h"

struct interrupt_frame;

__attribute__((interrupt)) void PageFault_Handler(interrupt_frame* frame);
__attribute__((interrupt)) void DoubleFault_Handler(interrupt_frame* frame);
__attribute__((interrupt)) void GPFault_Handler(interrupt_frame* frame);

__attribute__((interrupt)) void KB_int_Handler(interrupt_frame* frame);
__attribute__((interrupt)) void Mouse_int_Handler(interrupt_frame* frame);
