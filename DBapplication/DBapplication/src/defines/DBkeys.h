#pragma once
#include <conio.h>

constexpr auto W_KEY = 'w';
constexpr auto A_KEY = 'a';
constexpr auto S_KEY = 's';
constexpr auto D_KEY = 'd';
constexpr auto H_KEY = 'h';
constexpr auto UP_KEY = 1152;
constexpr auto LEFT_KEY = 1200;
constexpr auto DOWN_KEY = 1280;
constexpr auto RIGHT_KEY = 1232;
constexpr auto ENTER_KEY = 13;

static inline int parseKey(int c)
{
    if (c == 224) {  // if the first value is esc
            // handle ESC
        return _getch() << 4;
    }
    return c;
}
