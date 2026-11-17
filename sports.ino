// Torneo y pruebas de los gimnasios: carreras, saltos, pesas, escalada... nada de peleas.
// Se juegan solas: cuenta lo que es cada uno (sport.h: stats de la especie, genes,
// entrenamiento, etapa de evolucion, mega, tipo) y un poco de suerte.
//  - torneo: el compañero contra dos pokemon al azar en la prueba que elijas
//  - gimnasio (gyms.ino): uno contra uno, una prueba de la serie; al acabar, gymRoundDone()
// Cada prueba tiene su sitio (pista, piscina, foso, muro, tarima, pista de hielo...).
// Va a ~10 fps (lo que da volcar la pantalla entera): todo se mueve con el tiempo real.
// (pestaña de TamaPoke.ino: Arduino la une al final del principal)

enum : uint8_t { CH_MENU = 0, CH_READY, CH_COUNT, CH_PLAY, CH_DONE };
static uint8_t chStage = CH_MENU, chEv = EV_SPRINT;
static bool chGym = false;          // prueba de gimnasio (uno contra uno), no el torneo
static uint32_t chT0 = 0;           // inicio de la fase (millis)
static float chTime = 0;            // segundos de juego (las carreras largas van mas deprisa)
static int16_t rivalDex[2] = { -1, -1 };
static uint8_t nAth = 3;            // atletas: 0 = el del jugador
static PmdMon *athPmd[3];
static const char *athName[3];
static float marks[3];              // segundos, metros, kilos, puntos (segun la prueba)
static uint8_t chPlace = 0;
static bool chStone = false, chRoundWon = false;
static bool chPractice = false;     // consola SPORT: sin premio ni cansancio
static char chTitle[48];            // gimnasio: quien y que ronda

// como se juega en pantalla
static bool isRace() { return chEv == EV_SPRINT || chEv == EV_HURDLES || chEv == EV_SWIM || chEv == EV_SKATE ||
                              chEv == EV_TORCH || chEv == EV_TRAIL; }
static bool isClimb() { return chEv == EV_CLIMB; }
static bool byTurns() { return !isRace() && !isClimb(); }  // longitud, altura, pesas, saltos al agua

#define LJ_PX_M 22          // pixeles por metro en el foso
#define LJ_BOARD_X 230      // la tabla
#define HJ_GROUND 300
static float turnSecs() {
  if (chEv == EV_LONGJUMP) return 3.2f;
  return 2.6f;  // altura, pesas, saltos al agua
}
// por turnos: primero los rivales y el jugador el ultimo
static uint8_t turnAthlete(int k) {
  if (nAth == 2) return k == 0 ? 1 : 0;
  static const uint8_t T3[3] = { 1, 2, 0 };
  return T3[k < 3 ? k : 2];
}

static float urand() { return random(10000) / 10000.0f; }

// el compañero como atleta: su especie, genes, entrenamiento y si esta mega
Athlete petAthlete() {
  Athlete a = athleteOf(pet.speciesId);
  a.geneAtk = pet.geneAtk;
  a.geneDef = pet.geneDef;
  a.geneSpe = pet.geneSpe;
  a.trAtk = pet.trAtk;
  a.trDef = pet.trDef;
  a.trSpe = pet.trSpe;
  a.mega = pet.isMega();
  return a;
}

// dos rivales al azar de toda la pokedex (con sprite en la SD), distintos entre si y del jugador
static void pickRivals() {
  for (int i = 0; i < 2; i++) {
    rivalPmd[i].unload();
    rivalDex[i] = -1;
    for (int tries = 0; tries < 12 && !rivalPmd[i].loaded; tries++) {
      int16_t d = 1 + random(DEX_COUNT);
      if (d == pet.speciesId || (i == 1 && d == rivalDex[0]) || DEX_TBL[d].rarity == R_NONE) continue;
      if (rivalPmd[i].load(d, false)) rivalDex[i] = d;
    }
    if (rivalDex[i] < 0) rivalDex[i] = 1 + random(151);  // sin SD: al menos un nombre
  }
}

void openSports() {
  chOpen = true;
  chGym = false;
  chPractice = false;
  chStage = CH_MENU;
}

void sportsPractice(uint8_t ev) {  // consola: directo a una prueba del torneo, sin tocar la partida
  chOpen = true;
  chGym = false;
  chPractice = true;
  startEvent(ev);
}

static void closeSports() {
  chOpen = false;
  chStage = CH_MENU;
  for (int i = 0; i < 2; i++) rivalPmd[i].unload();
}

// torneo: se sortean los rivales y se decide ya como le va a cada uno; luego solo se ve
static void startEvent(uint8_t ev) {
  chEv = ev;
  chGym = false;
  pickRivals();
  Athlete a[3] = { petAthlete(), rivalAthlete(rivalDex[0], urand(), urand(), urand(), urand()),
                   rivalAthlete(rivalDex[1], urand(), urand(), urand(), urand()) };
  nAth = 3;
  athPmd[0] = &pmd;
  athPmd[1] = &rivalPmd[0];
  athPmd[2] = &rivalPmd[1];
  athName[0] = T(S_YOU);
  athName[1] = dexName(rivalDex[0]);
  athName[2] = dexName(rivalDex[1]);
  float r[3];
  for (int i = 0; i < 3; i++) {
    r[i] = athleteRating(a[i], ev);
    marks[i] = eventMark(ev, performance(r[i], urand()));
  }
  Serial.printf("sports ev=%u yo=%d %.0f rivales=%d %.0f, %d %.0f\n", ev, pet.speciesId, r[0], rivalDex[0], r[1],
                rivalDex[1], r[2]);
  chTime = 0;
  chStage = CH_READY;
}

