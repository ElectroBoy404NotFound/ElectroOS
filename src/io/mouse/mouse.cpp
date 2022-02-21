#include "mouse.h"

#define XSign     0B00010000
#define YSign     0B00100000

#define XOverflow 0B01000000
#define YOverflow 0B10000000

#define LButton 0B00000001
#define MButton 0B00000010
#define RButton 0B00000100

Mouse mouse;

void mouseWait() {
    uint64_t timeout = 100000;
    while(timeout--)
        if((inb(0x64) & 0B10) == 0) 
            return;
    return;
}
void mouseWaitInput() {
    uint64_t timeout = 100000;
    while(timeout--)
        if(inb(0x64) & 0B1)
            return;
    return;
}
void mouseWrite(uint8_t value) {
    mouseWait();
    outb(0x64, 0xD4);
    mouseWait();
    outb(0x60, value);
}
uint8_t mouseRead() {
    mouseWaitInput();
    return inb(0x60);
}

void Mouse::init(UtilClasses* utils) {
    outb(0x64, 0xA8);
    mouseWait();
    outb(0x64, 0x20);
    mouseWaitInput();
    uint8_t status = inb(0x60);
    status |= 0b10;
    mouseWait();
    outb(0x64, 0x60);
    mouseWait();
    outb(0x60, status);
    mouseWrite(0xF6);
    mouseRead();
    mouseWrite(0xF4);
    mouseRead();

    Mouse::utils = utils;
}

uint8_t cycle = 0;
uint8_t packet[4];
bool packetReady = false;
void Mouse::onIntrrupt() {
    uint8_t mouseData = inb(0x60);
    switch(cycle) {
        case 0:
            if(packetReady) break;
            if(mouseData & 0B00001000 == 0) break;
            packet[0] = mouseData;
            cycle++;
            break;
        case 1:
            if(packetReady) break;
            packet[1] = mouseData;
            cycle++;
            break;
        case 2:
            if(packetReady) break;
            packet[2] = mouseData;
            cycle = 0;
            packetReady = true;
            break;
    }
    PIC_EndSlave();
}

Point MousePosition;
Point MousePositionl;
void Mouse::process() {
    if(!packetReady) return;
        packetReady = false;
    
    bool xN, yN, xO, yO;
    xN = yN = xO = yO = false;

    if(packet[0] & XSign)
        xN = true;
    if(packet[0] & YSign)
        yN = true;
    if(packet[0] & XOverflow)
        xO = true;
    if(packet[0] & YOverflow)
        yO = true;

    if(!xN) 
        MousePosition.X += packet[1];
    else 
        MousePosition.X -= 256 - packet[1];
    if(xO)
        MousePosition.X += xN ? -255 : 255;

    if(!yN) 
        MousePosition.Y -= packet[2];
    else 
        MousePosition.Y += 256 - packet[2];
    if(yO)
        MousePosition.Y += yN ? 255 : -255;

    if (MousePosition.X < 0) MousePosition.X = 0;
    if (MousePosition.X > utils->print.getFramebuffer()->Width-8) MousePosition.X = utils->print.getFramebuffer()->Width-8;

    if (MousePosition.Y < 0) MousePosition.Y = 0;
    if (MousePosition.Y > utils->print.getFramebuffer()->Height-16) MousePosition.Y = utils->print.getFramebuffer()->Height-16;

    utils->mouseRenderer.clear(MousePositionl);
    utils->mouseRenderer.render(MousePosition);
    MousePositionl = MousePosition;
}