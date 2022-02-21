#pragma once

#include "../io.h"

#include "../../util/utils.h"

class Keyboard {
    public:
        Keyboard() {}
        void init(UtilClasses* utils);
        void onKey();
    private:
        UtilClasses* utils;
        bool isLeftShiftPressed;
        bool isRightShiftPressed;
};

extern Keyboard keyboard;