// gimnasio: una prueba uno contra uno (los sprites ya cargados por gyms.ino)
void sportsRound(uint8_t ev, PmdMon *mine, const char *mineName, const Athlete &a, PmdMon *theirs,
                 const char *theirName, const Athlete &b, const char *title) {
  chOpen = true;
  chGym = true;
  chPractice = false;
  chEv = ev;
  nAth = 2;
  athPmd[0] = mine;
  athPmd[1] = theirs;
  athName[0] = mineName;
  athName[1] = theirName;
  marks[0] = eventMark(ev, performance(athleteRating(a, ev), urand()));
  marks[1] = eventMark(ev, performance(athleteRating(b, ev), urand()));
  strncpy(chTitle, title, sizeof(chTitle) - 1);
  chTitle[sizeof(chTitle) - 1] = 0;
  chTime = 0;
  chStage = CH_READY;
}

static void finishEvent() {
  chStage = CH_DONE;
  chT0 = millis();  // el resultado no se cierra con un toque que aun llega
  if (chGym) {
    chRoundWon = wonRound(chEv, marks[0], marks[1]);
    sfxPlay(chRoundWon ? SFX_MEDAL : SFX_TAP);
    Serial.printf("sports ronda ev=%u yo=%.2f el=%.2f %s\n", chEv, marks[0], marks[1], chRoundWon ? "gana" : "pierde");
    return;
  }
  chPlace = playerPlace(marks[0], marks[1], marks[2], eventLowerWins(chEv));
  bool won = chPlace == 1;
  if (!chPractice) pet.sportsDone(won);
  chStone = won && !chPractice && pet.grantSportsStone();
  sfxPlay(won ? SFX_MEDAL : SFX_TAP);
  Serial.printf("sports fin ev=%u puesto=%u yo=%.2f rivales=%.2f %.2f piedra=%d\n", chEv, chPlace, marks[0], marks[1],
                marks[2], chStone);
}

static float slowest() {
  float m = marks[0];
  for (int i = 1; i < nAth; i++) if (marks[i] > m) m = marks[i];
  return m;
}
// las carreras largas se ven aceleradas: como mucho ~12 s de pantalla
static float raceRate() { return slowest() > 12 ? slowest() / 12 : 1; }
static float playEnd() {
  if (!byTurns()) return slowest() + 1.0f;
  return nAth * turnSecs() + 0.4f;
}

// ---------- los sitios ----------

static void drawSky(int y1, uint16_t col) { gfx->fillRect(0, 0, 466, y1, col); }
#define SKY C565(0x9c, 0xd4, 0xf4)
#define GRASS C565(0x5a, 0xb0, 0x50)

// gradas con publico de colores
static void drawStands(int y0, int y1) {
  static const uint16_t gente[5] = { C565(0xe8, 0x50, 0x3a), C565(0xf8, 0xd0, 0x40), C565(0x4f, 0x93, 0xc4),
                                     C565(0xff, 0xff, 0xff), C565(0x58, 0xb8, 0x68) };
  gfx->fillRect(0, y0, 466, y1 - y0, C565(0x7a, 0x80, 0x8e));
  for (int r = 0, y = y0 + 5; y < y1 - 6; r++, y += 11)
    for (int c = 0, x = (r % 2) * 6; x < 466; c++, x += 12)
      gfx->fillRect(x, y, 6, 6, gente[(c * 3 + r * 7) % 5]);
}

#define TRACK_X0 50
#define TRACK_X1 416
#define LANE_Y0 150
#define LANE_H 60
static int trackX(float pos) { return TRACK_X0 + (int)(pos * (TRACK_X1 - TRACK_X0)); }
static int laneTop(int lane) { return LANE_Y0 + lane * LANE_H; }
// obstaculos de las vallas y del campo a traves
static float obstaclePos(int i) { return hurdlePos(i); }

static void drawLanes(uint16_t suelo, uint16_t linea) {
  gfx->fillRect(0, LANE_Y0, 466, 3 * LANE_H, suelo);
  for (int i = 0; i <= 3; i++) gfx->fillRect(0, LANE_Y0 + i * LANE_H - 1, 466, 3, linea);
  gfx->fillRect(TRACK_X0 - 2, LANE_Y0, 3, 3 * LANE_H, linea);  // salida
  for (int y = LANE_Y0; y < LANE_Y0 + 3 * LANE_H; y += 6)     // meta a cuadros
    for (int k = 0; k < 2; k++)
      gfx->fillRect(TRACK_X1 + k * 6, y, 6, 6, ((y / 6 + k) % 2) ? RGB565_BLACK : UI_WHITE);
}

