#include "panic.h"

Panic panic;

void Panic::init(UtilClasses* classes) {
    Panic::utils = classes;
}
void Panic::panic(const char* error) {
    utils->print.clearScreen(0x00ff0000);
    utils->print.setColor(0);
    utils->print.setCursor(0, 0);
    utils->print.println("KERNEL PANIC");
    utils->print.println("ElectroOS has crashed due to an error!");
    utils->print.print("Please share the following to the dev: ");
    utils->print.println(error);
    while(1);
}