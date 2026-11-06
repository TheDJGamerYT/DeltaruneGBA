#ifndef DELTARUNEGBA_PALETTE_H
#define DELTARUNEGBA_PALETTE_H

#include <cstdint>

#include "sprites.h"

// Colors that get the same slot in EVERY room (UI, text, ...), seeded first.
// Sprites using these colors need no special handling: they dedupe into
// these slots like any other color.
inline constexpr uint16_t GLOBAL_COLORS[] = {
    RGB(255, 255, 255), // white
};
inline constexpr int GLOBAL_COLOR_COUNT = sizeof(GLOBAL_COLORS) / sizeof(GLOBAL_COLORS[0]);

// The merged palette for the currently loaded room.
//
// Slot 0 is the transparent/backdrop color; slots 1..255 hold room colors.
// Everything defaults to zero so the struct can live in zero-initialised EWRAM.
struct RoomPalette {
    uint16_t colors[256];
    uint16_t count;                         // used slots, not counting slot 0
    bool overflowed;                        // set if a room needed more than 255 colors
    bool added[SPRITE_COUNT];               // which sprites have been merged in already
    bool opaque[SPRITE_COUNT];              // sprite has no transparent pixels (fast blit path)
    uint8_t remap[SPRITE_COUNT][256];       // [sprite id][local index] -> room index

    // Start a fresh palette for a new room.
    void reset(const uint16_t backdrop) {
        for (int i = 0; i < 256; i++)
            colors[i] = 0;
        for (int s = 0; s < SPRITE_COUNT; s++) {
            added[s] = false;
            opaque[s] = false;
            for (int i = 0; i < 256; i++)
                remap[s][i] = 0;
        }

        colors[0] = backdrop;
        count = 0;
        overflowed = false;

        for (int i = 0; i < GLOBAL_COLOR_COUNT; i++)
            findOrAdd(GLOBAL_COLORS[i]);
    }

    // Room slot for this exact RGB555 color, adding it if it's new.
    uint8_t findOrAdd(const uint16_t color) {
        for (int i = 1; i <= count; i++) {
            if (colors[i] == color)
                return static_cast<uint8_t>(i);
        }

        if (count < 255) {
            colors[++count] = color;
            return static_cast<uint8_t>(count);
        }

        // Out of slots: fall back to the closest color we already have.
        overflowed = true;
        return nearest(color);
    }

    // Merge one sprite into the room palette (does nothing if already merged).
    // Only colors that actually appear in the pixel data get a slot.
    void addSprite(const DSprite &sprite) {
        if (added[sprite.id])
            return;
        added[sprite.id] = true;

        uint8_t *lut = remap[sprite.id];
        const int pixelCount = sprite.width * sprite.height;
        bool hasTransparency = false;

        for (int i = 0; i < pixelCount; i++) {
            const uint8_t local = sprite.pixels[i];

            if (local == 0) {
                hasTransparency = true;
                continue;
            }

            // lut[local] != 0 means this local index is already resolved.
            if (lut[local] != 0)
                continue;

            lut[local] = findOrAdd(sprite.colors[local]);
        }

        opaque[sprite.id] = !hasTransparency;
    }

private:
    // Closest existing slot by squared distance over the 5-bit channels.
    [[nodiscard]] uint8_t nearest(const uint16_t color) const {
        const int r = color & 31, g = (color >> 5) & 31, b = (color >> 10) & 31;

        int best = 1;
        int bestDist = 0x7FFFFFFF;

        for (int i = 1; i <= count; i++) {
            const int dr = r - (colors[i] & 31);
            const int dg = g - ((colors[i] >> 5) & 31);
            const int db = b - ((colors[i] >> 10) & 31);
            const int dist = dr * dr + dg * dg + db * db;

            if (dist < bestDist) {
                bestDist = dist;
                best = i;
            }
        }
        return static_cast<uint8_t>(best);
    }
};

#endif // DELTARUNEGBA_PALETTE_H