static void drawTrack() {
  drawSky(70, SKY);
  drawStands(70, 135);
  gfx->fillRect(0, 135, 466, 15, GRASS);
  drawLanes(C565(0xc8, 0x56, 0x40), UI_WHITE);
  gfx->fillRect(0, LANE_Y0 + 3 * LANE_H + 2, 466, 466 - (LANE_Y0 + 3 * LANE_H + 2), GRASS);
  if (chEv != EV_HURDLES) return;
  for (int lane = 0; lane < 3; lane++)
    for (int i = 0; i < HU_N; i++) {
      int x = trackX(obstaclePos(i)), y = laneTop(lane) + 22;
      gfx->fillRect(x, y, 3, 30, UI_INK);
      gfx->fillRect(x - 8, y, 18, 6, UI_WHITE);
      gfx->fillRect(x - 2, y, 6, 6, C565(0xe8, 0x50, 0x3a));
    }
}

static void drawPool() {
  drawSky(70, SKY);
  drawStands(70, 125);
  gfx->fillRect(0, 125, 466, 15, C565(0xee, 0xf2, 0xf4));  // borde de baldosas
  uint16_t agua = C565(0x2e, 0x8c, 0xd8);
  gfx->fillRect(0, LANE_Y0 - 10, 466, 3 * LANE_H + 10, agua);
  int fase = (int)(chTime * 30) % 24;
  for (int y = LANE_Y0; y < LANE_Y0 + 3 * LANE_H; y += 14)  // olas
    for (int x = ((y / 14) % 2) * 12 - fase; x < 466; x += 24)
      gfx->fillRect(x, y, 10, 2, C565(0x6c, 0xb8, 0xf0));
  for (int i = 1; i < 3; i++)  // corcheras
    for (int x = 0; x < 466; x += 10)
      gfx->fillCircle(x + 4, laneTop(i), 3, (x / 10) % 3 == 0 ? C565(0xe8, 0x50, 0x3a) : UI_WHITE);
  for (int i = 0; i < 3; i++) gfx->fillRect(18, laneTop(i) + 18, 20, 16, C565(0xaa, 0xb0, 0xbc));  // poyetes
  gfx->fillRect(TRACK_X1 + 6, LANE_Y0 - 10, 8, 3 * LANE_H + 10, UI_WHITE);  // la pared de llegada
  gfx->fillRect(0, LANE_Y0 + 3 * LANE_H, 466, 466 - (LANE_Y0 + 3 * LANE_H), C565(0xee, 0xf2, 0xf4));
}

// patinaje: pista de hielo con su valla y las lineas rojas y azules
static void drawRink() {
  drawSky(70, C565(0x30, 0x3a, 0x5a));  // pabellon cubierto
  drawStands(70, 130);
  gfx->fillRect(0, 130, 466, 14, UI_WHITE);  // valla
  for (int x = 0; x < 466; x += 60) gfx->fillRect(x, 133, 30, 8, C565(0x4f, 0x93, 0xc4));
  drawLanes(C565(0xe4, 0xf2, 0xfa), C565(0x9c, 0xc4, 0xe4));
  gfx->fillRect(CX - 2, LANE_Y0, 4, 3 * LANE_H, C565(0xe8, 0x50, 0x3a));  // linea roja central
  gfx->fillRect(0, LANE_Y0 + 3 * LANE_H + 2, 466, 466 - (LANE_Y0 + 3 * LANE_H + 2), C565(0xe4, 0xf2, 0xfa));
}

// antorcha: calles de la ciudad
static void drawRoad() {
  drawSky(150, C565(0xf8, 0xc0, 0x80));  // atardecer
  static const int16_t EDIF[8][2] = { { 0, 60 }, { 50, 90 }, { 95, 45 }, { 150, 100 }, { 215, 70 }, { 270, 110 },
                                      { 340, 55 }, { 395, 85 } };
  for (int i = 0; i < 8; i++) {  // edificios con ventanas
    int x = EDIF[i][0], h = EDIF[i][1];
    gfx->fillRect(x, 150 - h, 52, h, C565(0x6a, 0x5a, 0x7a));
    for (int y = 150 - h + 8; y < 144; y += 14)
      for (int wx = x + 8; wx < x + 46; wx += 14) gfx->fillRect(wx, y, 6, 6, C565(0xf8, 0xe0, 0x90));
  }
  drawLanes(C565(0x5a, 0x5e, 0x68), C565(0xf8, 0xf8, 0xf8));
  gfx->fillRect(0, LANE_Y0 + 3 * LANE_H + 2, 466, 466 - (LANE_Y0 + 3 * LANE_H + 2), C565(0xa0, 0xa4, 0xac));
}

// campo a traves: sendero de tierra en el bosque al anochecer, troncos que saltar
static void drawTrail() {
  drawSky(150, C565(0x3a, 0x3a, 0x6a));
  for (int x = -10; x < 466; x += 46) {  // arboles
    gfx->fillTriangle(x, 150, x + 46, 150, x + 23, 70 + (x * 7) % 30, C565(0x2a, 0x6a, 0x4a));
    gfx->fillRect(x + 20, 140, 6, 12, C565(0x6a, 0x48, 0x30));
  }
  drawLanes(C565(0x9a, 0x70, 0x48), C565(0x7a, 0x58, 0x38));
  gfx->fillRect(0, LANE_Y0 + 3 * LANE_H + 2, 466, 466 - (LANE_Y0 + 3 * LANE_H + 2), C565(0x2a, 0x6a, 0x4a));
  for (int lane = 0; lane < 3; lane++)
    for (int i = 0; i < HU_N; i++) {  // troncos
      int x = trackX(obstaclePos(i)), y = laneTop(lane) + 40;
      gfx->fillRoundRect(x - 12, y, 26, 12, 5, C565(0x7a, 0x50, 0x2a));
      gfx->fillCircle(x + 12, y + 6, 5, C565(0xc8, 0x98, 0x60));
    }
}

