#ifndef DELTARUNEGBA_ROOM_H
#define DELTARUNEGBA_ROOM_H
#include <cstdint>

// Maximum room size in tiles (8x8 px each). 256x256 = 64 KB per layer
// (floor + tile layer = 128 KB), i.e. rooms up to 2048x2048 px.
// Lives in EWRAM (see arenas.h); lower these if you need the space.
constexpr int ROOM_MAX_WIDTH = 256;
constexpr int ROOM_MAX_HEIGHT = 256;

struct RoomInfo {
    // Actual size of the loaded room, in tiles (<= ROOM_MAX_*).
    // Both default to 0 on purpose: the whole struct must be all-zero so it
    // can live in a zero-initialised (.sbss) section without bloating the ROM.
    uint16_t width = 0;
    uint16_t height = 0;

    // Both layers are indexed [y][x] (row-major) and hold tile IDs
    // (0 = empty), which getSprite() in main.cpp maps to sprites.

    // Floor layer: purely visual, drawn first, never collides.
    uint8_t floorID[ROOM_MAX_HEIGHT][ROOM_MAX_WIDTH]{};

    // Tile layer: drawn on top of the floor; any tile for which isSolid()
    // is true blocks the player.
    uint8_t tileID[ROOM_MAX_HEIGHT][ROOM_MAX_WIDTH]{};

    [[nodiscard]] bool inBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    [[nodiscard]] uint8_t get(int x, int y) const {
        return inBounds(x, y) ? tileID[y][x] : 0;
    }

    void set(int x, int y, uint8_t tile) {
        if (inBounds(x, y))
            tileID[y][x] = tile;
    }

    [[nodiscard]] uint8_t getFloor(int x, int y) const {
        return inBounds(x, y) ? floorID[y][x] : 0;
    }

    void setFloor(int x, int y, uint8_t tile) {
        if (inBounds(x, y))
            floorID[y][x] = tile;
    }
};

#endif //DELTARUNEGBA_ROOM_H