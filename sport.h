#pragma once
#include <stdint.h>
#include <math.h>
#include "dex.h"

// Torneo (sports.ino): las pruebas se juegan solas, sin tocar. Cuenta lo que es cada
// pokemon: los stats de su especie, sus genes y lo entrenado, su etapa de evolucion, su
// tipo (cada prueba favorece a unos: nadar, agua) y si esta megaevolucionado. Los rivales son dos especies al azar con genes y entrenamiento
// al azar, y en cada prueba hay algo de suerte: no siempre gana el mas fuerte.
// Aparte del sketch para poder probarlo en el host.
enum : uint8_t {
  EV_SPRINT = 0, EV_HURDLES, EV_SWIM, EV_LONGJUMP, EV_HIGHJUMP,  // las del torneo
  EV_LIFT, EV_CLIMB, EV_DIVE, EV_SKATE, EV_TORCH, EV_TRAIL,       // y las que se añaden en los gimnasios
  EV_COUNT
};

#define HU_N 5  // vallas en la pista
static inline float hurdlePos(int i) { return 0.15f + i * 0.16f; }  // 0.15 .. 0.79

struct Athlete {
  int16_t dex;
  uint8_t geneAtk, geneDef, geneSpe;  // 90..110 (% del stat de la especie)
  uint8_t trAtk, trDef, trSpe;        // 0..100 entrenado
  bool mega;
};

// 1 = primera forma, 2 = evoluciona de otra, 3 = de una que ya evoluciono. Los
// legendarios cuentan como forma final.
static inline uint8_t evoStage(int16_t dex) {
  if (dex < 1 || dex > DEX_COUNT) return 1;
  if (DEX_TBL[dex].rarity == R_LEGENDARIO) return 3;
  uint8_t stage = 1;
  for (int vuelta = 0; vuelta < 3; vuelta++) {  // hacia atras, de forma en forma
    int16_t prev = 0;
    for (int16_t p = 1; p <= DEX_COUNT && !prev; p++)
      if (DEX_TBL[p].evolvesTo == dex) prev = p;
    for (int i = 0; i < NUM_DEX_BRANCHES && !prev; i++)
      if (DEX_BRANCHES[i].to == dex) prev = DEX_BRANCHES[i].from;
    if (!prev) break;
    stage++;
    dex = prev;
  }
  return stage > 3 ? 3 : stage;
}

// atleta de una especie, con genes normales y sin entrenar
static inline Athlete athleteOf(int16_t dex) {
  Athlete a = { dex, 100, 100, 100, 0, 0, 0, false };
  return a;
}

// rival: especie dada, genes y entrenamiento al azar (u* en [0,1))
static inline Athlete rivalAthlete(int16_t dex, float uAtk, float uDef, float uSpe, float uTr) {
  Athlete a = athleteOf(dex);
  a.geneAtk = (uint8_t)(90 + uAtk * 21);
  a.geneDef = (uint8_t)(90 + uDef * 21);
  a.geneSpe = (uint8_t)(90 + uSpe * 21);
  a.trAtk = a.trDef = a.trSpe = (uint8_t)(uTr * 101);
  return a;
}

// tipo (el primero) de cada especie: dex.h no lo guarda, pero su color de acento es uno
// por tipo (comprobado con las 1025)
enum : uint8_t {
  TY_NORMAL = 0, TY_FIRE, TY_WATER, TY_GRASS, TY_ELECTRIC, TY_ICE, TY_FIGHTING, TY_POISON, TY_GROUND,
  TY_FLYING, TY_PSYCHIC, TY_BUG, TY_ROCK, TY_GHOST, TY_DRAGON, TY_DARK, TY_STEEL, TY_FAIRY, TY_COUNT
};
static inline uint8_t typeOf(int16_t dex) {
  if (dex < 1 || dex > DEX_COUNT) return TY_NORMAL;
  switch (DEX_TBL[dex].accent) {
    case 0xEA87: return TY_FIRE;
    case 0x4C98: return TY_WATER;
    case 0x3C49: return TY_GRASS;
    case 0xBCA1: return TY_ELECTRIC;
    case 0x4DB8: return TY_ICE;
    case 0xA2A5: return TY_FIGHTING;
    case 0x8A73: return TY_POISON;
    case 0xB447: return TY_GROUND;
    case 0x7C78: return TY_FLYING;
    case 0xD28F: return TY_PSYCHIC;
    case 0x7CC4: return TY_BUG;
    case 0x9407: return TY_ROCK;
    case 0x6AD3: return TY_GHOST;
    case 0x5A98: return TY_DRAGON;
    case 0x5A6E: return TY_DARK;
    case 0x7C73: return TY_STEEL;
    case 0xD435: return TY_FAIRY;
    default: return TY_NORMAL;
  }
}

