// iw_arena.h

#ifndef DELTARUNEGBA_IW_ARENA_H
#define DELTARUNEGBA_IW_ARENA_H

#include <cstdint>

#include "room.h"
#include "types.h"

namespace game {

    struct Frequented {
        uint16_t playerHealth;
        uint16_t playerTP;
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

    struct GameArena {
        RoomInfo currentRoom{};
        Stack gameStack{};
        Frequented frequented{};
        Entities entities{};
    };

    struct IwramArena {
        GameArena game{};
        uint8_t remaining[8192 - sizeof(GameArena)]{};
    };

    extern IwramArena iwramArena;
    extern uint8_t ewramArena[32768];

    static_assert(sizeof(Frequented) == 4);
    static_assert(sizeof(Entity) == 8);
    static_assert(sizeof(Entities) == 260);
    static_assert(sizeof(Stack) == 512);
    static_assert(sizeof(RoomInfo) == 600);
    static_assert(sizeof(IwramArena) == 8192);
}

#define GAME_STATE (game::iwramArena.game)

#endif