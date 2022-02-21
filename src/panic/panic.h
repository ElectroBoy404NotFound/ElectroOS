#pragma once

#include "../util/utils.h"

class Panic {
    public:
        Panic(){}
        void init(UtilClasses* classes);
        void panic(const char* error);
    private:
        UtilClasses* utils;
};

extern Panic panic;