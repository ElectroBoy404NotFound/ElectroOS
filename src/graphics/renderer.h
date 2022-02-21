#pragma once

#include "stddef.h"
#include "stdint.h"

#include "graphics.h"
#include "../util/math.h"

class Print {
    public:
        Print(Framebuffer* frameBuffer, PSF1_FONT* psf1_font);
        void print(const char* str);
        void setCursor(Point point);
        void setCursor(unsigned int x, unsigned int y);
        Point getCursorPosition();
        void setColor(unsigned int colour);
        void println(const char* str) {
            print(str);
            print("\n");
        }
        Framebuffer* getFramebuffer() { return framebuffer; }
        void clearScreen(uint32_t colour);
        void clearChar(uint32_t colour);
        void putChar(char chr);
    private:
        Point CursorPosition;
        Framebuffer* framebuffer;
        PSF1_FONT* psf1_font;
        unsigned int colour;
        void putChar(char chr, unsigned int xOff, unsigned int yOff);
};

class MouseRenderer {
    public:
        MouseRenderer(Framebuffer* frameBuffer, PSF1_FONT* psf1_font);
        void render(Point position);
        void clear(Point position);
        void setColor(unsigned int color);
    private:
        Framebuffer* framebuffer;
        PSF1_FONT* psf1_font;
        unsigned int colour;
        Point mouseLastRender = {0, 0};
        uint32_t mbuff[16 * 16];
        uint32_t mbuffa[16 * 16];
        bool MouseDrawn;
        void PutPix(uint32_t x, uint32_t y, uint32_t colour);
        uint32_t GetPix(uint32_t x, uint32_t y);
};