static const uint16_t FLAG_COL[3] = { 0xEA87, 0x4C98, 0x9B9A };  // jugador, rival 1, rival 2

static void drawLongJumpField(int hechos) {
  drawSky(80, SKY);
  drawStands(80, 160);
  gfx->fillRect(0, 160, 466, 306, GRASS);
  gfx->fillRect(20, 268, LJ_BOARD_X - 20, 14, C565(0xc0, 0x60, 0x48));  // pasillo
  gfx->fillRect(LJ_BOARD_X - 4, 266, 8, 18, UI_WHITE);                  // tabla
  gfx->fillRect(LJ_BOARD_X + 12, 262, 214, 24, C565(0xe8, 0xcc, 0x8c));  // foso
  gfx->setTextColor(UI_INK);
  setSize(1);
  for (int m = 3; m <= 9; m++) {
    int x = LJ_BOARD_X + m * LJ_PX_M;
    gfx->fillRect(x, 286, 2, 6, UI_INK);
    char n[4];
    snprintf(n, sizeof(n), "%d", m);
    setCur(x - 2, 296);
    printT(n);
  }
  for (int k = 0; k < hechos; k++) {  // banderas de los que ya saltaron
    int i = turnAthlete(k), x = LJ_BOARD_X + (int)(marks[i] * LJ_PX_M);
    gfx->fillRect(x, 238, 2, 26, UI_INK);
    gfx->fillTriangle(x + 2, 238, x + 2, 250, x + 14, 244, FLAG_COL[i]);
  }
}

static int hjBarY(float h) { return HJ_GROUND - (int)((h - 0.5f) * 110); }

static void drawHighJumpField(float bar) {
  drawSky(80, SKY);
  drawStands(80, 150);
  gfx->fillRect(0, 150, 466, 316, GRASS);
  gfx->fillRect(270, HJ_GROUND - 16, 150, 22, C565(0x2f, 0x6c, 0xd8));  // colchoneta
  int by = hjBarY(bar);
  gfx->fillRect(292, by - 4, 4, HJ_GROUND - by + 4, UI_INK);  // postes
  gfx->fillRect(368, by - 4, 4, HJ_GROUND - by + 4, UI_INK);
  gfx->fillRect(292, by, 80, 4, C565(0xf8, 0xd0, 0x40));
}

// pesas: tarima de madera con focos
static void drawLiftStage() {
  drawSky(150, C565(0x2a, 0x24, 0x3a));  // sala a oscuras
  drawStands(90, 150);
  gfx->fillTriangle(120, 0, 90, 150, 150, 150, C565(0x4a, 0x44, 0x5a));  // focos
  gfx->fillTriangle(346, 0, 316, 150, 376, 150, C565(0x4a, 0x44, 0x5a));
  gfx->fillRect(0, 150, 466, 316, C565(0x3a, 0x34, 0x4a));
  gfx->fillRect(110, 290, 246, 22, C565(0xc0, 0x90, 0x58));  // tarima
  gfx->fillRect(110, 312, 246, 6, C565(0x8a, 0x60, 0x38));
}

// la barra: discos rojos a los lados, a la altura y
static void drawBarbell(int cx, int y) {
  gfx->fillRect(cx - 70, y - 2, 140, 5, C565(0xc8, 0xcc, 0xd4));
  for (int s = -1; s <= 1; s += 2) {
    gfx->fillRect(cx + s * 56 - 5, y - 18, 10, 36, C565(0xe8, 0x50, 0x3a));
    gfx->fillRect(cx + s * 66 - 4, y - 13, 8, 26, C565(0x30, 0x30, 0x38));
  }
}

// saltos al agua: piscina y trampolin a la izquierda
#define DIVE_TOP 150
#define DIVE_WATER 290
static void drawDivePool() {
  drawSky(80, SKY);
  drawStands(80, 140);
  gfx->fillRect(0, 140, 466, DIVE_WATER - 140, C565(0xee, 0xf2, 0xf4));
  gfx->fillRect(60, DIVE_TOP, 12, DIVE_WATER - DIVE_TOP, C565(0xaa, 0xb0, 0xbc));  // torre
  gfx->fillRect(60, DIVE_TOP, 110, 8, C565(0x4f, 0x93, 0xc4));                     // plataforma
  gfx->fillRect(0, DIVE_WATER, 466, 466 - DIVE_WATER, C565(0x2e, 0x8c, 0xd8));
  int fase = (int)(chTime * 30) % 24;
  for (int y = DIVE_WATER + 10; y < 400; y += 14)
    for (int x = ((y / 14) % 2) * 12 - fase; x < 466; x += 24) gfx->fillRect(x, y, 10, 2, C565(0x6c, 0xb8, 0xf0));
}

