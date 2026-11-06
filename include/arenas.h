// iw_arena.h

#ifndef DELTARUNEGBA_IW_ARENA_H
#define DELTARUNEGBA_IW_ARENA_H

#include <cstdint>

#include "palette.h"
#include "room.h"
#include "types.h"

namespace game {

    struct Frequented {
        uint16_t playerHealth;
        uint16_t playerTP;
        uint16_t playerVelocity;
        uint16_t tick;

        [[nodiscard]] int8_t getVelocityX() const {
            return static_cast<int8_t>(playerVelocity >> 8);
        }

        [[nodiscard]] int8_t getVelocityY() const {
            return static_cast<int8_t>(playerVelocity & 0xFF);
        }

        void setVelocityX(int8_t x) {
            playerVelocity = (playerVelocity & 0x00FF) |
                (static_cast<uint16_t>(static_cast<uint8_t>(x)) << 8);
        }

        void setVelocityY(int8_t y) {
            playerVelocity = (playerVelocity & 0xFF00) |
                static_cast<uint8_t>(y);
        }
    };

    /**
     * A stack with a size of 256, stores uint16's intended for use for temporary variables
     */
    struct Stack {
        int16_t stack[255]{};
        uint8_t top = 0;
        bool empty = true;

        bool push(const int16_t value) {
            if (top >= 255 && !empty)
                return false;

            stack[top] = value;
            ++top;
            empty = false;
            return true;
        }

        bool pop(int16_t& value) {
            if (empty)
                return false;

            value = stack[--top];

            if (top == 0)
                empty = true;

            return true;
        }
    };

    struct Entities {
        Entity entities[32];
        EntityPosition playerPosition;
    };

    // Small, hot state: stays in fast 32 KB IWRAM.
    struct GameArena {
        Stack gameStack{};
        Frequented frequented{};
        Entities entities{};
    };

    struct IwramArena {
        GameArena game{};
        uint8_t remaining[8192 - sizeof(GameArena)]{};
    };

    // Big state: lives in 256 KB EWRAM.
    struct EwramArena {
        // Software back buffer (Mode 4, 8bpp). Everything is drawn here with
        // plain byte writes, then block-copied to the hidden VRAM page.
        alignas(4) uint8_t backBuffer[240 * 160];

        // The currently loaded room.
        RoomInfo room{};

        // Merged palette + per-sprite index remap tables for that room.
        RoomPalette palette{};
    };

    extern IwramArena iwramArena;
    extern EwramArena ewramArena;

    static_assert(sizeof(Entity) == 8);
    static_assert(sizeof(Frequented) == 8);
    static_assert(sizeof(Entities) == 260);
    static_assert(sizeof(Stack) == 512);
    static_assert(sizeof(IwramArena) == 8192);
    static_assert(sizeof(EwramArena) <= 256 * 1024, "EWRAM arena does not fit in 256 KB");
}

#define GAME_STATE (game::iwramArena.game)
#define CURRENT_ROOM (game::ewramArena.room)
#define BACK_BUFFER (game::ewramArena.backBuffer)
#define ROOM_PALETTE (game::ewramArena.palette)

#endif