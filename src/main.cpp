#include <gba.h>

#include "arenas.h"
#include "save_manager.h"
#include "sprites.h"

#define PLATFORM_GBA true
#define PLATFORM_3DS false
#define PLATFORM_PC false

namespace {
    constexpr int SCREEN_W = 240;
    constexpr int SCREEN_H = 160;
    constexpr int CENTER_X = SCREEN_W / 2;
    constexpr int CENTER_Y = SCREEN_H / 2;
    constexpr int TILE_SIZE = 8;

    // Unmapped tile ID -> MISSING sprite. Used for the test wall block.
    constexpr uint8_t TILE_TEST_WALL = 4;

    // CpuFastSet (BIOS) mode words: low 21 bits = number of 32-bit words
    // (must be a multiple of 8), bit 24 = fill instead of copy.
    constexpr u32 FRAME_WORDS = SCREEN_W * SCREEN_H / 4;
    constexpr u32 FASTSET_COPY = FRAME_WORDS;
    constexpr u32 FASTSET_FILL = FRAME_WORDS | (1u << 24);

    // REG_DISPCNT bit 7: forced blank (screen white, VRAM/palette freely accessible).
    constexpr u16 FORCE_BLANK = 1 << 7;

    // Player hitbox, relative to the sprite's top-left corner.
    // Only the feet block movement, so Kris can walk "behind" tall things.
    constexpr int HB_X = 3;
    constexpr int HB_Y = 29;
    constexpr int HB_W = 13;
    constexpr int HB_H = 8;

    volatile u8 *const framebuffer0 = reinterpret_cast<volatile u8 *>(0x06000000);
    volatile u8 *const framebuffer1 = reinterpret_cast<volatile u8 *>(0x0600A000);

    // The VRAM page that is NOT currently being displayed.
    volatile u8 *hiddenPage = framebuffer1;

    bool firstFrameInRoom = true;

    // Top-left of the screen in world pixels. Recomputed every frame.
    int camX = 0;
    int camY = 0;

    const DSprite *getSprite(const uint8_t spriteID) {
        switch (spriteID) {
            case 0: return nullptr; // Empty
            case 1: return &DSprite::FLOOR_TILE;
            case 2: return &DSprite::FLOOR_TILE_2;
            case 3: return &DSprite::KRIS_IDLE;

            default:
                return &DSprite::MISSING;
        }
    }

    // Which tiles block the player.
    bool isSolid(const uint8_t tile) {
        return tile != 0;
    }

    // ---------------------------------------------------------------------
    // Rendering (everything draws into the EWRAM back buffer)
    // ---------------------------------------------------------------------

    // Hot code: build as ARM and run from fast 32-bit IWRAM instead of Thumb from ROM.
    // (If your toolchain complains about this attribute, just define it empty.)
#ifdef __arm__
#define IWRAM_CODE __attribute__((section(".iwram"), target("arm"), long_call))
#else
#define IWRAM_CODE
#endif

    // Draw a sprite with its top-left at (screenX, screenY), clipped to the screen.
    // CheckTransparency = false is the fast path for sprites with no transparent pixels.
    template<bool CheckTransparency>
    IWRAM_CODE void blit(const DSprite *sprite, const int screenX, const int screenY) {
        if (sprite == nullptr)
            return;

        const int w = sprite->width;
        const int h = sprite->height;

        // Completely off-screen.
        if (screenX >= SCREEN_W || screenY >= SCREEN_H ||
            screenX + w <= 0 || screenY + h <= 0)
            return;

        // Visible part of the sprite, in sprite-local coordinates.
        const int x0 = screenX < 0 ? -screenX : 0;
        const int y0 = screenY < 0 ? -screenY : 0;
        const int x1 = screenX + w > SCREEN_W ? SCREEN_W - screenX : w;
        const int y1 = screenY + h > SCREEN_H ? SCREEN_H - screenY : h;

        // Sprite-local index -> room palette index for the loaded room.
        const uint8_t *lut = ROOM_PALETTE.remap[sprite->id];

        for (int py = y0; py < y1; py++) {
            const uint8_t *src = sprite->pixels + py * w;
            u8 *row = BACK_BUFFER + (screenY + py) * SCREEN_W + screenX;

            for (int px = x0; px < x1; px++) {
                const uint8_t local = src[px];

                if constexpr (CheckTransparency) {
                    // Local index 0 is transparent.
                    if (local != 0)
                        row[px] = lut[local];
                } else {
                    row[px] = lut[local];
                }
            }
        }
    }

