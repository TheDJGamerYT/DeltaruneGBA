//
// Created by george on 11/3/26.
//

#ifndef DELTARUNEGBA_SAVE_MANAGER_H
#define DELTARUNEGBA_SAVE_MANAGER_H

#define SRAM_ADDR ((volatile u8*) 0x0E000000)
#include <gba_types.h>

namespace sram {
    struct Save_Manager {
        // Function to write a buffer of data to SRAM
        static void write_sram(u32 offset, const void* source, u32 size) {
            const u8* src = (const u8*) source;
            volatile u8* dst = SRAM_ADDR + offset;

            for (u32 i = 0; i < size; i++) {
                dst[i] = src[i];
                // TODO: small delay or NOP if the flash/SRAM chip requires write recovery time
            }
        }

        // Function to read a buffer of data from SRAM
        static void read_sram(u32 offset, void* dest, u32 size) {
            u8* dst = (u8*) dest;
            volatile u8* src = SRAM_ADDR + offset;

            for (u32 i = 0; i < size; i++) {
                dst[i] = src[i];
            }
        }

        static void fill_sram(u32 size, u8 fill_value) {
            volatile u8* sram_ptr = SRAM_ADDR;

            for (u32 i = 0; i < size; i++) {
                sram_ptr[i] = fill_value;
            }
        }
    };

    extern Save_Manager save_manager;
}

#define SRAM_ACCESS sram::save_manager

#endif //DELTARUNEGBA_SAVE_MANAGER_H
