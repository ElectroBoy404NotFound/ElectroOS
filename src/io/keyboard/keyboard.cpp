#include "keyboard.h"

Keyboard keyboard;

void Keyboard::init(UtilClasses* utils) {
    Keyboard::utils = utils;
}

namespace QWERTYKeyboard {
    #define LeftShift 0x2A
    #define RightShift 0x36
    #define Enter 0x1C
    #define BackSpace 0x0E
    #define Spacebar 0x39

    const char ASCIITable[] = {
         0 ,  0 , '1', '2',
        '3', '4', '5', '6',
        '7', '8', '9', '0',
        '-', '=',  0 ,  0 ,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '[', ']',
         0 ,  0 , 'a', 's',
        'd', 'f', 'g', 'h',
        'j', 'k', 'l', ';',
        '\'','`',  0 , '\\',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',',
        '.', '/',  0 , '*',
         0 , ' '
    };

    char Translate(uint8_t scancode, bool uppercase){
        if (scancode > 58) return 0;

        if (uppercase){
            return ASCIITable[scancode] - 32;
        }
        else return ASCIITable[scancode];
    }
}
void Keyboard::onKey() {
    uint8_t scode = inb(0x60);

    switch (scode){
        case LeftShift:
            isLeftShiftPressed = true;
            break;
        case LeftShift + 0x80:
            isLeftShiftPressed = false;
            break;
        case RightShift:
            isRightShiftPressed = true;
            break;
        case RightShift + 0x80:
            isRightShiftPressed = false;
            break;
        case Enter:
            utils->print.print("\n");
            break;
        case Spacebar:
            utils->print.putChar(' ');
            break;
        case BackSpace:
           utils->print.clearChar(0);
           break;
        default:
            char ascii = QWERTYKeyboard::Translate(scode, isLeftShiftPressed | isRightShiftPressed);
            if (ascii != 0)
                utils->print.putChar(ascii);
    }
    
    PIC_EndMaster();
}