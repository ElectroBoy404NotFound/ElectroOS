#pragma once

#include "../io.h"

#include "../../util/utils.h"

class Mouse {
    public:
        Mouse() {}
        void init(UtilClasses* utils);
        void onIntrrupt();
        void process();
    private:
        UtilClasses* utils;
};

extern Mouse mouse;