// color de cada tipo (el acento de dex.h): medallas y etiquetas
static const uint16_t TYPE_COL[TY_COUNT] = { 0x8C4D, 0xEA87, 0x4C98, 0x3C49, 0xBCA1, 0x4DB8, 0xA2A5, 0x8A73, 0xB447,
                                             0x7C78, 0xD28F, 0x7CC4, 0x9407, 0x6AD3, 0x5A98, 0x5A6E, 0x7C73, 0xD435 };

// cada prueba favorece a unos tipos (nadar: agua) y a otros les cuesta (nadar: fuego, roca).
// Con las de los gimnasios, cada tipo tiene al menos una prueba donde va con ventaja
struct TypeMod { uint8_t ev, type; float mul; };
static const TypeMod TYPE_MODS[] = {
  { 0, TY_ELECTRIC, 1.15f }, { 0, TY_FIGHTING, 1.10f }, { 0, TY_NORMAL, 1.05f },   // 100 m
  { 0, TY_ROCK, 0.90f }, { 0, TY_STEEL, 0.90f },
  { 1, TY_FIGHTING, 1.15f }, { 1, TY_BUG, 1.10f }, { 1, TY_GRASS, 1.05f },         // vallas
  { 1, TY_ROCK, 0.90f }, { 1, TY_STEEL, 0.90f },
  { 2, TY_WATER, 1.25f }, { 2, TY_ICE, 1.10f }, { 2, TY_DRAGON, 1.05f },           // natacion
  { 2, TY_FIRE, 0.85f }, { 2, TY_ROCK, 0.85f }, { 2, TY_GROUND, 0.90f }, { 2, TY_STEEL, 0.90f },
  { 3, TY_FIGHTING, 1.15f }, { 3, TY_FLYING, 1.15f }, { 3, TY_BUG, 1.10f },        // longitud
  { 3, TY_NORMAL, 1.05f }, { 3, TY_ROCK, 0.90f }, { 3, TY_STEEL, 0.90f },
  { 4, TY_FLYING, 1.25f }, { 4, TY_PSYCHIC, 1.15f }, { 4, TY_FIGHTING, 1.10f },   // altura
  { 4, TY_FAIRY, 1.05f }, { 4, TY_ROCK, 0.85f }, { 4, TY_STEEL, 0.85f }, { 4, TY_GROUND, 0.90f },
  { 5, TY_FIGHTING, 1.20f }, { 5, TY_STEEL, 1.15f }, { 5, TY_ROCK, 1.15f }, { 5, TY_GROUND, 1.10f },  // pesas
  { 5, TY_FLYING, 0.90f }, { 5, TY_FAIRY, 0.90f }, { 5, TY_BUG, 0.90f },
  { 6, TY_ROCK, 1.20f }, { 6, TY_GROUND, 1.15f }, { 6, TY_BUG, 1.10f }, { 6, TY_GRASS, 1.05f },      // escalada
  { 6, TY_WATER, 0.90f }, { 6, TY_STEEL, 0.90f },
  { 7, TY_WATER, 1.20f }, { 7, TY_PSYCHIC, 1.10f }, { 7, TY_FLYING, 1.10f }, { 7, TY_FAIRY, 1.10f }, // saltos
  { 7, TY_FIRE, 0.85f }, { 7, TY_ROCK, 0.85f }, { 7, TY_STEEL, 0.90f },                             // al agua
  { 8, TY_ICE, 1.25f }, { 8, TY_FAIRY, 1.10f }, { 8, TY_PSYCHIC, 1.05f },                           // patinaje
  { 8, TY_FIRE, 0.85f }, { 8, TY_GRASS, 0.90f },
  { 9, TY_FIRE, 1.25f }, { 9, TY_DRAGON, 1.10f }, { 9, TY_NORMAL, 1.05f },                          // antorcha
  { 9, TY_WATER, 0.90f }, { 9, TY_ICE, 0.90f },
  { 10, TY_DARK, 1.15f }, { 10, TY_GHOST, 1.15f }, { 10, TY_POISON, 1.10f }, { 10, TY_GRASS, 1.10f }, // campo a
  { 10, TY_BUG, 1.05f }, { 10, TY_STEEL, 0.90f }, { 10, TY_ROCK, 0.90f },                             // traves
};
static inline float typeBonus(uint8_t ev, uint8_t type) {
  for (unsigned i = 0; i < sizeof(TYPE_MODS) / sizeof(TYPE_MODS[0]); i++)
    if (TYPE_MODS[i].ev == ev && TYPE_MODS[i].type == type) return TYPE_MODS[i].mul;
  return 1.0f;
}