    // Pick the fast path automatically.
    void drawSprite(const DSprite *sprite, const int screenX, const int screenY) {
        if (sprite == nullptr)
            return;

        if (ROOM_PALETTE.opaque[sprite->id])
            blit<false>(sprite, screenX, screenY);
        else
            blit<true>(sprite, screenX, screenY);
    }

    void updateCamera() {
        const auto &p = GAME_STATE.entities.playerPosition;
        const DSprite &kris = DSprite::KRIS_IDLE;

        // Center the camera on the middle of the sprite, not its top-left corner.
        camX = static_cast<int>(p.x) + kris.width / 2 - CENTER_X;
        camY = static_cast<int>(p.y) + kris.height / 2 - CENTER_Y;
    }

    // Draw one layer's tiles in the given (already clamped) tile range.
    IWRAM_CODE void drawLayer(const uint8_t (*layer)[ROOM_MAX_WIDTH],
                              const int tx0, const int ty0, const int tx1, const int ty1) {
        for (int y = ty0; y <= ty1; y++) {
            for (int x = tx0; x <= tx1; x++) {
                const uint8_t tile = layer[y][x];

                if (tile == 0)
                    continue;

                drawSprite(getSprite(tile), x * TILE_SIZE - camX, y * TILE_SIZE - camY);
            }
        }
    }

    void drawRoom() {
        const auto &room = CURRENT_ROOM;

        // Only walk the tiles that can actually be on screen.
        // (>> on a negative int is an arithmetic shift in GCC, i.e. floor division.)
        int tx0 = camX >> 3;
        int ty0 = camY >> 3;
        int tx1 = (camX + SCREEN_W - 1) >> 3;
        int ty1 = (camY + SCREEN_H - 1) >> 3;

        if (tx0 < 0) tx0 = 0;
        if (ty0 < 0) ty0 = 0;
        if (tx1 >= room.width) tx1 = room.width - 1;
        if (ty1 >= room.height) ty1 = room.height - 1;

        drawLayer(room.floorID, tx0, ty0, tx1, ty1); // floor first
        drawLayer(room.tileID, tx0, ty0, tx1, ty1);  // then solid tiles on top
    }

    // ---------------------------------------------------------------------
    // Movement / collision
    // ---------------------------------------------------------------------

    // Would the player's hitbox overlap a solid tile (or leave the room)
    // with the sprite's top-left at (x, y)?
    bool boxBlocked(const int x, const int y) {
        const auto &room = CURRENT_ROOM;

        if (x < 0 || y < 0)
            return true;

        const int left = x + HB_X;
        const int top = y + HB_Y;
        const int right = left + HB_W - 1;
        const int bottom = top + HB_H - 1;

        const int tx0 = left >> 3;
        const int tx1 = right >> 3;
        const int ty0 = top >> 3;
        const int ty1 = bottom >> 3;

        // Outside the room counts as solid.
        if (tx1 >= room.width || ty1 >= room.height)
            return true;

        for (int ty = ty0; ty <= ty1; ty++) {
            for (int tx = tx0; tx <= tx1; tx++) {
                if (isSolid(room.tileID[ty][tx]))
                    return true;
            }
        }
        return false;
    }

