#if defined(TAMAPOKE_SMALLTV_PRO)
#include "smalltv_display.h"

static uint8_t rgb332(uint16_t color) {
  return ((color >> 8) & 0xE0) | ((color >> 6) & 0x1C) | ((color >> 3) & 0x03);
}

static uint16_t rgb565(uint8_t color) {
  const uint16_t r = (color >> 5) & 7;
  const uint16_t g = (color >> 2) & 7;
  const uint16_t b = color & 3;
  return ((r * 31 / 7) << 11) | ((g * 63 / 7) << 5) | (b * 31 / 3);
}

SmallTvDisplay::SmallTvDisplay(Arduino_ST7789 &panel)
    : Arduino_GFX(LCD_WIDTH, LCD_HEIGHT), panel(panel) {}

bool SmallTvDisplay::begin(int32_t speed) {
  pinMode(SMALLTV_BL, OUTPUT);
  digitalWrite(SMALLTV_BL, HIGH);
  if (!ledcAttach(SMALLTV_BL, 12000, 8)) return false;
  ledcWrite(SMALLTV_BL, 255);
  pixels = static_cast<uint8_t *>(calloc(Size * Size, 1));
  if (!pixels) return false;
  // The ST7789 driver sends INVON during initialization for an IPS panel.
  return panel.begin(speed);
}

void SmallTvDisplay::setBrightness(uint8_t level) {
  ledcWrite(SMALLTV_BL, 255 - (level * 255 / 180));
}

void SmallTvDisplay::writePixelPreclipped(int16_t x, int16_t y, uint16_t color) {
  const int px = x * Size / LCD_WIDTH;
  const int py = y * Size / LCD_HEIGHT;
  pixels[py * Size + px] = rgb332(color);
}

void SmallTvDisplay::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h,
                                            uint16_t color) {
  const int x0 = x * Size / LCD_WIDTH;
  const int y0 = y * Size / LCD_HEIGHT;
  const int x1 = (x + w - 1) * Size / LCD_WIDTH;
  const int y1 = (y + h - 1) * Size / LCD_HEIGHT;
  const uint8_t packed = rgb332(color);
  for (int py = y0; py <= y1; ++py) {
    memset(pixels + py * Size + x0, packed, x1 - x0 + 1);
  }
}

void SmallTvDisplay::setSelector(const SmallTvTarget *target, int gesture) {
  selection = target ? *target : SmallTvTarget{};
  selectedGesture = gesture;
}

void SmallTvDisplay::readRow(int y, uint16_t *row) const {
  const uint8_t *source = pixels + y * Size;
  for (int x = 0; x < Size; ++x) row[x] = rgb565(source[x]);
}

void SmallTvDisplay::flush(bool) {
  uint16_t row[Size];
  panel.startWrite();
  panel.writeAddrWindow(0, 0, Size, Size);
  for (int y = 0; y < Size; ++y) {
    readRow(y, row);
    panel.writePixels(row, Size);
  }
  panel.endWrite();

  if (selection.w > 0 && selection.h > 0) {
    const int x = constrain(selection.x * Size / LCD_WIDTH, 0, Size - 1);
    const int y = constrain(selection.y * Size / LCD_HEIGHT, 0, Size - 1);
    const int w = min(Size, (selection.x + selection.w) * Size / LCD_WIDTH) - x;
    const int h = min(Size, (selection.y + selection.h) * Size / LCD_HEIGHT) - y;
    if (w >= 4 && h >= 4) {
      panel.drawRect(x, y, w, h, 0x0000);
      panel.drawRect(x + 1, y + 1, w - 2, h - 2, 0xFFE0);
    }
  }

  if (selectedGesture >= 0) {
    static const char *const labels[] = {"LEFT", "RIGHT", "UP", "DOWN", "PET"};
    constexpr int tile = Size / 5;
    panel.fillRect(0, Size - 24, Size, 24, 0x0000);
    panel.drawRect(selectedGesture * tile, Size - 24, tile, 24, 0xFFE0);
    panel.setTextSize(1);
    panel.setTextColor(0xFFFF);
    for (int i = 0; i < 5; ++i) {
      panel.setCursor(i * tile + 7, Size - 15);
      panel.print(labels[i]);
    }
  }
}
#endif