// escalada: muro con presas de colores, una via por atleta
#define WALL_TOP 110
#define WALL_BOT 340
static int climbX(int i) {
  if (nAth == 2) return i == 0 ? 290 : 176;
  static const int X3[3] = { 233, 120, 346 };
  return X3[i];
}
static void drawWall() {
  drawSky(466, C565(0x8a, 0xb8, 0xd8));
  gfx->fillRect(60, 60, 346, 320, C565(0x9a, 0x84, 0x6a));  // el muro
  static const uint16_t PRESA[4] = { C565(0xe8, 0x50, 0x3a), C565(0xf8, 0xd0, 0x40), C565(0x58, 0xb8, 0x68),
                                     C565(0x4f, 0x93, 0xc4) };
  for (int k = 0; k < 40; k++) {
    int x = 70 + (k * 67) % 326, y = 70 + (k * 41) % 300;
    gfx->fillCircle(x, y, 5, PRESA[k % 4]);
  }
  for (int i = 0; i < nAth; i++) gfx->drawFastVLine(climbX(i), 60, WALL_BOT - 60, UI_WHITE);  // cuerdas
  gfx->fillRect(0, WALL_BOT + 10, 466, 466 - WALL_BOT - 10, GRASS);
}

// ---------- los atletas ----------

static void drawAthlete(int i, bool moving, int x, int ground, uint8_t maxS) {
  PmdMon &m = *athPmd[i];
  if (!m.loaded) {  // sin SD: un circulo
    gfx->fillCircle(x, ground - 18, 16, i == 0 ? UI_BAR_WARN : UI_INK);
    return;
  }
  uint8_t act = moving && m.has(PMD_WALKR) ? PMD_WALKR : PMD_IDLE;
  drawPmdActM(m, act, x, ground, (uint32_t)(chTime * 1500) + i * 137, true, false, maxS);
}

static float racePos(int i) {
  if (chStage != CH_PLAY && chStage != CH_DONE) return 0;
  float p = chTime / marks[i];
  return p > 1 ? 1 : p;
}

// carril de cada atleta: el jugador en medio
static int laneOf(int i) { return i == 0 ? 1 : (i == 1 ? 0 : 2); }

static void drawRacers() {
  for (int lane = 0; lane < 3; lane++) {  // de atras adelante
    int i = -1;
    for (int k = 0; k < nAth; k++) if (laneOf(k) == lane) i = k;
    if (i < 0) continue;
    float p = racePos(i);
    int x = trackX(p), g = laneTop(lane) + 54;
    bool mueve = chStage == CH_PLAY && p < 1;
    if ((chEv == EV_HURDLES || chEv == EV_TRAIL) && mueve)  // salta vallas y troncos
      for (int h = 0; h < HU_N; h++) {
        float d = (p - obstaclePos(h)) / 0.05f;
        if (d > -1 && d < 1) g -= (int)(cosf(d * 1.5708f) * 30);
      }
    if (chEv == EV_SWIM) {
      g = laneTop(lane) + 40;
      drawAthlete(i, mueve, x, g, 2);
      gfx->fillRect(x - 46, laneTop(lane) + 28, 92, LANE_H - 30, C565(0x2e, 0x8c, 0xd8));  // en el agua
      if (mueve)
        for (int k = 0; k < 4; k++) gfx->fillCircle(x - 34 - k * 7, laneTop(lane) + 26 + (k % 2) * 6, 3, UI_WHITE);
      continue;
    }
    drawAthlete(i, mueve, x, g, 2);
    if (chEv == EV_TORCH) {  // la antorcha encendida
      gfx->fillRect(x + 22, g - 46, 4, 18, C565(0xc0, 0xa0, 0x60));
      gfx->fillCircle(x + 24, g - 50, 6, C565(0xf8, 0x90, 0x30));
      gfx->fillCircle(x + 24, g - 53, 3, C565(0xf8, 0xe0, 0x60));
    }
    if (chEv == EV_SKATE && mueve)  // rastro en el hielo
      gfx->drawFastHLine(x - 50, g - 2, 30, C565(0x9c, 0xc4, 0xe4));
  }
}

// carreras: el numero de cada calle a la izquierda y, abajo, quien va en cada una (el
// jugador en rojo, como en los resultados): en la pista no se distinguia quien era quien
static void drawRaceLegend() {
  int n = 0;
  for (int lane = 0; lane < 3; lane++) {
    for (int k = 0; k < nAth; k++) {
      if (laneOf(k) != lane) continue;
      char c[4];
      snprintf(c, sizeof(c), "%d", lane + 1);
      gfx->setTextColor(UI_WHITE);
      setSize(2);
      setCur(20, laneTop(lane) + 22);
      printT(c);
      n++;
    }
  }
  int y0 = 340, h = n * 24 + 12;
  gfx->fillRoundRect(CX - 120, y0, 240, h, 12, UI_BG_DAY);
  gfx->drawRoundRect(CX - 120, y0, 240, h, 12, UI_INK);
  int row = 0;
  for (int lane = 0; lane < 3; lane++)
    for (int k = 0; k < nAth; k++) {
      if (laneOf(k) != lane) continue;
      char l[48];
      snprintf(l, sizeof(l), "%d  %s", lane + 1, athName[k]);
      gfx->setTextColor(k == 0 ? UI_BAR_BAD : UI_INK);
      setSize(2);
      setCur(CX - 100, y0 + 8 + row * 24);
      printT(l);
      row++;
    }
}

static void drawClimbers() {
  for (int i = 0; i < nAth; i++) {
    float p = racePos(i);
    int y = WALL_BOT - (int)(p * (WALL_BOT - WALL_TOP - 40));
    drawAthlete(i, chStage == CH_PLAY && p < 1, climbX(i), y, 2);
  }
  // la campana de arriba
  gfx->fillRect(60, WALL_TOP - 30, 346, 4, C565(0xf8, 0xd0, 0x40));
}

