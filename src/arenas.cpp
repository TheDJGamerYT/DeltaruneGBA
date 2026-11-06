#include "arenas.h"

namespace game {

    __attribute__((section(".iwram"), aligned(4)))
    IwramArena iwramArena{};

    // ".sbss" is devkitARM's zero-initialised EWRAM section (EWRAM_BSS in libgba).
    // Using ".ewram" instead would store the whole (all-zero) arena in the ROM
    // image and copy it at startup, adding ~100 KB to the ROM.
    // Everything in this struct must therefore default to zero.
    __attribute__((section(".sbss"), aligned(4)))
    EwramArena ewramArena{};

}