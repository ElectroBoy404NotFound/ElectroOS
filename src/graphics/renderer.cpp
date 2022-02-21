#include "renderer.h"

Print::Print(Framebuffer* framebuffer, PSF1_FONT* psf1_font) {
    Print::colour = 0xffffffff;
    Print::CursorPosition = {0, 0};
    Print::framebuffer = framebuffer;
    Print::psf1_font = psf1_font;
}

void Print::setColor(unsigned int colour) {
    Print::colour = colour;
}
Point Print::getCursorPosition() {
    return Print::CursorPosition;
}
void Print::setCursor(Point point) {
    Print::CursorPosition = point;
}
void Print::setCursor(unsigned int x, unsigned int y) {
    Print::CursorPosition = {x, y};
}

void Print::putChar(char chr, unsigned int xOff, unsigned int yOff) {
    unsigned int* pixPtr = (unsigned int*)framebuffer->BaseAddress;
    char* fontPtr = (char*)psf1_font->glyphBuffer + (chr * psf1_font->psf1_Header->charsize);
    for (unsigned long y = yOff; y < yOff + 16; y++){
        for (unsigned long x = xOff; x < xOff+8; x++){
            if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0){
                    *(unsigned int*)(pixPtr + x + (y * framebuffer->PixelsPerScanLine)) = colour;
                }

        }
        fontPtr++;
    }
}

void Print::putChar(char chr) {
    putChar(chr, CursorPosition.X, CursorPosition.Y);
    CursorPosition.X += 8;
    if (CursorPosition.X + 8 > framebuffer->Width){
        CursorPosition.X = 0; 
        CursorPosition.Y += 16;
    }
}
void Print::print(const char* str) {
    char* chr = (char*)str;
    while(*chr != 0){
        if(*chr == '\n') { CursorPosition.X = 0; CursorPosition.Y += 16; }
        else { putChar(*chr, CursorPosition.X, CursorPosition.Y); CursorPosition.X+=8; }
        if(CursorPosition.X + 8 > framebuffer->Width)
        {
            CursorPosition.X = 0;
            CursorPosition.Y += 16;
        }
        chr++;
    }
}
void Print::clearScreen(uint32_t colour) {
    uint64_t fbBase = (uint64_t)framebuffer->BaseAddress;
    uint64_t bytesPerScanline = framebuffer->PixelsPerScanLine * 4;
    uint64_t fbHeight = framebuffer->Height;
    uint64_t fbSize = framebuffer->BufferSize;

    for (int verticalScanline = 0; verticalScanline < fbHeight; verticalScanline ++){
        uint64_t pixPtrBase = fbBase + (bytesPerScanline * verticalScanline);
        for (uint32_t* pixPtr = (uint32_t*)pixPtrBase; pixPtr < (uint32_t*)(pixPtrBase + bytesPerScanline); pixPtr ++){
            *pixPtr = colour;
        }
    }
    CursorPosition = {0,0};
}

void Print::clearChar(uint32_t colour){
    if(!((CursorPosition.X <= 0 && CursorPosition.Y <= 0))) {
        if (CursorPosition.X <= 0){
            CursorPosition.X = framebuffer->Width;
            CursorPosition.Y -= 16;
            if (CursorPosition.Y <= 0) CursorPosition.Y = 0;
        }

        unsigned int xOff = CursorPosition.X;
        unsigned int yOff = CursorPosition.Y;

        unsigned int* pixPtr = (unsigned int*)framebuffer->BaseAddress;
        for (unsigned long y = yOff; y < yOff + 16; y++){
            for (unsigned long x = xOff - 8; x < xOff; x++){
                        *(unsigned int*)(pixPtr + x + (y * framebuffer->PixelsPerScanLine)) = colour;
            }
        }

        CursorPosition.X -= 8;
        if(CursorPosition.X < 0 && CursorPosition.Y < 0) CursorPosition = {0, 0};
        if (CursorPosition.X <= 0){
            CursorPosition.X = framebuffer->Width;
            CursorPosition.Y -= 16;
            if (CursorPosition.Y <= 0) CursorPosition.Y = 0;
        }
    }
}

