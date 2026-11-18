#pragma once
#include <stdint.h>

constexpr uint16_t STUDY_LEVEL_CAP = 998;

struct StudyProgress {
  uint32_t endsAt = 0;
  uint8_t hours = 0;
  uint16_t bonus = 0;
  bool awarded = false;

  static StudyProgress decode(uint64_t record) {
    // Older saves only used bits 0–39, with the duration in the low byte.
    StudyProgress progress;
    progress.endsAt = uint32_t(record >> 8);
    progress.hours = uint8_t(record & 0x7f);
    progress.bonus = uint16_t((record >> 40) & 0x3ff);
    progress.awarded = (record & 0x80) != 0;
    return progress;
  }

  uint64_t encode() const {
    return (uint64_t(bonus) << 40) | (uint64_t(endsAt) << 8) | (awarded ? 0x80 : 0) | hours;
  }

  bool complete(uint32_t now) {
    if (awarded || !endsAt || now < endsAt) return false;
    const uint16_t levels = bonus + hours;
    bonus = levels > STUDY_LEVEL_CAP ? STUDY_LEVEL_CAP : levels;
    awarded = true;
    return true;
  }
};
