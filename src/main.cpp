#include <gba.h>

// Draw a pixel
void drawPixel(volatile u16* buffer, int x, int y, u8 color) {
    int index = y * 240 + x;
    int address = index / 2;

    u16 value = buffer[address];

    if (index & 1) {
        value = (value & 0x00FF) | (color << 8);
    } else {
        value = (value & 0xFF00) | color;
    }

    buffer[address] = value;

}





// Main
int main() {
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_4 | BG2_ON);


    // Background Pallette
    BG_COLORS[0] = RGB5(0,0,0);
    BG_COLORS[1] = RGB5(31,31,31);

    volatile u16* framebuffer =
        reinterpret_cast<volatile u16*>(0x06000000);

    for (int i=0; i < 240; i++) {
        framebuffer[i] = 0;
    }

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