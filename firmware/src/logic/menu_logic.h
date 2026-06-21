#pragma once

#define MENU_PAGES 2
#define GRID_MID_X 100
#define GRID_MID_Y 95

enum Screen {
    SCREEN_MENU,
    SCREEN_CLOCK, SCREEN_POMODORO, SCREEN_PHOTOS, SCREEN_JESSIE,  // page 0
    SCREEN_5, SCREEN_6, SCREEN_7, SCREEN_8                        // page 1
};

// Pure tap-to-screen router — no hardware dependency.
inline Screen menuScreenForTap(int x, int y, int page) {
    bool top  = y < GRID_MID_Y;
    bool left = x < GRID_MID_X;
    if (page == 0) {
        if ( left &&  top) return SCREEN_CLOCK;
        if (!left &&  top) return SCREEN_POMODORO;
        if ( left && !top) return SCREEN_PHOTOS;
        return SCREEN_JESSIE;
    } else {
        if ( left &&  top) return SCREEN_5;
        if (!left &&  top) return SCREEN_6;
        if ( left && !top) return SCREEN_7;
        return SCREEN_8;
    }
}
