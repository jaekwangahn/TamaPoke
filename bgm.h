#pragma once
#include <stdint.h>

enum class MusicPlace : uint8_t { Home, Farm, Sports };
enum class MusicLight : uint8_t { Day, Twilight, Night };
enum class MusicWeather : uint8_t { Fair, Wet, Snow };
enum class MusicVoice : uint8_t { Felt, Pluck, Bell };

struct MusicContext {
  MusicPlace place;
  MusicLight light;
  MusicWeather weather;
  uint8_t region;
};

struct MusicStep {
  uint8_t lead, harmony, bass; // MIDI note numbers, zero is a rest
  uint16_t milliseconds;
  MusicVoice voice;
};

// Original eight-bar themes. Region shifts the key, daylight changes the pulse
// and harmony, and weather changes the instrument and voicing.
inline MusicStep musicStep(MusicContext context, uint8_t step) {
  static constexpr int8_t motifs[][8] = {
    {7, 9, 12, 11, 7, 4, 2, -1},
    {0, 4, 7, 9, 7, 4, 2, -1},
    {0, 7, 12, 7, 9, 12, 14, 12},
  };
  static constexpr int8_t roots[] = {0, 5, 9, 7, 0, 4, 5, 7};
  static constexpr uint8_t keys[] = {0, 2, 5, 0, 3, 5, 2, 3, 0};
  const uint8_t bar = (step / 8) % 8;
  const uint8_t beat = step % 8;
  const int key = 48 + keys[context.region % 9];
  const bool dark = context.light == MusicLight::Night;
  const bool twilight = context.light == MusicLight::Twilight;
  const bool wet = context.weather == MusicWeather::Wet;
  const bool snow = context.weather == MusicWeather::Snow;
  const int root = key + roots[bar];
  const int third = bar == 2 || bar == 5 || (dark && (bar == 0 || bar == 4)) ? 3 : 4;
  int tune = motifs[static_cast<uint8_t>(context.place)][beat];
  if (dark && tune == 4) tune = 3;
  if (bar == 7 && beat == 6) tune = 0;
  const uint16_t pulse = (context.place == MusicPlace::Sports ? 265 : 345)
                        + (dark ? 95 : twilight ? 40 : 0) + (wet ? 40 : 0);
  return {
    static_cast<uint8_t>(tune < 0 ? 0 : root + 12 + tune),
    static_cast<uint8_t>(root + (beat % 4 == 1 ? third : beat % 4 == 3 ? 12 : 7)),
    static_cast<uint8_t>(beat % 4 == 0 ? root - 12 : 0),
    pulse,
    snow ? MusicVoice::Bell : wet || context.place == MusicPlace::Farm
                             ? MusicVoice::Pluck : MusicVoice::Felt
  };
}
