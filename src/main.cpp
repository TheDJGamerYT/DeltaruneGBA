#include <gba.h>

// Draw a pixel
void drawPixel(volatile u16* buffer, int x, int y, u8 color) {
    int index;
}


// Main
int main() {



    // main loop
    while (true) {
        // VBlank and Input
        VBlankIntrWait();
        scanKeys();
        u16 pressed = keysDown();
        u16 held = keysHeld();
        u16 released = keysUp();


    }

}