    void movementTick() {
        auto &pos = GAME_STATE.entities.playerPosition;
        auto &fq = GAME_STATE.frequented;

        const int8_t velX = fq.getVelocityX();
        const int8_t velY = fq.getVelocityY();

        // Vertical movement, one pixel at a time.
        if (velY != 0) {
            const int step = velY > 0 ? 1 : -1;
            const int steps = velY > 0 ? velY : -velY;

            for (int i = 0; i < steps; i++) {
                const int nextY = static_cast<int>(pos.y) + step;

                if (boxBlocked(pos.x, nextY)) {
                    fq.setVelocityY(0);
                    break;
                }

                pos.y = static_cast<uint16_t>(nextY);
            }
        }

        // Horizontal movement
        if (velX != 0) {
            const int step = velX > 0 ? 1 : -1;
            const int steps = velX > 0 ? velX : -velX;

            for (int i = 0; i < steps; i++) {
                const int nextX = static_cast<int>(pos.x) + step;

                if (boxBlocked(nextX, pos.y)) {
                    fq.setVelocityX(0);
                    break;
                }

                pos.x = static_cast<uint16_t>(nextX);
            }
        }

        // Friction. Re-read the velocities: a collision above may have zeroed
        // them, and using the stale copies would bring the old speed back.
        if (fq.tick % 2 == 1) {
            const int8_t vx = fq.getVelocityX();
            const int8_t vy = fq.getVelocityY();

            if (vx != 0)
                fq.setVelocityX(vx > 0 ? vx - 1 : vx + 1);
            if (vy != 0)
                fq.setVelocityY(vy > 0 ? vy - 1 : vy + 1);
        }
    }

    // ---------------------------------------------------------------------
    // Frame
    // ---------------------------------------------------------------------

    // Draw the whole scene into the EWRAM back buffer.
    void renderScene() {
        updateCamera();

        // Clear the back buffer (32-bit BIOS fill, much faster than a byte loop).
        static const u32 zero = 0;
        CpuFastSet(&zero, BACK_BUFFER, FASTSET_FILL);

        drawRoom();

        const auto &p = GAME_STATE.entities.playerPosition;
        drawSprite(&DSprite::KRIS_IDLE, static_cast<int>(p.x) - camX, static_cast<int>(p.y) - camY);
    }

    void frameTick() {
#if PLATFORM_GBA
        renderScene();

        // Copy the finished frame into the hidden VRAM page with 32-bit writes.
        // (Byte writes to VRAM would duplicate into the neighboring pixel.)
        CpuFastSet(BACK_BUFFER, const_cast<u8 *>(hiddenPage), FASTSET_COPY);

        // Wait for VBlank before changing the displayed page.
        VBlankIntrWait();

        // Show the page we just filled; the other one becomes the hidden page.
        if (hiddenPage == framebuffer0) {
            REG_DISPCNT &= ~(1 << 4);
            hiddenPage = framebuffer1;
        } else {
            REG_DISPCNT |= (1 << 4);
            hiddenPage = framebuffer0;
        }
#endif
    }

    void initGraphics() {
#if PLATFORM_GBA
        irqInit();
        irqEnable(IRQ_VBLANK);

        SetMode(MODE_4 | BG2_ON);

        static const u32 zero = 0;
        CpuFastSet(&zero, const_cast<u8 *>(framebuffer0), FASTSET_FILL);
        CpuFastSet(&zero, const_cast<u8 *>(framebuffer1), FASTSET_FILL);

        // Display page 0, draw into page 1.
        REG_DISPCNT &= ~(1 << 4);
        hiddenPage = framebuffer1;
#endif
    }

    void initRoom() {
        auto &room = CURRENT_ROOM;

        // Any size up to ROOM_MAX_WIDTH x ROOM_MAX_HEIGHT works.
        room.width = 64;
        room.height = 48;

        // Floor layer: covers the whole room, never collides.
        for (int y = 0; y < room.height; y++) {
            for (int x = 0; x < room.width; x++) {
                room.setFloor(x, y, (x % 2 == 0) ? 1 : 2);
            }
        }

        // Tile layer: a 4x3 test block. ID 4 has no sprite mapped, so it draws as
        // the MISSING sprite, and it's solid (see isSolid) - handy for testing collision.
        for (int y = 4; y <= 6; y++) {
            for (int x = 0; x <= 3; x++) {
                room.set(x, y, TILE_TEST_WALL);
            }
        }

        // Spawn point (the hitbox must not start inside a solid tile).
        GAME_STATE.entities.playerPosition.x = 80;
        GAME_STATE.entities.playerPosition.y = 40;
    }