// saltos y demas por turnos: turno k y segundos dentro del turno
static void turnNow(int &k, float &t) {
  k = (int)(chTime / turnSecs());
  t = chTime - k * turnSecs();
  if (k > nAth - 1) { k = nAth - 1; t = turnSecs(); }
}

static void drawTurns() {
  int k;
  float t;
  turnNow(k, t);
  if (chStage == CH_READY || chStage == CH_COUNT) k = 0, t = 0;
  int i = turnAthlete(k);
  float m = marks[i];
  if (chEv == EV_LONGJUMP) {
    drawLongJumpField(t >= 2.1f ? k + 1 : k);
    if (t < 1.2f) {  // carrerilla
      drawAthlete(i, chStage == CH_PLAY, 40 + (int)(t / 1.2f * (LJ_BOARD_X - 40)), 268, 3);
    } else {         // vuelo y caida en el foso
      float p = t < 2.1f ? (t - 1.2f) / 0.9f : 1.0f;
      drawAthlete(i, p < 1, LJ_BOARD_X + (int)(p * m * LJ_PX_M), 268 - (int)(sinf(3.1416f * p) * (20 + m * 8)), 3);
    }
  } else if (chEv == EV_HIGHJUMP) {
    drawHighJumpField(m);
    int x = 170, y = HJ_GROUND;
    if (t >= 0.5f) {  // corre y pasa por encima del liston
      float q = t < 1.4f ? (t - 0.5f) / 0.9f : 1.0f;
      x = 200 + (int)(q * 170);
      y = HJ_GROUND - (int)(sinf(3.1416f * q) * (HJ_GROUND - hjBarY(m) + 14));
    } else {
      x = 170 + (int)(t / 0.5f * 30);
    }
    drawAthlete(i, chStage == CH_PLAY && t < 1.4f, x, y, 3);
  } else if (chEv == EV_LIFT) {
    drawLiftStage();
    gPmdTopY = 292 - 36;  // sin sprite: el circulo
    drawAthlete(i, false, CX, 292, 3);
    // la barra sube del suelo a justo encima de la cabeza
    float q = t < 0.6f ? 0 : (t < 1.5f ? (t - 0.6f) / 0.9f : 1.0f);
    int top = gPmdTopY - 16;
    if (top < 160) top = 160;
    drawBarbell(CX, 280 - (int)(q * (280 - top)));
  } else {  // saltos al agua: de la plataforma al agua, con salpicadura
    drawDivePool();
    if (t < 0.6f) {
      drawAthlete(i, false, 130, DIVE_TOP, 3);
    } else if (t < 1.5f) {
      float q = (t - 0.6f) / 0.9f;
      int x = 130 + (int)(q * 150), y = DIVE_TOP - (int)(sinf(3.1416f * q * 0.6f) * 50) + (int)(q * q * (DIVE_WATER - DIVE_TOP + 30));
      drawAthlete(i, true, x, y, 3);
    } else {  // al agua: salpicadura mas pequeña cuanto mejor (mas puntos)
      int r = 30 - (int)(m / 5);
      if (r < 6) r = 6;
      for (int s = 0; s < 6; s++) gfx->fillCircle(280 - r + s * r * 2 / 5, DIVE_WATER - 8 - (s % 3) * 8, 4, UI_WHITE);
    }
  }
}

// ---------- pantallas ----------

static void label(const char *t, int y, int size, uint16_t col) {
  gfx->setTextColor(col);
  setSize(size);
  setCur(centerX(t, size), y);
  printT(t);
}

// cartel claro para que se lea sobre el sitio
static void sportsBox(int y, int h) {
  gfx->fillRoundRect(CX - 170, y, 340, h, 16, UI_BG_DAY);
  gfx->drawRoundRect(CX - 170, y, 340, h, 16, UI_INK);
}
// marcador de arriba: estrecho, que arriba la pantalla redonda corta los lados
static void hudBox() {
  gfx->fillRoundRect(CX - 110, 24, 220, 40, 14, UI_BG_DAY);
  gfx->drawRoundRect(CX - 110, 24, 220, 40, 14, UI_INK);
}

// una marca con su unidad
void fmtMark(char *b, size_t n, uint8_t ev, float v) {
  unsigned c;
  switch (eventUnit(ev)) {
    case UNIT_METERS:
      c = (unsigned)(v * 100 + 0.5f);
      snprintf(b, n, T(S_METERS_FMT), c / 100, c % 100);
      break;
    case UNIT_KG: snprintf(b, n, T(S_KG_FMT), (unsigned)v); break;
    case UNIT_POINTS: snprintf(b, n, T(S_PTS_FMT), (unsigned)v, (unsigned)(v * 10) % 10); break;
    case UNIT_MINSEC: snprintf(b, n, T(S_MINSEC_FMT), (unsigned)v / 60, (unsigned)v % 60); break;
    default: snprintf(b, n, T(S_SECS_FMT), (unsigned)v, (unsigned)(v * 10) % 10);
  }
}