MouseRenderer::MouseRenderer(Framebuffer* framebuffer, PSF1_FONT* psf1_font) {
    MouseRenderer::colour = 0xffffffff;
    MouseRenderer::framebuffer = framebuffer;
    MouseRenderer::psf1_font = psf1_font;
}
void MouseRenderer::setColor(unsigned int colour) {
    MouseRenderer::colour = colour;
}
void MouseRenderer::render(Point position) {
    uint8_t mousePointer[] = {
        0b11111111, 0b11100000, 
        0b11111111, 0b10000000, 
        0b11111110, 0b00000000, 
        0b11111100, 0b00000000, 
        0b11111000, 0b00000000, 
        0b11110000, 0b00000000, 
        0b11100000, 0b00000000, 
        0b11000000, 0b00000000, 
        0b11000000, 0b00000000, 
        0b10000000, 0b00000000, 
        0b10000000, 0b00000000, 
        0b00000000, 0b00000000, 
        0b00000000, 0b00000000, 
        0b00000000, 0b00000000, 
        0b00000000, 0b00000000, 
        0b00000000, 0b00000000, 
    };

    int xMax = 16;
    int yMax = 16;
    int differenceX = framebuffer->Width - position.X;
    int differenceY = framebuffer->Height - position.Y;

    if (differenceX < 16) xMax = differenceX;
    if (differenceY < 16) yMax = differenceY;

    for (int y = 0; y < yMax; y++){
        for (int x = 0; x < xMax; x++){
            int bit = y * 16 + x;
            int byte = bit / 8;
            if ((mousePointer[byte] & (0b10000000 >> (x % 8)))) {
                mbuff[x + y * 16] = GetPix(position.X + x, position.Y + y);
                *(uint32_t*)((uint64_t)framebuffer->BaseAddress + ((position.X + x)*4) + ((position.Y + y) * framebuffer->PixelsPerScanLine * 4)) = colour;
                mbuffa[x + y * 16] = GetPix(position.X + x, position.Y + y);
            }
        }
    }
}
void MouseRenderer::PutPix(uint32_t x, uint32_t y, uint32_t colour){
    *(uint32_t*)((uint64_t)framebuffer->BaseAddress + (x*4) + (y * framebuffer->PixelsPerScanLine * 4)) = colour;
}
uint32_t MouseRenderer::GetPix(uint32_t x, uint32_t y){
    return *(uint32_t*)((uint64_t)framebuffer->BaseAddress + (x*4) + (y * framebuffer->PixelsPerScanLine * 4));
}
void MouseRenderer::clear(Point c) {
    if (!MouseDrawn) { MouseDrawn = true; return; }

    uint8_t mousePointer[] = {(uint8_t)255U, (uint8_t)224U, (uint8_t)255U, (uint8_t)128U, (uint8_t)254U, (uint8_t)0U, (uint8_t)252U, (uint8_t)0U, (uint8_t)248U, (uint8_t)0U, (uint8_t)240U, (uint8_t)0U, (uint8_t)224U, (uint8_t)0U, (uint8_t)192U, (uint8_t)0U, (uint8_t)192U, (uint8_t)0U, (uint8_t)128U, (uint8_t)0U, (uint8_t)128U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U, (uint8_t)0U};

    int xMax = 16;
    int yMax = 16;
    int differenceX = framebuffer->Width - c.X;
    int differenceY = framebuffer->Height - c.Y;

    if (differenceX < 16) xMax = differenceX;
    if (differenceY < 16) yMax = differenceY;
    for (int y = 0; y < yMax; y++){
        for (int x = 0; x < xMax; x++){
            int bit = y * 16 + x;
            int byte = bit / 8;
            if ((mousePointer[byte] & (0b10000000 >> (x % 8))))
            {
                if (GetPix(c.X + x, c.Y + y) == mbuffa[x + y *16]){
                    PutPix(c.X + x, c.Y + y, mbuff[x + y * 16]);
                }
            }
        }
    }
}