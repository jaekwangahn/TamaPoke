#include "audio.h"
#include "pin_config.h"
#include <Arduino.h>
#include <Wire.h>
#include <ESP_I2S.h>
#include <Preferences.h>
#include <atomic>
#include <math.h>

// ---------------------------------------------------------------------------
// Audio del TamaPoke: códec ES8311 (DAC -> amplificador PA -> altavoz) por I2S.
// Init del ES8311 portado del driver oficial de Espressif (esp-bsp), fijado a
// MCLK=4.096MHz (256*fs), 16kHz, 16-bit, esclavo I2S. Los efectos son tonos
// cuadrados (estilo Game Boy) sintetizados en una tarea aparte para no
// bloquear el loop de juego.
// ---------------------------------------------------------------------------

#define ES8311_ADDR 0x18
#define SAMPLE_RATE 16000

static I2SClass i2s;
static bool gReady = false;
static std::atomic<bool> gOn{true}, gMusicOn{true};
static std::atomic<uint32_t> gMusicContext{0}, gMusicBlocks{0};
static std::atomic<bool> gSleeping{false};
static bool gIdle = false;          // sin interaccion reciente (ver power.h): el amplificador consume en reposo
static uint32_t gAmpHoldUntil = 0;  // un efecto pedido en reposo mantiene el amplificador encendido hasta aqui
static bool gAmpHold = false;       // (indicador aparte: asi la comparacion no falla al dar la vuelta millis())
static std::atomic<uint32_t> gAmpOnAt{0};       // cuando se encendio el amplificador
static QueueHandle_t gQ = nullptr;

// El NS4150B tarda bastante mas de 8 ms en estabilizarse tras cada apagado, asi
// que encenderlo justo antes de cada efecto se comia los cortos: el jingle de
// arranque (440 ms) se oia y los pitidos de 35 ms no. Se deja encendido mientras
// haya sonido y la mascota este despierta, como en el ejemplo verificado de
// Waveshare, y solo se apaga al dormir o al silenciar.
static void updateAmplifierPower() {
  // gReady evita encenderlo en una placa donde el codec no arranco: ahi el
  // amplificador solo aportaria siseo, porque no va a sonar nada.
  if (gAmpHold && (int32_t)(gAmpHoldUntil - millis()) <= 0) gAmpHold = false;
  bool idleOff = gIdle && !gAmpHold;
  static int last = -1;  // solo se escribe el pin si cambia
  int want = (gReady && !gSleeping && (gMusicOn || (gOn && !idleOff))) ? HIGH : LOW;
  if (want != last) {
    last = want;
    if (want == HIGH) gAmpOnAt = millis();
    digitalWrite(PA, want);
  }
}

// ---- I2C del códec ----
static bool esW(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}
static uint8_t esR(uint8_t reg) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ES8311_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

// Secuencia de init VERIFICADA EN ESTA PLACA (proyecto PlaneRadar2.0, misma
// Waveshare 1.75). Clave: reloj DERIVADO DEL BCLK (reg01=0xBF, sin MCLK externo)
// y referencia interna que alimenta el DAC (reg44=0x58); sin esos dos el codec
// respondia por I2C pero no salia audio. 16kHz, 16-bit, esclavo I2S.
static bool es8311Init() {
  Wire.beginTransmission(ES8311_ADDR);
  if (Wire.endTransmission() != 0) return false;

  // open()
  esW(0x0D, 0xFA); esW(0x44, 0x08); esW(0x44, 0x08);  // power up + quirk de 1a escritura
  esW(0x01, 0x30); esW(0x02, 0x00); esW(0x03, 0x10); esW(0x16, 0x24);
  esW(0x04, 0x10); esW(0x05, 0x00); esW(0x0B, 0x00); esW(0x0C, 0x00);
  esW(0x10, 0x1F); esW(0x11, 0x7F);
  esW(0x00, 0x80); esW(0x00, 0x80);                   // reset clock, esclavo
  esW(0x01, 0xBF);                                    // clk src = BCLK (sin MCLK externo)
  { uint8_t r = esR(0x06); r &= ~0x20; esW(0x06, r); }  // SCLK no invertido
  esW(0x13, 0x10); esW(0x1B, 0x0A); esW(0x1C, 0x6A);
  esW(0x44, 0x58);                                    // referencia interna -> alimenta el DAC

  // config_sample(): BCLK*8 = DIG_MCLK
  esW(0x02, 0x18); esW(0x05, 0x00); esW(0x03, 0x10); esW(0x04, 0x20);
  { uint8_t r = esR(0x07); r &= 0xC0; esW(0x07, r); }
  esW(0x08, 0xFF);
  { uint8_t r = esR(0x06); r &= 0xE0; r |= 0x03; esW(0x06, r); }  // bclk_div=4

  // formato I2S 16-bit
  esW(0x09, 0x0C); esW(0x0A, 0x0C);

  // start() DAC esclavo
  esW(0x00, 0x80); esW(0x01, 0xBF); esW(0x09, 0x0C); esW(0x0A, 0x0C);
  esW(0x17, 0xBF); esW(0x0E, 0x02); esW(0x12, 0x00); esW(0x14, 0x1A);
  esW(0x0D, 0x01); esW(0x15, 0x40); esW(0x37, 0x08); esW(0x45, 0x00);

  // volumen + unmute
  esW(0x32, 0xBF);                                    // volumen DAC ~0 dB
  { uint8_t r = esR(0x31); r &= 0x9F; esW(0x31, r); }  // unmute
  return true;
}

