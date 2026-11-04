#include <cstdio>
#include <gba.h>

#include "arenas.h"
#include "save_manager.h"

// Draw a pixel
void drawPixel(volatile u8 *buffer, int x, int y, u8 color) {
    if (x < 0 || x >= 240 || y < 0 || y >= 160)
        return;

    buffer[y * 240 + x] = color;
}

void drawRoom(volatile u8 *buffer) {
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 30; x++) {
            u8 tile = GAME_STATE.currentRoom.tileID[x][y];

            for (int py = 0; py < 8; py++) {
                for (int px = 0; px < 8; px++) {
                    drawPixel(buffer,
                              x * 8 + px,
                              y * 8 + py,
                              tile
                    );
                }
            }
        }
    }
}

void initRoom() {
    GAME_STATE.currentRoom.tileID[0][5] = 1;
    GAME_STATE.currentRoom.tileID[1][5] = 2;
    GAME_STATE.currentRoom.tileID[1][6] = 3;
    GAME_STATE.currentRoom.tileID[14][6] = 4;
    GAME_STATE.currentRoom.tileID[5][3] = 5;
}

int main() {
    initRoom();
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_4 | BG2_ON);

    // Background Pallette
    BG_COLORS[0] = RGB5(0, 0, 0);       // Black
    BG_COLORS[1] = RGB5(31, 31, 31);   // White
    BG_COLORS[2] = RGB5(31, 0, 0);     // Red
    BG_COLORS[3] = RGB5(0, 31, 0);     // Green
    BG_COLORS[4] = RGB5(0, 0, 31);     // Blue
    BG_COLORS[5] = RGB5(31, 31, 0);   // Yellow
    BG_COLORS[6] = RGB5(31, 0, 31);   // Magenta
    BG_COLORS[7] = RGB5(0, 31, 31);   // Cyan
    volatile u8 *framebuffer0 =
            reinterpret_cast<volatile u8 *>(0x06000000);

    volatile u8 *framebuffer1 =
            reinterpret_cast<volatile u8 *>(0x0600A000);

    for (int i = 0; i < 240 * 160; i++) {
        framebuffer0[i] = 0;
        framebuffer1[i] = 0;
    }

    volatile u8 *drawBuffer = framebuffer1;

    int timesPressed = SRAM_ADDR[0];

    // main loop
    while (true) {
        // VBlank and Input
        VBlankIntrWait();
        // Switch displayed page
        REG_DISPCNT ^= (1 << 4);

        // Switch drawing page
        drawBuffer = (drawBuffer == framebuffer0)
                         ? framebuffer1
                         : framebuffer0;

        scanKeys();
        u16 pressed = keysDown();
        u16 held = keysHeld();
        u16 released = keysUp();

        auto& pos = GAME_STATE.entities.playerPosition;
        auto& room = GAME_STATE.currentRoom;

        if (((pressed | held) & KEY_LEFT) &&
            (pos.getOffsetX() != 0 ||
             room.tileID[pos.getTileX() - 1][pos.getTileY()] == 0))
        {
            pos.x--;
        }

        if (((pressed | held) & KEY_RIGHT) &&
            (pos.getOffsetX() != 7 ||
             room.tileID[pos.getTileX() + 1][pos.getTileY()] == 0))
        {
            pos.x++;
        }

        if (((pressed | held) & KEY_UP) &&
            (pos.getOffsetY() != 0 ||
             room.tileID[pos.getTileX()][pos.getTileY() - 1] == 0))
        {
            pos.y--;
        }

        if (((pressed | held) & KEY_DOWN) &&
            (pos.getOffsetY() != 7 ||
             room.tileID[pos.getTileX()][pos.getTileY() + 1] == 0))
        {
            pos.y++;
        }

        if (pressed & KEY_A)
            timesPressed++;

        if (pressed & KEY_START) {
            // SRAM_ACCESS.write_sram(0, "Hello, world!", 13);
            SRAM_ACCESS.fill_sram(40, 0xFF);
            // SRAM_ACCESS.write_sram(0, "Hi", 3);
            SRAM_ADDR[0] = timesPressed;
            SRAM_ADDR[1] = GAME_STATE.entities.playerPosition.x;
            SRAM_ADDR[2] = GAME_STATE.entities.playerPosition.y;
            break;
        }

        drawRoom(drawBuffer);

        drawPixel(drawBuffer,
                  GAME_STATE.entities.playerPosition.x,
                  GAME_STATE.entities.playerPosition.y,
                  1);
    }
}
