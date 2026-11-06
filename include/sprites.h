#ifndef DELTARUNEGBA_SPRITES_H
#define DELTARUNEGBA_SPRITES_H

#include <cstdint>

// ---------------------------------------------------------------------------
// Colors
// ---------------------------------------------------------------------------

// GBA RGB555 from 5-bit channels (0-31).
constexpr uint16_t RGB555(int r, int g, int b) {
    return static_cast<uint16_t>(r | (g << 5) | (b << 10));
}

// RGB555 from ordinary 8-bit channels (0-255).
constexpr uint16_t RGB(int r, int g, int b) {
    return RGB555(r >> 3, g >> 3, b >> 3);
}

// ---------------------------------------------------------------------------
// Sprites
// ---------------------------------------------------------------------------

// Every sprite has a unique ID; it indexes the per-room palette remap table.
enum SpriteId : uint8_t {
    SPR_KRIS_IDLE,
    SPR_KRIS_WALK_1,
    SPR_KRIS_WALK_2,
    SPR_FLOOR_TILE,
    SPR_FLOOR_TILE_2,
    SPR_MISSING,
    SPRITE_COUNT
};

// Pixel data uses sprite-LOCAL palette indices:
//   0         = transparent
//   1..N      = colors[1..N]  (colors[0] is unused)
// When a room loads, the colors of every sprite in it are merged into one
// shared room palette (see palette.h) and indices are remapped at draw time.
struct DSprite {
    uint8_t id;
    uint8_t width;
    uint8_t height;
    const uint8_t* pixels;
    const uint16_t* colors;  // RGB555, colors[0] unused
    uint8_t colorCount;      // number of valid entries after colors[0]

    constexpr DSprite(
        uint8_t id,
        uint8_t width,
        uint8_t height,
        const uint8_t* pixels,
        const uint16_t* colors,
        uint8_t colorCount
    ) : id(id), width(width), height(height),
        pixels(pixels), colors(colors), colorCount(colorCount) {}

    static const DSprite KRIS_IDLE;
    static const DSprite KRIS_WALK_1;
    static const DSprite KRIS_WALK_2;
    static const DSprite FLOOR_TILE;
    static const DSprite FLOOR_TILE_2;
    static const DSprite MISSING;
};

namespace SpriteData {

    // ---- Per-sprite color tables (RGB555, index 0 unused) ----

    inline constexpr uint16_t KRIS_IDLE_COLORS[] = {
        0,
        RGB(61, 18, 14),  // 1
        RGB(104, 121, 81),  // 2
        RGB(103, 121, 81),  // 3
        RGB(60, 15, 14),  // 4
        RGB(255, 201, 13),  // 5
        RGB(181, 230, 29),  // 6
        RGB(255, 242, 0),  // 7
    };

    inline constexpr uint16_t KRIS_WALK_COLORS[] = {
        0,
        RGB(255, 255, 255),  // 1: white (placeholder sprite)
    };

    inline constexpr uint16_t FLOOR_COLORS[] = {
        0,
        RGB(231, 171, 111),  // 1: light tan
        RGB(163, 121, 83),   // 2: dark tan (grid lines)
    };

    inline constexpr uint16_t MISSING_COLORS[] = {
        0,
        RGB(255, 0, 255),    // 1: magenta
    };

    // ---- Pixel data ----

    inline constexpr uint8_t KRIS_IDLE[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 2, 3, 3, 2, 3, 3, 2, 2, 4, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 1, 1, 1, 1,
        1, 1, 1, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 1, 1,
        1, 1, 1, 1, 4, 5, 5, 4, 4, 4, 4, 5, 5, 5, 4, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 4, 5, 5, 5, 5, 5, 5, 5, 4, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 4, 5, 5, 4, 1, 1, 1, 1, 1, 1, 1,
        0, 1, 0, 1, 0, 1, 1, 1, 1, 5, 5, 1, 1, 1, 0, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 4, 4, 1, 1, 5, 5, 1, 1, 4, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 6, 6, 4, 1, 1, 1, 1, 4, 6, 6, 4, 0, 0, 0,
        0, 0, 0, 4, 6, 6, 6, 6, 4, 4, 4, 4, 6, 6, 6, 6, 4, 0, 0,
        0, 0, 0, 4, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 4, 0, 0,
        0, 0, 0, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 0, 0,
        0, 0, 4, 7, 7, 3, 7, 7, 7, 7, 7, 7, 7, 7, 3, 7, 7, 4, 0,
        0, 0, 4, 6, 6, 3, 7, 7, 7, 7, 7, 7, 7, 7, 3, 6, 6, 4, 0,
        0, 0, 4, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 4, 0,
        0, 0, 4, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 4, 0,
        0, 0, 1, 4, 4, 4, 6, 6, 6, 6, 6, 6, 6, 6, 4, 4, 4, 1, 0,
        0, 0, 1, 5, 5, 5, 4, 6, 6, 6, 6, 6, 6, 4, 5, 5, 5, 1, 0,
        0, 0, 1, 5, 5, 5, 1, 4, 4, 4, 4, 4, 4, 1, 5, 5, 5, 1, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    };

    inline constexpr uint8_t KRIS_WALK_1[] = {
        0, 1, 0,
        1, 1, 1,
        1, 0, 1,
    };

    inline constexpr uint8_t KRIS_WALK_2[] = {
        0, 1, 0,
        1, 1, 1,
        0, 1, 0,
    };

    inline constexpr uint8_t MISSING[] = {
        1, 1, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1,
    };

    inline constexpr uint8_t FLOOR_TILE[] = {
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 2, 1, 1, 1, 1,
        1, 1, 1, 2, 1, 1, 1, 1,
        1, 1, 1, 2, 1, 1, 1, 1,
    };

    inline constexpr uint8_t FLOOR_TILE_2[] = {
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 2,
        1, 1, 1, 1, 1, 1, 1, 2,
        1, 1, 1, 1, 1, 1, 1, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
    };
}

// Static sprite definitions.
// All pixel and color data lives in ROM.

inline const DSprite DSprite::KRIS_IDLE = {
    SPR_KRIS_IDLE, 19, 37, SpriteData::KRIS_IDLE, SpriteData::KRIS_IDLE_COLORS, 4
};

inline const DSprite DSprite::KRIS_WALK_1 = {
    SPR_KRIS_WALK_1, 3, 3, SpriteData::KRIS_WALK_1, SpriteData::KRIS_WALK_COLORS, 1
};

inline const DSprite DSprite::KRIS_WALK_2 = {
    SPR_KRIS_WALK_2, 3, 3, SpriteData::KRIS_WALK_2, SpriteData::KRIS_WALK_COLORS, 1
};

inline const DSprite DSprite::MISSING = {
    SPR_MISSING, 8, 8, SpriteData::MISSING, SpriteData::MISSING_COLORS, 1
};

inline const DSprite DSprite::FLOOR_TILE = {
    SPR_FLOOR_TILE, 8, 8, SpriteData::FLOOR_TILE, SpriteData::FLOOR_COLORS, 2
};

inline const DSprite DSprite::FLOOR_TILE_2 = {
    SPR_FLOOR_TILE_2, 8, 8, SpriteData::FLOOR_TILE_2, SpriteData::FLOOR_COLORS, 2
};

#endif // DELTARUNEGBA_SPRITES_H