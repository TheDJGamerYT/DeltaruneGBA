#include "arenas.h"

namespace game {

    __attribute__((section(".iwram"), aligned(4)))
    IwramArena iwramArena{};

    __attribute__((section(".ewram"), aligned(4)))
    uint8_t ewramArena[32768]{};

}