    // Merge the colors of every sprite this room can show into the room palette.
    void buildRoomPalette(const uint16_t backdrop) {
        const auto &room = CURRENT_ROOM;
        auto &pal = ROOM_PALETTE;

        pal.reset(backdrop);

        // Everything that can be drawn: the player, the fallback sprite,
        // and whatever sprite each floor/tile in the room maps to.
        pal.addSprite(DSprite::KRIS_IDLE);
        pal.addSprite(DSprite::MISSING);

        for (int y = 0; y < room.height; y++) {
            for (int x = 0; x < room.width; x++) {
                const DSprite *floorSprite = getSprite(room.floorID[y][x]);
                if (floorSprite != nullptr)
                    pal.addSprite(*floorSprite);

                const DSprite *tileSprite = getSprite(room.tileID[y][x]);
                if (tileSprite != nullptr)
                    pal.addSprite(*tileSprite);
            }
        }
    }

    // Copy the merged palette into palette RAM (16-bit writes only).
    void uploadPalette() {
        const auto &pal = ROOM_PALETTE;

        for (int i = 0; i <= pal.count; i++)
            BG_COLORS[i] = pal.colors[i];
    }

    // Load a room: tiles, palette, and a first frame on both VRAM pages.
    // The screen is force-blanked so nothing is shown with a half-swapped palette.
    void loadRoom() {
#if PLATFORM_GBA
        REG_DISPCNT |= FORCE_BLANK;
#endif

        initRoom();
        buildRoomPalette(RGB5(0, 0, 0)); // black backdrop for this room

#if PLATFORM_GBA
        uploadPalette();

        // Render once and put the same frame on both pages, so the first
        // flip doesn't show a page drawn with the previous room's palette.
        renderScene();
        CpuFastSet(BACK_BUFFER, const_cast<u8 *>(framebuffer0), FASTSET_COPY);
        CpuFastSet(BACK_BUFFER, const_cast<u8 *>(framebuffer1), FASTSET_COPY);

        // Display page 0, draw into page 1.
        REG_DISPCNT &= ~(1 << 4);
        hiddenPage = framebuffer1;

        REG_DISPCNT &= ~FORCE_BLANK;
#endif
    }
}

int main() {
    initGraphics();

    loadRoom();

    int timesPressed = SRAM_ADDR[0];

    // main loop
    while (true) {
        // VBlank and Input
        GAME_STATE.frequented.tick++;

        scanKeys();
        u16 pressed = keysDown();
        u16 held = keysHeld();

        int8_t velX = GAME_STATE.frequented.getVelocityX();
        int8_t velY = GAME_STATE.frequented.getVelocityY();

        if ((pressed | held) & KEY_LEFT) {
            if (velX > -10)
                GAME_STATE.frequented.setVelocityX(velX - 1);
        }

        if ((pressed | held) & KEY_RIGHT) {
            if (velX < 10)
                GAME_STATE.frequented.setVelocityX(velX + 1);
        }

        if ((pressed | held) & KEY_UP) {
            if (velY > -10)
                GAME_STATE.frequented.setVelocityY(velY - 1);
        }

        if ((pressed | held) & KEY_DOWN) {
            if (velY < 10)
                GAME_STATE.frequented.setVelocityY(velY + 1);
        }

        if (pressed & KEY_A)
            timesPressed++;

        if (pressed & KEY_START) {
            const auto &p = GAME_STATE.entities.playerPosition;

            SRAM_ACCESS.fill_sram(40, 0xFF);
            SRAM_ADDR[0] = timesPressed;

            // Positions are 16-bit now that rooms can be large: store low/high bytes.
            SRAM_ADDR[1] = p.x & 0xFF;
            SRAM_ADDR[2] = p.x >> 8;
            SRAM_ADDR[3] = p.y & 0xFF;
            SRAM_ADDR[4] = p.y >> 8;
            break;
        }

        movementTick();

        frameTick();
    }
}