// ---- sintetizador de tono cuadrado ----
struct Note { uint16_t f, ms; };

static const Note N_TAP[]    = {{880, 35}};
static const Note N_EAT[]    = {{660, 45}, {0, 12}, {660, 45}};
static const Note N_PLAY[]   = {{784, 45}, {988, 60}};
static const Note N_HEART[]  = {{1047, 55}, {1319, 90}};
static const Note N_HATCH[]  = {{523, 80}, {659, 80}, {784, 110}, {1047, 170}};
static const Note N_EVOLVE[] = {{523, 80}, {659, 80}, {784, 80}, {1047, 90}, {1319, 230}};
static const Note N_MEDAL[]  = {{784, 70}, {0, 25}, {784, 70}, {0, 25}, {1047, 200}};
static const Note N_DENY[]   = {{300, 110}, {200, 170}};
static const Note N_BYE[]    = {{784, 150}, {659, 150}, {523, 280}};
static const Note N_LEVEL[]  = {{784, 70}, {1047, 130}};

struct SfxDef { const Note *n; uint8_t len; };
static const SfxDef SFX[SFX_COUNT] = {
  {N_TAP, 1}, {N_EAT, 3}, {N_PLAY, 2}, {N_HEART, 2}, {N_HATCH, 4},
  {N_EVOLVE, 5}, {N_MEDAL, 5}, {N_DENY, 2}, {N_BYE, 3}, {N_LEVEL, 2},
};

static int16_t buf[256 * 2];  // estéreo intercalado (L=R)

// One task owns I2S. Render music and effects in 16 ms blocks so a switch
// or sleep transition can take effect without waiting for a whole phrase.
struct Voice {
  uint32_t phase = 0, increment = 0;
  void start(uint8_t midi) {
    phase = 0;
    increment = midi ? static_cast<uint32_t>(440.0 * pow(2.0, (midi - 69) / 12.0)
                                             * 4294967296.0 / SAMPLE_RATE) : 0;
  }
  int sample(const int16_t *wave, uint8_t harmonic, int gain, int envelope) {
    if (!increment) return 0;
    const uint32_t p = phase;
    phase += increment;
    const int value = wave[p >> 24] + wave[(p * harmonic) >> 24] / 4;
    return value * gain * envelope / (256 * 8);
  }
};

static void audioTask(void *) {
  int16_t wave[256];
  for (int i = 0; i < 256; ++i) wave[i] = lroundf(1600 * sinf(i * 6.2831853f / 256));
  Voice lead, harmony, bass;
  uint8_t step = 0, id = SFX_COUNT, noteIndex = 0;
  int position = 0, length = 0, fxPosition = 0, fxLength = 0, fxHalf = 0;
  uint32_t activeContext = UINT32_MAX;
  MusicVoice instrument = MusicVoice::Felt;
  for (;;) {
    const bool sleeping = gSleeping;
    const bool music = gMusicOn && !sleeping;
    const bool effects = gOn && !sleeping;
    if (!effects) {
      id = SFX_COUNT;
      fxLength = 0;
      xQueueReset(gQ);
    }
    if (!music) length = 0;
    if (!music && id == SFX_COUNT && uxQueueMessagesWaiting(gQ) == 0) {
      vTaskDelay(pdMS_TO_TICKS(12));
      continue;
    }
    if (millis() - gAmpOnAt.load() < 50) {
      vTaskDelay(pdMS_TO_TICKS(8));
      continue;
    }
    if (effects && id == SFX_COUNT && xQueueReceive(gQ, &id, 0)) noteIndex = 0;
    const uint32_t context = gMusicContext.load();
    for (int i = 0; i < 256; ++i) {
      int value = 0;
      if (music) {
        if (position >= length || activeContext != context) {
          if (activeContext != context) step = 0;
          activeContext = context;
          MusicStep bar = musicStep({MusicPlace(context & 3), MusicLight((context >> 2) & 3),
                                     MusicWeather((context >> 4) & 3), uint8_t(context >> 8)}, step++);
          lead.start(bar.lead);
          harmony.start(bar.harmony);
          bass.start(bar.bass);
          position = 0;
          length = SAMPLE_RATE * bar.milliseconds / 1000;
          instrument = bar.voice;
        }
        const int edge = min(256, min(position, length - position));
        const int decay = 256 - position * (instrument == MusicVoice::Felt ? 110 : 220) / length;
        const int env = edge * decay / 256;
        value = lead.sample(wave, instrument == MusicVoice::Bell ? 3 : 2, 2, env)
              + harmony.sample(wave, 2, 1, env)
              + bass.sample(wave, 2, 1, env);
        // Keep the instrumental bed well below the game cues.
        ++position;
      }
      if (effects) {
        if (id < SFX_COUNT && fxLength == 0) {
          const Note &note = SFX[id].n[noteIndex++];
          fxLength = SAMPLE_RATE * note.ms / 1000;
          fxHalf = note.f ? SAMPLE_RATE / (2 * note.f) : 0;
          fxPosition = 0;
        }
        if (fxLength) {
          const int ramp = min(64, min(fxPosition, fxLength - fxPosition));
          if (fxHalf) value += ((fxPosition / fxHalf) % 2 ? -5000 : 5000) * ramp / 64;
          if (++fxPosition >= fxLength) {
            fxLength = 0;
            if (noteIndex == SFX[id].len) id = SFX_COUNT;
          }
        }
      }
      buf[i * 2] = buf[i * 2 + 1] = value;
    }
    const size_t written = i2s.write(reinterpret_cast<uint8_t *>(buf), sizeof(buf));
    if (music && written == sizeof(buf)) ++gMusicBlocks;
    if (written == 0) vTaskDelay(pdMS_TO_TICKS(12));
  }
}

