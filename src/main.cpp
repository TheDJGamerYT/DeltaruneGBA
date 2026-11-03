#include <gba.h>

void drawPixel(volatile u16* buffer, int x, int y, u8 color) {
    int index;
}



int main() {




    while (true) {
        VBlankIntrWait();
        scanKeys();
        u16 pressed = keysDown();
        u16 held = keysHeld();
        u16 released = keysUp();


    }

}