// que mira cada prueba: peso de PS, ataque, defensa y velocidad
static const float EV_W[EV_COUNT][4] = {
  { 0.00f, 0.25f, 0.00f, 0.75f },  // 100 m: sobre todo velocidad
  { 0.00f, 0.25f, 0.20f, 0.55f },  // vallas: velocidad, impulso y aplomo
  { 0.50f, 0.00f, 0.00f, 0.50f },  // natacion: velocidad y aguante (PS)
  { 0.00f, 0.50f, 0.00f, 0.50f },  // longitud: fuerza y velocidad
  { 0.00f, 0.65f, 0.00f, 0.35f },  // altura: sobre todo fuerza
  { 0.10f, 0.60f, 0.30f, 0.00f },  // pesas: fuerza y aplomo
  { 0.00f, 0.40f, 0.20f, 0.40f },  // escalada: fuerza y agilidad
  { 0.00f, 0.30f, 0.30f, 0.40f },  // saltos al agua: tecnica
  { 0.00f, 0.20f, 0.00f, 0.80f },  // patinaje de velocidad
  { 0.50f, 0.00f, 0.10f, 0.40f },  // carrera de la antorcha: aguante
  { 0.20f, 0.30f, 0.20f, 0.30f },  // campo a traves: de todo un poco
};

// capacidad para una prueba (un buen atleta ronda 100)
static inline float athleteRating(const Athlete &a, uint8_t ev) {
  const DexEntry &d = companionEntry(a.dex);
  const float *w = EV_W[ev < EV_COUNT ? ev : 0];
  float base = w[0] * d.bHp + w[1] * d.bAtk * a.geneAtk / 100.0f + w[2] * d.bDef * a.geneDef / 100.0f +
               w[3] * d.bSpe * a.geneSpe / 100.0f;
  static const float ETAPA[4] = { 0.9f, 0.9f, 1.0f, 1.1f };
  float entreno = 1.0f + 0.12f * (a.trAtk + a.trDef + a.trSpe) / 300.0f;  // hasta +12%
  return base * ETAPA[evoStage(a.dex)] * entreno * (a.mega ? 1.15f : 1.0f) * typeBonus(ev, typeOf(a.dex));
}

// rendimiento del dia: la capacidad con +-15% de suerte (u en [0,1))
static inline float performance(float rating, float u) { return rating * (0.85f + 0.30f * u); }

// carreras y escalada: menos tiempo; saltos, pesas y saltos al agua: mas
static inline bool eventLowerWins(uint8_t ev) {
  return ev <= EV_SWIM || ev == EV_CLIMB || ev == EV_SKATE || ev == EV_TORCH || ev == EV_TRAIL;
}
// que se cuenta: segundos, metros, kilos, puntos o minutos y segundos
enum : uint8_t { UNIT_SECS = 0, UNIT_METERS, UNIT_KG, UNIT_POINTS, UNIT_MINSEC };
static inline uint8_t eventUnit(uint8_t ev) {
  if (ev == EV_LONGJUMP || ev == EV_HIGHJUMP) return UNIT_METERS;
  if (ev == EV_LIFT) return UNIT_KG;
  if (ev == EV_DIVE) return UNIT_POINTS;
  if (ev == EV_TORCH || ev == EV_TRAIL) return UNIT_MINSEC;
  return UNIT_SECS;
}

// marca de un rendimiento: segundos (carreras) o metros (saltos)
static inline float eventMark(uint8_t ev, float perf) {
  if (perf < 20) perf = 20;
  float m;
  switch (ev) {
    case EV_SPRINT: m = 7.0f + 500.0f / perf; return m > 25 ? 25 : m;
    case EV_HURDLES: m = 9.0f + 600.0f / perf; return m > 30 ? 30 : m;
    case EV_SWIM: m = 13.0f + 700.0f / perf; return m > 45 ? 45 : m;
    case EV_LONGJUMP: m = 2.0f + 4.6f * perf / 100.0f; return m < 2.5f ? 2.5f : (m > 9.0f ? 9.0f : m);
    case EV_LIFT:  // de 5 en 5 kg
      m = 40.0f + 1.2f * perf;
      m = floorf(m / 5.0f) * 5.0f;
      return m < 30 ? 30 : (m > 300 ? 300 : m);
    case EV_CLIMB: m = 6.0f + 900.0f / perf; return m > 60 ? 60 : m;     // muro de 15 m
    case EV_DIVE: m = 20.0f + 0.6f * perf; return m > 100 ? 100 : m;     // puntos de los jueces
    case EV_SKATE: m = 30.0f + 1200.0f / perf; return m > 90 ? 90 : m;  // 500 m
    case EV_TORCH: m = 480.0f + 30000.0f / perf; return m > 1800 ? 1800 : m;  // 3000 m
    case EV_TRAIL: m = 120.0f + 9000.0f / perf; return m > 900 ? 900 : m;     // 1 km con obstaculos
    default:  // altura: el liston va de 5 en 5 cm
      m = 0.7f + perf / 100.0f;
      m = floorf(m * 20.0f + 0.001f) / 20.0f;
      return m < 0.8f ? 0.8f : (m > 2.45f ? 2.45f : m);
  }
}

// puesto (1..3) del jugador: carreras, menos es mejor; saltos, mas es mejor. Los empates
// son para el jugador
static inline uint8_t playerPlace(float me, float a, float b, bool lowerWins) {
  uint8_t place = 1;
  if (lowerWins) {
    if (a < me) place++;
    if (b < me) place++;
  } else {
    if (a > me) place++;
    if (b > me) place++;
  }
  return place;
}