void audioBegin() {
  // I2S primero: arranca el MCLK que necesita el códec para engancharse
  pinMode(PA, OUTPUT);
  digitalWrite(PA, LOW);   // hasta saber si el sonido esta activado

  i2s.setPins(I2S_BCK_IO, I2S_WS_IO, I2S_DO_IO, I2S_DI_IO, I2S_MCK_IO);
  if (!i2s.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                 I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
    Serial.println("I2S begin fallo");
    return;
  }
  if (!es8311Init()) { Serial.println("ES8311 no responde (audio off)"); return; }

  Preferences p;
  p.begin("tamapoke", true);
  gOn = p.getBool("snd", true);
  gMusicOn = p.getBool("bgm", gOn.load());
  p.end();

  gQ = xQueueCreate(8, sizeof(uint8_t));
  if (!gQ || xTaskCreatePinnedToCore(audioTask, "audio", 4096, nullptr, 1, nullptr, 0) != pdPASS) return;
  gReady = true;
  updateAmplifierPower();
  sfxPlay(SFX_HATCH);  // jingle de arranque (confirma que suena)
}

bool audioBusy() { return (gReady && gMusicOn && !gSleeping) ||
    (gAmpHold && (int32_t)(gAmpHoldUntil - millis()) > 0); }

void sfxPlay(uint8_t id) {
  if (id < SFX_COUNT && gReady && gOn && !gSleeping && gQ) {
    if (gIdle) {  // un aviso en reposo (p. ej. "listo para evolucionar") tiene que oirse
      gAmpHoldUntil = millis() + 4000;
      gAmpHold = true;
      updateAmplifierPower();
    }
    xQueueSend(gQ, &id, 0);  // descarta si la cola esta llena
  }
}

void audioSetEnabled(bool on) {
  gOn = on;
  updateAmplifierPower();
  Preferences p;
  p.begin("tamapoke", false);
  p.putBool("snd", on);
  p.end();
}
bool audioEnabled() { return gOn; }

void audioSetSleeping(bool sleeping) {
  if (gSleeping == sleeping) return;
  gSleeping = sleeping;
  updateAmplifierPower();  // durmiendo no hay efectos: amp apagado, sin consumo
}

void audioSetIdle(bool idle) {
  gIdle = idle;
  updateAmplifierPower();  // tambien cuando vence el mantenimiento de un aviso
}

bool audioMusicEnabled() { return gMusicOn; }

void audioSetMusicEnabled(bool on) {
  gMusicOn = on;
  updateAmplifierPower();
  Preferences p;
  p.begin("tamapoke", false);
  p.putBool("bgm", on);
}

void audioSetMusicContext(MusicContext context) {
  gMusicContext.store(uint32_t(context.place) | (uint32_t(context.light) << 2) |
                      (uint32_t(context.weather) << 4) | (uint32_t(context.region) << 8));
}

void audioPrintStatus() {
  const uint32_t context = gMusicContext.load();
  Serial.printf("audio ready=%d sfx=%d bgm=%d sleeping=%d place=%u light=%u weather=%u region=%u blocks=%lu\n",
                gReady, gOn.load(), gMusicOn.load(), gSleeping.load(),
                unsigned(context & 3), unsigned((context >> 2) & 3),
                unsigned((context >> 4) & 3), unsigned(context >> 8),
                static_cast<unsigned long>(gMusicBlocks.load()));
}
