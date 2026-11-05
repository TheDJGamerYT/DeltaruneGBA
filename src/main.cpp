#include <gba.h>

#include "arenas.h"
#include "save_manager.h"

#define PLATFORM_GBA true
#define PLATFORM_3DS false
#define PLATFORM_PC false

namespace {
    volatile u8 *framebuffer0 =
            reinterpret_cast<volatile u8 *>(0x06000000);

    volatile u8 *framebuffer1 =
            reinterpret_cast<volatile u8 *>(0x0600A000);

    volatile u8 *drawBuffer = framebuffer1;
    // Draw a pixel
    void drawPixel(volatile u8 *buffer, int x, int y, u8 color) {
        if (x < 0 || x >= 240 || y < 0 || y >= 160)
            return;

        buffer[y * 240 + x] = color;
    }

    void drawRoom(volatile u8 *buffer) {
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 30; x++) {
                const u8 tile = GAME_STATE.currentRoom.tileID[x][y];

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


    void movementTick() {
        auto &pos = GAME_STATE.entities.playerPosition;
        auto &room = GAME_STATE.currentRoom;

        const int8_t velX = GAME_STATE.frequented.getVelocityX();
        const int8_t velY = GAME_STATE.frequented.getVelocityY();

        // Vertical movement
        if (velY != 0) {
            const int step = velY > 0 ? 1 : -1;
            const int steps = velY > 0 ? velY : -velY;

            for (int i = 0; i < steps; i++) {
                const int nextY = static_cast<int>(pos.y) + step;

                if (nextY < 0 || nextY >= 160)
                    break;

                const int tileX = pos.x >> 3;
                const int tileY = nextY >> 3;

                if (room.tileID[tileX][tileY] != 0) {
                    GAME_STATE.frequented.setVelocityY(0);
                    break;
                }

                pos.y = nextY;
            }
        }

        // Horizontal movement
        if (velX != 0) {
            const int step = velX > 0 ? 1 : -1;
            const int steps = velX > 0 ? velX : -velX;

            for (int i = 0; i < steps; i++) {
                const int nextX = static_cast<int>(pos.x) + step;

                if (nextX < 0 || nextX >= 240)
                    break;

                const int tileX = nextX >> 3;
                const int tileY = pos.y >> 3;

                if (room.tileID[tileX][tileY] != 0) {
                    GAME_STATE.frequented.setVelocityX(0);
                    break;
                }

                pos.x = nextX;
            }
        }

        if (GAME_STATE.frequented.tick % 2 == 1) {
            if (velX != 0)
                GAME_STATE.frequented.setVelocityX(velX > 0 ? velX - 1 : velX + 1);
            if (velY != 0)
                GAME_STATE.frequented.setVelocityY(velY > 0 ? velY - 1 : velY + 1);
        }
    }

    void frameTick() {
#if PLATFORM_GBA
        // Switch displayed page
        REG_DISPCNT ^= (1 << 4);

        // Switch drawing page
        drawBuffer = (drawBuffer == framebuffer0)
                         ? framebuffer1
                         : framebuffer0;

        drawRoom(drawBuffer);

        drawPixel(drawBuffer,
                  GAME_STATE.entities.playerPosition.x,
                  GAME_STATE.entities.playerPosition.y,
                  1);
#endif
    }

    void initGraphics() {
#if PLATFORM_GBA
        irqInit();
        irqEnable(IRQ_VBLANK);

        SetMode(MODE_4 | BG2_ON);

        // Background Pallette
        BG_COLORS[0] = RGB5(0, 0, 0); // Black
        BG_COLORS[1] = RGB5(31, 31, 31); // White
        BG_COLORS[2] = RGB5(31, 0, 0); // Red
        BG_COLORS[3] = RGB5(0, 31, 0); // Green
        BG_COLORS[4] = RGB5(0, 0, 31); // Blue
        BG_COLORS[5] = RGB5(31, 31, 0); // Yellow
        BG_COLORS[6] = RGB5(31, 0, 31); // Magenta
        BG_COLORS[7] = RGB5(0, 31, 31); // Cyan

        for (int i = 0; i < 240 * 160; i++) {
            framebuffer0[i] = 0;
            framebuffer1[i] = 0;
        }
#endif
    }

    void initRoom() {
        GAME_STATE.currentRoom.tileID[0][5] = 1;
        GAME_STATE.currentRoom.tileID[1][5] = 2;
        GAME_STATE.currentRoom.tileID[1][6] = 3;
        GAME_STATE.currentRoom.tileID[14][6] = 4;
        GAME_STATE.currentRoom.tileID[5][3] = 5;
    }
}

int main() {
    initRoom();

    initGraphics();

    int timesPressed = SRAM_ADDR[0];

    // main loop
    while (true) {
        // VBlank and Input
        VBlankIntrWait();
        GAME_STATE.frequented.tick++;

        scanKeys();
        u16 pressed = keysDown();
        u16 held = keysHeld();
        u16 released = keysUp();

        int8_t velX = GAME_STATE.frequented.getVelocityX();
        int8_t velY = GAME_STATE.frequented.getVelocityY();

        if ((pressed | held) & KEY_LEFT) {
            // const auto prevVel = static_cast<int8_t>(GAME_STATE.frequented.playerVelocity >> 8);
            // const uint16_t newVel = ((prevVel > -10 ? prevVel - 1 : prevVel) << 8) | (GAME_STATE.frequented.playerVelocity & 255);
            // GAME_STATE.frequented.playerVelocity = newVel;

            if (velX > -10)
            GAME_STATE.frequented.setVelocityX(velX - 1);

            // pos.x--;
        }

        if ((pressed | held) & KEY_RIGHT) {
            // const auto prevVel = static_cast<int8_t>(GAME_STATE.frequented.playerVelocity >> 8);
            // const uint16_t newVel = ((prevVel < 10 ? prevVel + 1 : prevVel) << 8) | (GAME_STATE.frequented.playerVelocity & 255);
            // GAME_STATE.frequented.playerVelocity = newVel;
            if (velX < 10)
                GAME_STATE.frequented.setVelocityX(velX + 1);
            // pos.x++;
        }

        if ((pressed | held) & KEY_UP) {
            // const auto prevVel = static_cast<int8_t>(GAME_STATE.frequented.playerVelocity & 0xFF);
            // const uint16_t newVel = (GAME_STATE.frequented.playerVelocity & ~((uint16_t)255)) | (prevVel > -10 ? prevVel - 1 : prevVel);
            // GAME_STATE.frequented.playerVelocity = newVel;
            if (velY > -10)
                GAME_STATE.frequented.setVelocityY(velY - 1);
            // pos.y--;
        }

        if ((pressed | held) & KEY_DOWN) {
            // const auto prevVel = static_cast<int8_t>(GAME_STATE.frequented.playerVelocity & 0xFF);;
            // const uint16_t newVel = (GAME_STATE.frequented.playerVelocity & ~((uint16_t)255)) | (prevVel < 10 ? prevVel + 1 : prevVel);
            // GAME_STATE.frequented.playerVelocity = newVel;
            if (velY < 10)
                GAME_STATE.frequented.setVelocityY(velY + 1);
            // pos.y++;
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

        movementTick();
        frameTick();
    }
}
