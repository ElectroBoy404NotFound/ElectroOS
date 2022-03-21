
#include "Panic.h"
#include "Basic_Renderer.h"

void Panic(const char* panicMessage){
    
    CLEAR_FAULT_COLOR;
    CLEAR_SCREEN;

    CURSOR_DEFAULT;

    GlobalRenderer->Color = 0;

    GlobalRenderer->Print("Kernel Panic");

    CURSOR_DOUBLE;

    GlobalRenderer->Print(panicMessage);
}

void DSYFX_Fault(const char* faultMessage){
    
    CLEAR_FAULT_COLOR;
    CLEAR_SCREEN;

    CURSOR_DEFAULT;

    GlobalRenderer->Color = 0;

    GlobalRenderer->Print("Security Fault");

    CURSOR_DOUBLE;

    GlobalRenderer->Print(faultMessage);
}