// menu del torneo: las 11 pruebas en dos columnas
#define MENU_Y0 82
#define MENU_H 46
static void renderSportsMenu() {
  drawPageBackground();
  label(T(S_HUB_SPORTS), 36, 3, UI_INK);
  for (int i = 0; i < EV_COUNT; i++) {
    int col = i % 2, row = i / 2;
    int x = col ? CX + 4 : CX - 154, y = MENU_Y0 + row * MENU_H;
    if (i == EV_COUNT - 1 && EV_COUNT % 2) x = CX - 75;  // la ultima sola, en medio
    gfx->fillRoundRect(x, y, 150, 40, 12, EV_COL[i]);
    gfx->setTextColor(UI_WHITE);
    setSize(2);
    setCur(x + (150 - textW(T(EV_NAME[i]), 2)) / 2, y + 12);
    printT(T(EV_NAME[i]));
  }
  label(T(S_SWIPE_R_BACK), 372, 2, UI_HINT);
}
static int menuHit(int16_t x, int16_t y) {
  if (y < MENU_Y0) return -1;
  int row = (y - MENU_Y0) / MENU_H;
  if (row > (EV_COUNT - 1) / 2) return -1;
  int i = row * 2 + (x >= CX ? 1 : 0);
  if (EV_COUNT % 2 && row == (EV_COUNT - 1) / 2) i = EV_COUNT - 1;
  return i < EV_COUNT ? i : -1;
}

// marcador de arriba mientras se juega: el tiempo, o quien va y su marca
static void drawHud() {
  char a[24];
  hudBox();
  if (!byTurns()) {
    float t = chTime;
    if (racePos(0) >= 1) t = marks[0];  // el jugador ya llego: su tiempo
    fmtMark(a, sizeof(a), chEv, t);
    label(a, 36, 2, UI_INK);
    return;
  }
  int k;
  float t;
  turnNow(k, t);
  int i = turnAthlete(k);
  label(athName[i], 36, 2, i == 0 ? UI_BAR_BAD : UI_INK);
  if (t >= turnSecs() - 1.1f) {  // ya acabo su turno: su marca
    fmtMark(a, sizeof(a), chEv, marks[i]);
    sportsBox(186, 50);
    label(a, 200, 3, UI_INK);
  }
}

// orden de los atletas por marca (los empates, para el jugador)
static void ranking(int orden[3]) {
  for (int i = 0; i < 3; i++) orden[i] = i;
  bool menos = eventLowerWins(chEv);
  for (int a = 0; a < nAth; a++)
    for (int b = a + 1; b < nAth; b++) {
      float x = marks[orden[a]], y = marks[orden[b]];
      bool mejor = menos ? y < x || (y == x && orden[b] == 0) : y > x || (y == x && orden[b] == 0);
      if (mejor) { int t = orden[a]; orden[a] = orden[b]; orden[b] = t; }
    }
}

// torneo: podio (oro en medio, plata a la izquierda, bronce a la derecha)
static void renderPodium() {
  drawPageBackground();
  static const StrId MEDAL[3] = { S_GOLD, S_SILVER, S_BRONZE };
  static const uint16_t MEDAL_COL[3] = { 0xE5CC, 0xAD97, 0xB3C8 };
  static const uint16_t MEDAL_INK[3] = { 0xBC23, 0x7BF1, 0x9A86 };  // titulo: los mismos, mas oscuros (sobre el fondo claro)
  label(T(MEDAL[chPlace - 1]), 40, 3, MEDAL_INK[chPlace - 1]);
  int orden[3];
  ranking(orden);
  static const int PX[3] = { CX, CX - 100, CX + 100 }, PH[3] = { 70, 48, 32 };
  for (int k = 0; k < 3; k++) {
    gfx->fillRect(PX[k] - 48, 300 - PH[k], 96, PH[k], MEDAL_COL[k]);
    gfx->drawRect(PX[k] - 48, 300 - PH[k], 96, PH[k], UI_INK);
    char n[4];
    snprintf(n, sizeof(n), "%d", k + 1);
    gfx->setTextColor(UI_INK);
    setSize(2);
    setCur(PX[k] - textW(n, 2) / 2, 300 - PH[k] + 8);  // el puesto, en su cajon
    printT(n);
    drawAthlete(orden[k], false, PX[k], 300 - PH[k], 2);
  }
  for (int k = 0; k < 3; k++) {  // marcas de los tres
    int i = orden[k];
    char v[24], line[56];
    fmtMark(v, sizeof(v), chEv, marks[i]);
    snprintf(line, sizeof(line), "%d. %s  %s", k + 1, athName[i], v);
    label(line, 318 + k * 22, 2, i == 0 ? UI_BAR_BAD : UI_INK);
  }
  if (chPlace == 1 && !chPractice)  // la piedra del dia (el entrenamiento de la consola no da)
    label(T(chStone ? S_GOT_STONE : S_STONE_TODAY), 92, 2, chStone ? UI_BAR_OK : UI_HINT);
}

// gimnasio: quien gano esta ronda (y gyms.ino pinta el marcador de la serie encima)
static void renderRound() {
  drawPageBackground();
  label(chTitle, 40, 2, UI_INK);
  label(T(chRoundWon ? S_ROUND_WON : S_ROUND_LOST), 76, 3, chRoundWon ? UI_BAR_OK : UI_BAR_BAD);
  for (int i = 0; i < 2; i++) {
    int x = i == 0 ? CX - 90 : CX + 90;
    drawAthlete(i, false, x, 250, 2);
    char v[24];
    fmtMark(v, sizeof(v), chEv, marks[i]);
    gfx->setTextColor(i == 0 ? UI_BAR_BAD : UI_INK);
    setSize(2);
    setCur(x - textW(athName[i], 2) / 2, 262);
    printT(athName[i]);
    setCur(x - textW(v, 2) / 2, 288);
    printT(v);
  }
  gymScoreLine(330, chRoundWon);  // gyms.ino: 1 : 0
  label(T(S_TAP_NEXT), 372, 2, UI_HINT);
}

