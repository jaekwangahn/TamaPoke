#pragma once

#include <Arduino_GFX_Library.h>
#include "pin_config.h"

struct SmallTvTarget {
  int16_t x, y, w, h;
};

struct SmallTvTargets {
  SmallTvTarget items[32];
  uint8_t count = 0;
  uint32_t context = 0;

  void add(int x, int y, int w, int h) {
    if (count < sizeof(items) / sizeof(items[0]))
      items[count++] = {int16_t(x), int16_t(y), int16_t(w), int16_t(h)};
  }
  void yesNo() {
    add(118, 252, 100, 52);
    add(248, 252, 100, 52);
  }
};

class SmallTvDisplay : public Arduino_GFX {
public:
  static constexpr int Size = 240;

  explicit SmallTvDisplay(Arduino_ST7789 &panel);
  bool begin(int32_t speed) override;
  void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override;
  void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void flush(bool force = false) override;
  void setBrightness(uint8_t level);
  void setSelector(const SmallTvTarget *target, int gesture);
  void readRow(int y, uint16_t *row) const;

private:
  Arduino_ST7789 &panel;
  uint8_t *pixels = nullptr;
  SmallTvTarget selection{};
  int selectedGesture = -1;
};
