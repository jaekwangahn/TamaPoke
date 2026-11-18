#pragma once
#include <stdint.h>

// A companion ID outside the official Pokédex and its egg pools.
constexpr int16_t DEEPGONG_ID = 1026;

// Noti evolves at levels 10, 20, 30 and 40. The last form stays from 40 on.
constexpr uint8_t deepgongForm(uint16_t level) {
  return level >= 40 ? 4 : uint8_t(level / 10);
}