void renderSports() {
  uint32_t now = millis();
  if (chStage == CH_MENU) {
    renderSportsMenu();
    drawToast();
    gfx->flush();
    return;
  }
  if (chStage == CH_COUNT || chStage == CH_PLAY) wakeScreen(now);  // mirando no se atenua
  if (chStage == CH_COUNT && now - chT0 >= 2100) {
    chStage = CH_PLAY;
    chT0 = now;
  }
  if (chStage == CH_PLAY) {
    chTime = (now - chT0) / 1000.0f * (byTurns() ? 1.0f : raceRate());
    if (chTime >= playEnd()) finishEvent();
  }
  if (chStage == CH_DONE) {
    if (chGym) renderRound();
    else renderPodium();
    gfx->flush();
    return;
  }
  gfx->fillScreen(RGB565_BLACK);
  if (chEv == EV_SWIM) drawPool();
  else if (chEv == EV_SKATE) drawRink();
  else if (chEv == EV_TORCH) drawRoad();
  else if (chEv == EV_TRAIL) drawTrail();
  else if (isRace()) drawTrack();
  if (isRace()) {
    drawRacers();
    if (chStage != CH_READY) drawRaceLegend();
  } else if (isClimb()) { drawWall(); drawClimbers(); }
  else drawTurns();
  if (chStage == CH_READY) {
    char vs[96];
    sportsBox(136, 182);
    label(T(EV_NAME[chEv]), 150, 3, UI_INK);
    label(T(EV_HOW[chEv]), 190, 2, UI_INK);
    label(T(EV_BONUS[chEv]), 216, 2, UI_INK);
    if (chGym) snprintf(vs, sizeof(vs), "%s  vs  %s", athName[0], athName[1]);
    else snprintf(vs, sizeof(vs), T(S_VS_FMT), athName[1], athName[2]);
    label(chGym ? chTitle : vs, 242, 2, UI_INK);
    if (chGym) label(vs, 268, 2, UI_INK);
    else {
      float tb = typeBonus(chEv, typeOf(pet.speciesId));  // lo que le toca al compañero por su tipo
      if (tb != 1.0f) {
        char yo[24];
        snprintf(yo, sizeof(yo), T(tb > 1 ? S_BONUS_UP_FMT : S_BONUS_DOWN_FMT), (unsigned)(fabsf(tb - 1) * 100 + 0.5f));
        label(yo, 268, 2, tb > 1 ? UI_BAR_OK : UI_BAR_BAD);
      }
    }
    label(T(S_TAP_START), 294, 2, UI_BAR_OK);
  } else if (chStage == CH_COUNT) {
    uint32_t e = now - chT0;
    char n[16];
    if (e < 1800) snprintf(n, sizeof(n), "%lu", (unsigned long)(3 - e / 600));
    else strcpy(n, T(S_GO));
    sportsBox(176, 70);
    label(n, 192, 4, UI_INK);
  } else {
    drawHud();
  }
  gfx->flush();
}

void sportsTap(int16_t x, int16_t y) {
  switch (chStage) {
    case CH_MENU: {
      int i = menuHit(x, y);
      if (i >= 0) {
        sfxPlay(SFX_TAP);
        startEvent(i);
      }
      break;
    }
    case CH_READY:
      chStage = CH_COUNT;
      chT0 = millis();
      break;
    case CH_DONE:  // siguiente (no con un toque nada mas salir el resultado)
      if (millis() - chT0 < 1500) break;
      if (chGym) {
        chOpen = false;
        gymRoundDone(chRoundWon);  // gyms.ino: la siguiente prueba o el final
        break;
      }
      for (int i = 0; i < 2; i++) rivalPmd[i].unload();
      chStage = CH_MENU;
      break;
    default:  // se juega solo: mirando no hace falta tocar
      break;
  }
}

void sportsSwipe(int dir) {
  if (dir <= 0 || chGym) return;  // en un gimnasio no se sale a medias
  if (chStage == CH_COUNT || chStage == CH_PLAY) return;  // mientras se juega no se sale sin querer
  if (chStage == CH_MENU) closeSports();  // a actividades
  else {
    for (int i = 0; i < 2; i++) rivalPmd[i].unload();
    chStage = CH_MENU;
  }
}

#if defined(TAMAPOKE_SMALLTV_PRO)
void smallTvSportsTargets(SmallTvTargets &list) {
  list.context = 0x70000 + chStage;
  if (chStage == CH_MENU) {
    for (int i = 0; i < EV_COUNT; ++i) {
      const bool alone = i == EV_COUNT - 1 && EV_COUNT % 2;
      list.add(alone ? 136 : 54 + i % 2 * 184, MENU_Y0 + i / 2 * MENU_H, 174, MENU_H - 2);
    }
  } else if (chStage == CH_READY || chStage == CH_DONE) {
    list.add(100, 347, 266, 66);
  }
}
#endif
