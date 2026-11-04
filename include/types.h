//
// Created by george on 11/3/26.
//

#ifndef DELTARUNEGBA_TYPES_H
#define DELTARUNEGBA_TYPES_H

#include <cstdint>


struct EntityPosition {
    uint16_t x = 0;
    uint16_t y = 0;

    [[nodiscard]] uint8_t getTileX() const { return x >> 3; }
    [[nodiscard]] uint8_t getTileY() const { return y >> 3; }

    [[nodiscard]] uint8_t getOffsetX() const { return x & 7; }
    [[nodiscard]] uint8_t getOffsetY() const { return y & 7; }
};

typedef struct  {
    EntityPosition position;
    uint16_t entityID;
    uint16_t flags;
} Entity;

#endif //DELTARUNEGBA_TYPES_H