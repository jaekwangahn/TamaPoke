#pragma once
#include <stdint.h>

enum class MusicPlace : uint8_t { Home, Farm, Sports };
enum class MusicLight : uint8_t { Day, Twilight, Night };
enum class MusicWeather : uint8_t { Fair, Wet, Snow };

struct MusicContext {
  MusicPlace place;
  MusicLight light;
  MusicWeather weather;
  uint8_t region;
};

struct MusicStep {
  uint8_t lead, harmony, bass; // MIDI note numbers, zero is a rest
  uint16_t milliseconds;
};

constexpr uint8_t MUSIC_BAR_STEPS = 6;
constexpr uint8_t MUSIC_STEPS = 16 * MUSIC_BAR_STEPS;

// Original music-box waltz. Six eighth notes make one bar of 3/4.
inline MusicStep musicStep(MusicContext context, uint8_t step) {
  static constexpr int8_t melody[][MUSIC_BAR_STEPS] = {
    { 9, -1, 12, 14, 16, -1}, {15, 12, 11, -1,  7, -1},
    { 5,  9, 12, -1, 17, 16}, {14, -1, 12,  9,  7, -1},
    { 4,  7, 11, 14, 11,  7}, { 5, -1,  9, 12, 16, -1},
    {14, 11,  8, -1, 11, 14}, { 9, -1, -1, 12,  9, -1},
    {12, -1, 16, 19, 17, 16}, {14, 12,  9, -1,  5, -1},
    { 7, 11, 14, -1, 17, 14}, {16, -1, 12,  7,  4, -1},
    { 5,  9, 14, 17, 16, 14}, {12,  9,  5, -1,  2, -1},
    { 4,  8, 11, 14, 11,  8}, { 9, -1, 12, -1,  9, -1},
  };
  // Triads rooted on A minor, C, F, D minor, G, and E, voiced under the chimes.
  struct Chord { uint8_t root, third, fifth; };
  static constexpr Chord chords[] = {
    {9, 12, 16}, {0, 4, 7}, {5, 9, 12}, {2, 5, 9}, {7, 11, 14}, {4, 8, 11},
  };
  static constexpr uint8_t progression[] = {0, 1, 2, 3, 4, 2, 5, 0, 1, 2, 4, 1, 3, 2, 5, 0};
  static constexpr uint8_t keys[] = {0, 2, 5, 0, 3, 5, 2, 3, 0};
  const uint8_t bar = (step % MUSIC_STEPS) / MUSIC_BAR_STEPS;
  const uint8_t beat = step % MUSIC_BAR_STEPS;
  const auto chord = chords[progression[bar]];
  const int key = 48 + keys[context.region % 9];
  const bool night = context.light == MusicLight::Night;
  const bool wet = context.weather == MusicWeather::Wet;
  const int pitch = melody[bar][beat];
  const int octave = context.place == MusicPlace::Farm && beat == 2 ? 12 : 24;
  return {
    static_cast<uint8_t>(pitch < 0 ? 0 : key + octave + pitch),
    static_cast<uint8_t>(beat == 2 ? key + 12 + chord.third : beat == 4 ? key + 12 + chord.fifth : 0),
    static_cast<uint8_t>(beat == 0 ? key + chord.root : 0),
    static_cast<uint16_t>((context.place == MusicPlace::Sports ? 245 : 315) +
                          (night ? 75 : context.light == MusicLight::Twilight ? 30 : 0) + (wet ? 25 : 0))
  };
}
