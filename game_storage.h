#pragma once

#include <Arduino.h>

#if defined(TAMAPOKE_SMALLTV_PRO)
#include <LittleFS.h>
inline fs::FS &gameFiles = LittleFS;

// Leave internal heap available for Wi-Fi and TLS on the no-PSRAM board.
inline void *storageAlloc(size_t bytes) {
  constexpr size_t reserve = 80 * 1024;
  if (ESP.getFreeHeap() < reserve + bytes) return nullptr;
  return malloc(bytes);
}
#else
#include <SD_MMC.h>
inline fs::FS &gameFiles = SD_MMC;
inline void *storageAlloc(size_t bytes) { return ps_malloc(bytes); }
#endif
