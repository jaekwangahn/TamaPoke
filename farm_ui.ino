// Pantallas de la granja y del album de recuerdos.
// (pestaña de TamaPoke.ino: Arduino la une al final del principal)

// ---------- granja: compañeros guardados en la SD ----------

// aviso breve arriba (lo usan la pantalla principal y la granja)
void drawToast() {
  if (!toastMsg || !timeLeft(toastUntil)) return;
  setSize(2);
  int w = textW(toastMsg, 2) + 32;
  gfx->fillRoundRect(CX - w / 2, 92, w, 40, 12, UI_INK);
  gfx->setTextColor(UI_WHITE);
  setCur(CX - w / 2 + 16, 104);
  printT(toastMsg);
}

// Confirmar antes de gastar puntos (tienda, ocio): que (l1) y cuanto (l2), SI / NO. Como el
// de restaurar la copia. askTap: 1 = SI, 0 = NO o fuera (se cancela)
void drawAskBox(const char *l1, const char *l2) {
  gfx->fillRoundRect(64, 160, 338, 160, 16, UI_WHITE);
  gfx->drawRoundRect(64, 160, 338, 160, 16, UI_INK);
  gfx->setTextColor(UI_INK);
  setSize(2);
  setCur(centerX(l1, 2), 184);
  printT(l1);
  setCur(centerX(l2, 2), 214);
  printT(l2);
  gfx->fillRoundRect(118, 252, 100, 52, 12, UI_BAR_OK);
  gfx->fillRoundRect(248, 252, 100, 52, 12, UI_BAR_BAD);
  gfx->setTextColor(UI_WHITE);
  setCur(118 + (100 - textW(T(S_YES), 2)) / 2, 270);
  printT(T(S_YES));
  setCur(248 + (100 - textW(T(S_NO), 2)) / 2, 270);
  printT(T(S_NO));
}
int askTap(int16_t x, int16_t y) { return (x >= 118 && x <= 218 && y >= 252 && y <= 304) ? 1 : 0; }

// RECOGER: guarda al compañero tal cual en la granja y se despide hacia alla
void collectCurrent() {
  if (pet.isEgg() || pet.ceremony) return;
  if (!farmReady()) { toast(T(S_NEED_SD)); return; }
  PetRec r;
  pet.exportRec(r);
  if (farmPut(pet, r)) pet.startCollect();
}

// ---------- actividades: la pagina a la izquierda de la principal ----------
// cuatro filas: granja, torneo, gimnasios y ocio
#define HUB_ROW_X 63
#define HUB_ROW_W 340
#define HUB_ROW_H 66
#define HUB_ROW_Y0 88
#define HUB_ROW_STEP 74
enum { HUB_FARM, HUB_SPORTS, HUB_GYMS, HUB_FUN, HUB_ROWS };

void openHub() {
  if (pet.ceremony) return;
  hubOpen = true;
}

// por que no se puede competir ahora (nullptr = si se puede)
const char *sportsBlocked() {
  if (pet.vacant || pet.isEgg()) return T(S_NO_PARTNER);
  if (pet.sleeping) return T(S_ASLEEP);
  if (pet.sick) return T(S_SICK);
  if (pet.evolving()) return T(S_EVOLVING);
  if (pet.questAway()) return T(S_AWAY);
  if (pet.energy < 15) return T(S_TOO_TIRED);
  return nullptr;
}

static void hubRow(int row, uint16_t fill, const char *const *icon, bool on, const char *title, const char *sub) {
  int y = HUB_ROW_Y0 + row * HUB_ROW_STEP;
  gfx->fillRoundRect(HUB_ROW_X, y, HUB_ROW_W, HUB_ROW_H, 18, on ? fill : UI_TRACK);
  gfx->drawRoundRect(HUB_ROW_X, y, HUB_ROW_W, HUB_ROW_H, 18, UI_INK);
  drawMap(icon, 16, HUB_ROW_X + 14, y + 9, 3, !on);
  gfx->setTextColor(UI_INK);
  setSize(3);
  setCur(HUB_ROW_X + 76, y + 8);
  printT(title);
  setSize(2);
  setCur(HUB_ROW_X + 76, y + 42);
  printT(sub);
}

void renderHub() {
  gfx->fillScreen(RGB565_BLACK);
  gfx->fillCircle(CX, CY, 231, UI_BG_DAY);
  gfx->setTextColor(UI_INK);
  setSize(3);
  setCur(centerX(T(S_HUB_TITLE), 3), 50);
  printT(T(S_HUB_TITLE));
  char n[24];
  snprintf(n, sizeof(n), T(S_FARM_SUB_FMT), (unsigned)farmCount());
  hubRow(HUB_FARM, 0xB6F2, SPR_ICON_FARM, true, T(S_HUB_FARM), farmReady() ? n : T(S_NEED_SD));
  const char *why = sportsBlocked();
  hubRow(HUB_SPORTS, 0xFF2E, SPR_ICON_TROPHY, !why, T(S_HUB_SPORTS), why ? why : T(S_HUB_SPORTS_HINT));
  char b[32];
  snprintf(b, sizeof(b), T(S_BADGES_TOTAL_FMT), (unsigned)pet.totalBadges());
  hubRow(HUB_GYMS, 0xAEFF, SPR_ICON_GYM, true, T(S_GYMS), b);  // la lista sirve tambien de vitrina
  hubRow(HUB_FUN, C565(0xf8, 0xd0, 0xc8), SPR_ICON_FUN, true, T(S_HUB_FUN), T(S_HUB_FUN_HINT));
  gfx->setTextColor(UI_HINT);
  setSize(2);
  setCur(centerX(T(S_SWIPE_R_BACK), 2), 392);
  printT(T(S_SWIPE_R_BACK));
  drawToast();
  gfx->flush();
}

void hubTap(int16_t x, int16_t y) {
  if (x < HUB_ROW_X || x >= HUB_ROW_X + HUB_ROW_W || y < HUB_ROW_Y0) return;
  int row = (y - HUB_ROW_Y0) / HUB_ROW_STEP;
  if (row >= HUB_ROWS || y - HUB_ROW_Y0 - row * HUB_ROW_STEP > HUB_ROW_H) return;  // entre filas
  if (row == HUB_FARM) {
    sfxPlay(SFX_TAP);
    openFarm();
  } else if (row == HUB_SPORTS) {
    const char *why = sportsBlocked();
    if (why) { toast(why); return; }
    sfxPlay(SFX_TAP);
    openSports();
  } else if (row == HUB_GYMS) {
    sfxPlay(SFX_TAP);
    openGyms();
  } else {
    sfxPlay(SFX_TAP);
    openFun();
  }
}

void hubSwipe(int dir) {
  if (dir > 0) hubOpen = false;  // a la derecha: vuelta a la principal
}

// Keep the animated sprites for one farm page in PSRAM.
#define FARM_PER_PAGE 9
struct FarmWalker {
  PmdMon sprite;
  float x = 0, y = 0;
  int destX = 0, destY = 0;
  uint32_t since = 0, duration = 0;
  uint8_t action = PMD_IDLE;
  bool moving = false;
};
static FarmWalker farmWalkers[FARM_PER_PAGE];
static uint8_t farmWalkerCount = 0;
static bool farmWalkersLoaded = false;
static uint32_t farmLastFrame = 0;

static void releaseFarmWalkers() {
  for (auto &walker : farmWalkers) walker.sprite.unload();
  farmWalkerCount = 0;
  farmWalkersLoaded = false;
}

void openFarm() {
  if (pet.ceremony) return;
  releaseFarmWalkers();
  farmOpen = true;
  farmPage = 0;
  farmDetail = -1;
  farmDirty = true;
}

void closeFarm() {
  releaseFarmWalkers();
  farmOpen = false;
  albumOpen = false;
  farmDetail = -1;
  galleryPmd.unload();
}

static int farmPages() { return farmCount() > 0 ? (farmCount() + FARM_PER_PAGE - 1) / FARM_PER_PAGE : 1; }

// sacar al elegido: el compañero actual entra en la granja en su lugar
void farmSwapIn(int idx) {
  if (pet.isEgg()) { toast(T(S_HATCH_FIRST)); return; }
  if (pet.ceremony || idx < 0 || idx >= farmCount()) return;
  int16_t dex = farmAt(idx).speciesId;
  PetRec cur, sale;
  pet.exportRec(cur);
  if (!farmTake(pet, dex, sale)) return;
  if (!farmPut(pet, cur)) {  // no se pudo guardar al actual: se deshace
    farmPut(pet, sale);
    toast(T(S_NEED_SD));
    return;
  }
  pet.importRec(sale);
  closeFarm();
}

void farmSwipe(int dir) {
  if (albumOpen) {
    albumSwipe(dir);
    return;
  }
  if (farmDetail >= 0) {  // en detalle: volver al prado
    farmDetail = -1;
    galleryPmd.unload();
    farmDirty = true;
    return;
  }
  int np = farmPage - dir;  // deslizar a la izquierda avanza pagina
  if (np < 0) { closeFarm(); return; }  // retroceder desde la primera = salir
  if (np > farmPages() - 1) np = farmPages() - 1;
  if (np != farmPage) {
    releaseFarmWalkers();
    farmPage = np;
    farmDirty = true;
  }
}

#define FARM_BTN_X 143
#define FARM_BTN_Y 352
#define FARM_BTN_W 180
#define FARM_BTN_H 48

#define ALBUM_BTN_X 173
#define ALBUM_BTN_Y 398  // mas abajo lo cortaba el circulo
#define ALBUM_BTN_W 120
#define ALBUM_BTN_H 32

// frutas para los de la granja: la misma regla que el compañero (FRUITS_PER_DAY al dia,
// en su ficha); alli no pasan hambre, asi que pasado el tope no se gasta
#define FF_Y 128
static PetRec farmRec;        // la ficha (SD) del que se esta viendo: tipo y capacidades
static bool farmRecOk = false;
static int fruitRowX(int k) { return CX - 96 + k * 64; }
static bool hasStatFruit() {
  for (int k = 0; k < 4; k++)
    if (pet.bag[ITEM_FRUIT_STA + k]) return true;
  return false;
}
static void feedFarmFruit(int idx, uint8_t item) {
  if (pet.bag[item] == 0) { toast(T(S_NONE_LEFT)); return; }
  PetRec r;
  if (!farmPeek(farmAt(idx).speciesId, r)) return;
  uint8_t hoy = (uint8_t)(pet.dayNow() & 0xFF);
  if (r.fruitDay8 != hoy) { r.fruitDay8 = hoy; r.fruitCnt = 0; }
  if (r.fruitCnt >= FRUITS_PER_DAY) { toast(T(S_FRUIT_FULL)); return; }
  if (!fruitRaise(item, r.trAtk, r.trDef, r.trSpe)) { toast(T(S_MAXED)); return; }
  r.fruitCnt++;
  if (!farmPut(pet, r)) return;
  farmRec = r;
  pet.bag[item]--;
  pet.markDirty();
  toast(T(fruitUpMsg(item)));
  sfxPlay(SFX_EAT);
}

static void loadFarmWalkers(uint32_t now) {
  if (farmWalkersLoaded) return;
  farmWalkersLoaded = true;
  farmLastFrame = now;
  const int first = farmPage * FARM_PER_PAGE;
  farmWalkerCount = min(FARM_PER_PAGE, max(0, farmCount() - first));
  static const int16_t starts[FARM_PER_PAGE][2] = {
    {130, 200}, {290, 156}, {284, 311}, {171, 297}, {345, 233},
    {108, 269}, {218, 213}, {337, 284}, {229, 358}
  };
  for (uint8_t i = 0; i < farmWalkerCount; ++i) {
    auto &walker = farmWalkers[i];
    const auto &entry = farmAt(first + i);
    // A single TPK2 can be up to 3 MiB. Leave room for other screens and Wi-Fi.
    if (ESP.getFreePsram() >= 4UL * 1024 * 1024)
      walker.sprite.load(entry.speciesId, entry.shiny);
    walker.x = starts[i][0];
    walker.y = starts[i][1];
    walker.since = now;
    walker.duration = 250 + i * 310;
    walker.action = PMD_IDLE;
    walker.moving = false;
  }
}

static void updateFarmWalker(uint8_t slot, uint32_t now, float dt) {
  auto &walker = farmWalkers[slot];
  if (walker.moving) {
    float dx = walker.destX - walker.x, dy = walker.destY - walker.y;
    float distance = sqrtf(dx * dx + dy * dy);
    float step = 0.032f * dt;
    if (distance > step) {
      walker.x += dx * step / distance;
      walker.y += dy * step / distance;
      return;
    }
    walker.x = walker.destX;
    walker.y = walker.destY;
    walker.moving = false;
    walker.since = now;
    walker.duration = random(850, 2350);
    const uint8_t actions[] = { PMD_IDLE, PMD_HOP, PMD_NOD, PMD_POSE, PMD_SIT, PMD_EAT };
    walker.action = actions[random(sizeof(actions))];
    if (!walker.sprite.has(walker.action)) walker.action = PMD_IDLE;
    return;
  }
  if (now - walker.since < walker.duration) return;
  // Project a nearby destination into the walkable ellipse.
  float x = (walker.x + random(-100, 101) - CX) / 133.0f;
  float y = (walker.y + random(-77, 78) - 252) / 100.0f;
  float radius = sqrtf(x * x + y * y);
  if (radius > 1) { x /= radius; y /= radius; }
  walker.destX = CX + 133 * x;
  walker.destY = 252 + 100 * y;
  walker.action = walker.destX < walker.x ? PMD_WALKL : PMD_WALKR;
  if (!walker.sprite.has(walker.action)) walker.action = PMD_IDLE;
  walker.moving = true;
  walker.since = now;
}

static void drawFarmMeadow() {
  gfx->fillScreen(RGB565_BLACK);
  gfx->fillCircle(CX, CY, 231, C565(187, 212, 159));
  gfx->fillEllipse(CX, 250, 177, 126, C565(207, 225, 172));
  gfx->fillEllipse(354, 155, 49, 28, C565(140, 172, 133));
  gfx->fillEllipse(351, 152, 43, 23, C565(101, 173, 180));
  gfx->drawFastHLine(332, 148, 18, C565(170, 221, 213));
  gfx->drawFastHLine(355, 157, 24, C565(170, 221, 213));
  for (int i = 0; i < 24; ++i) {
    const int x = 64 + (i * 107) % 336;
    const int y = 90 + (i * 63) % 283;
    if ((x - CX) * (x - CX) + (y - CY) * (y - CY) > 181 * 181) continue;
    gfx->drawFastVLine(x, y, 5, C565(136, 171, 115));
    gfx->drawPixel(x - 2, y + 1, C565(136, 171, 115));
    gfx->drawPixel(x + 2, y + 1, C565(136, 171, 115));
    if (i % 3 == 0) gfx->fillRect(x - 2, y - 3, 4, 3, C565(240, 172, 143));
  }
  const int16_t trees[][2] = { {78, 152}, {390, 265}, {96, 337} };
  for (const auto &tree : trees) {
    const int x = tree[0], y = tree[1];
    gfx->fillEllipse(x, y + 10, 19, 6, C565(154, 183, 131));
    gfx->fillRect(x - 4, y - 11, 8, 21, C565(137, 103, 76));
    gfx->fillRoundRect(x - 20, y - 34, 40, 28, 8, C565(88, 146, 108));
    gfx->fillRoundRect(x - 14, y - 44, 28, 29, 8, C565(113, 166, 113));
    gfx->fillRect(x - 8, y - 39, 12, 5, C565(153, 194, 124));
  }
}

static void drawFarmWalkers(uint32_t now) {
  uint8_t order[FARM_PER_PAGE];
  for (uint8_t i = 0; i < farmWalkerCount; ++i) {
    uint8_t j = i;
    while (j && farmWalkers[order[j - 1]].y > farmWalkers[i].y) {
      order[j] = order[j - 1];
      --j;
    }
    order[j] = i;
  }
  for (uint8_t i = 0; i < farmWalkerCount; ++i) {
    const auto slot = order[i];
    auto &walker = farmWalkers[slot];
    const auto &entry = farmAt(farmPage * FARM_PER_PAGE + slot);
    const int x = walker.x, y = walker.y;
    gfx->fillEllipse(x, y - 1, 20, 5, C565(158, 184, 129));
    if (walker.sprite.has(walker.action)) {
      drawPmdActM(walker.sprite, walker.action, x, y, now - walker.since,
                  walker.moving || walker.action == PMD_IDLE, false, 2);
    } else if (const uint8_t *thumb = thumbs.get(entry.speciesId)) {
      drawThumbIn(thumb, x - 24, y - 48, 1, false, 48);
    } else {
      gfx->fillCircle(x, y - 15, 12, DEX_TBL[entry.speciesId].accent);
    }
    if (entry.shiny) {
      gfx->drawFastVLine(x + 23, y - 40, 7, C_GOLD);
      gfx->drawFastHLine(x + 20, y - 37, 7, C_GOLD);
    }
  }
}

void farmTap(int16_t x, int16_t y) {
  if (albumOpen) {
    albumTap(x, y);
    return;
  }
  if (farmDetail < 0 && x >= ALBUM_BTN_X && x <= ALBUM_BTN_X + ALBUM_BTN_W && y >= ALBUM_BTN_Y - 6 && y <= ALBUM_BTN_Y + ALBUM_BTN_H) {
    releaseFarmWalkers();
    albumOpen = true;  // boton del album, abajo
    albumPage = 0;
    farmDirty = true;
    return;
  }
  if (farmDetail >= 0) {
    if (x >= FARM_BTN_X && x <= FARM_BTN_X + FARM_BTN_W && y >= FARM_BTN_Y && y <= FARM_BTN_Y + FARM_BTN_H) {
      farmSwapIn(farmDetail);
      return;
    }
    if (hasStatFruit() && abs(y - FF_Y) <= 26) {  // fila de frutas: darle una
      for (int k = 0; k < 4; k++)
        if (abs(x - fruitRowX(k)) <= 28) {
          feedFarmFruit(farmDetail, ITEM_FRUIT_STA + k);
          return;
        }
    }
    farmDetail = -1;  // fuera del boton: volver al prado
    galleryPmd.unload();
    farmDirty = true;
    return;
  }
  if (y < 70) { closeFarm(); return; }
  int idx = -1;
  float front = -1;
  for (uint8_t i = 0; i < farmWalkerCount; ++i) {
    const auto &walker = farmWalkers[i];
    if (fabsf(x - walker.x) <= 29 && y >= walker.y - 60 && y <= walker.y + 8 && walker.y > front) {
      idx = farmPage * FARM_PER_PAGE + i;
      front = walker.y;
    }
  }
  if (idx < 0 || idx >= farmCount()) return;
  releaseFarmWalkers();
  farmDetail = idx;
  galleryPmd.load(farmAt(idx).speciesId, farmAt(idx).shiny);
  farmRecOk = farmPeek(farmAt(idx).speciesId, farmRec);
}

void renderFarm() {
  if (albumOpen) {
    renderAlbum();
    return;
  }
  if (farmDetail >= 0 && farmDetail < farmCount()) {  // detalle: animado, se repinta siempre
    const FarmEntry &e = farmAt(farmDetail);
    const DexEntry &d = DEX_TBL[e.speciesId];
    gfx->fillScreen(RGB565_BLACK);
    gfx->fillCircle(CX, CY, 231, UI_BG_DAY);
    char head[40];
    snprintf(head, sizeof(head), "%s%s", e.shiny ? "*" : "", e.nick[0] ? e.nick : dexName(e.speciesId));
    gfx->setTextColor(d.accent);
    setSize(3);
    int ts = (textW(head, 3) <= 260) ? 3 : 2;
    setSize(ts);
    setCur(centerX(head, ts), 52);
    printT(head);
    // nivel y tipo en una linea; abajo, sus capacidades (lo que cuenta en gimnasios y torneo)
    char lv[16];
    snprintf(lv, sizeof(lv), T(S_LVL_FMT), farmLevel(e, pet.lastSeenEpoch));
    uint8_t ty = typeOf(e.speciesId);
    const char *tn = T((StrId)(S_TY_NORMAL + ty));
    setSize(2);
    int wl = textW(lv, 2), x0 = CX - (wl + 16 + textW(tn, 2)) / 2;
    gfx->setTextColor(UI_INK);
    setCur(x0, 92);
    printT(lv);
    gfx->setTextColor(TYPE_COL[ty]);
    setCur(x0 + wl + 16, 92);
    printT(tn);
    if (farmRecOk) {
      char tr[48];
      snprintf(tr, sizeof(tr), T(S_TRAIN_FMT), farmRec.trAtk, farmRec.trDef, farmRec.trSpe);
      gfx->setTextColor(UI_INK);
      setCur(centerX(tr, 2), 326);
      printT(tr);
    }
    if (galleryPmd.loaded) drawPmdActM(galleryPmd, PMD_IDLE, CX, 320, millis(), true, false, 5);
    if (hasStatFruit())  // las frutas de la mochila: tocar una se la da
      for (int k = 0; k < 4; k++) {
        uint8_t it = ITEM_FRUIT_STA + k;
        drawFruitIcon(it, fruitRowX(k) - 8, FF_Y, 13, pet.bag[it] == 0);
        char n[8];
        snprintf(n, sizeof(n), "x%u", pet.bag[it]);
        gfx->setTextColor(UI_INK);
        setSize(2);
        setCur(fruitRowX(k) + 8, FF_Y - 7);
        printT(n);
      }
    gfx->fillRoundRect(FARM_BTN_X, FARM_BTN_Y, FARM_BTN_W, FARM_BTN_H, 12, UI_BAR_OK);
    gfx->setTextColor(UI_WHITE);
    setSize(2);
    setCur(FARM_BTN_X + (FARM_BTN_W - textW(T(S_SWAP), 2)) / 2, FARM_BTN_Y + 16);
    printT(T(S_SWAP));
    drawToast();
    gfx->flush();
    return;
  }

  const uint32_t now = millis();
  loadFarmWalkers(now);
  const uint32_t elapsed = now - farmLastFrame;
  if (!farmDirty && elapsed < 66) return;
  farmDirty = false;
  farmLastFrame = now;
  for (uint8_t i = 0; i < farmWalkerCount; ++i)
    updateFarmWalker(i, now, min<uint32_t>(elapsed, 100));
  drawFarmMeadow();
  drawFarmWalkers(now);
  char head[24];
  snprintf(head, sizeof(head), T(S_FARM_FMT), (unsigned)farmCount());
  gfx->setTextColor(UI_INK);
  setSize(3);
  setCur(centerX(head, 3), 36);
  printT(head);
  if (!farmReady() || farmCount() == 0) {
    const char *m = farmReady() ? T(S_FARM_EMPTY) : T(S_NEED_SD);
    setSize(2);
    setCur(centerX(m, 2), CY - 8);
    printT(m);
  }
  if (farmPages() > 1) {  // pagina, a la izquierda del boton del album
    char pg[8];
    snprintf(pg, sizeof(pg), "%d/%d", farmPage + 1, farmPages());
    gfx->setTextColor(UI_INK);
    setSize(2);
    setCur(ALBUM_BTN_X - 12 - textW(pg, 2), ALBUM_BTN_Y + 8);
    printT(pg);
  }
  // boton del album de recuerdos
  gfx->fillRoundRect(ALBUM_BTN_X, ALBUM_BTN_Y, ALBUM_BTN_W, ALBUM_BTN_H, 10, 0x4C98);
  gfx->setTextColor(UI_WHITE);
  setSize(2);
  setCur(ALBUM_BTN_X + (ALBUM_BTN_W - textW(T(S_ALBUM), 2)) / 2, ALBUM_BTN_Y + 8);
  printT(T(S_ALBUM));
  drawToast();
  gfx->flush();
}

// ---------- album de recuerdos: una fila por compañero que acabo ----------

#define ALBUM_ROWS 4
#define ALBUM_X 70
#define ALBUM_Y 78
#define ALBUM_W 326
#define ALBUM_RH 76

static int albumPages() {
  int n = memoirCount();
  return n > 0 ? (n + ALBUM_ROWS - 1) / ALBUM_ROWS : 1;
}

static void albumDate(uint32_t e, char *out, size_t cap) {
  if (!e) { snprintf(out, cap, "?"); return; }
  time_t t = e;
  struct tm tm;
  gmtime_r(&t, &tm);
  snprintf(out, cap, "%d/%02d/%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

void renderAlbum() {
  bool avisando = toastMsg && timeLeft(toastUntil);
  if (!farmDirty && !avisando) return;
  farmDirty = avisando;
  gfx->fillScreen(RGB565_BLACK);
  gfx->fillCircle(CX, CY, 231, UI_BG_DAY);
  int n = memoirCount();
  char head[24];
  snprintf(head, sizeof(head), T(S_ALBUM_FMT), n);
  gfx->setTextColor(UI_INK);
  setSize(3);
  setCur(centerX(head, 3), 36);
  printT(head);
  if (n == 0) {
    const char *m = farmReady() ? T(S_ALBUM_EMPTY) : T(S_NEED_SD);
    setSize(2);
    setCur(centerX(m, 2), CY - 8);
    printT(m);
  }
  static const uint16_t END_COL[4] = { UI_BAR_OK, 0x8410, UI_BAR_BAD, 0x4C98 };
  for (int k = 0; k < ALBUM_ROWS; k++) {
    MemRec r;
    if (!memoirGet(albumPage * ALBUM_ROWS + k, r)) break;
    int y = ALBUM_Y + k * ALBUM_RH;
    gfx->fillRoundRect(ALBUM_X, y, ALBUM_W, ALBUM_RH - 6, 10, UI_WHITE);
    const uint8_t *t = thumbs.get(r.speciesId);
    if (t) drawThumb(t, ALBUM_X + 4, y + 3, 2, false);
    char l1[48];
    snprintf(l1, sizeof(l1), "%s%s Lv.%u", r.shiny ? "*" : "", r.nick[0] ? r.nick : dexName(r.speciesId), r.level);
    gfx->setTextColor(DEX_TBL[r.speciesId].accent);
    setSize(2);
    setCur(ALBUM_X + 76, y + 6);
    printT(l1);
    uint8_t e = (r.endType >= CER_FAREWELL && r.endType <= CER_COLLECT) ? r.endType : CER_FAREWELL;
    gfx->setTextColor(END_COL[e - CER_FAREWELL]);
    setCur(ALBUM_X + 76, y + 28);
    printT(T((StrId)(S_END_FAREWELL + e - CER_FAREWELL)));
    int medals = __builtin_popcount(r.medals & ~MED_RETIRED);
    if (medals) {  // estrella + numero de medallas
      drawMap(SPR_ICON_STAR, 16, ALBUM_X + ALBUM_W - 64, y + 26, 1, false);
      char m[8];
      snprintf(m, sizeof(m), "%d", medals);
      gfx->setTextColor(UI_INK);
      setCur(ALBUM_X + ALBUM_W - 44, y + 28);
      printT(m);
    }
    char d0[16], d1[16], l3[40];
    albumDate(r.bornEpoch, d0, sizeof(d0));
    albumDate(r.endEpoch, d1, sizeof(d1));
    snprintf(l3, sizeof(l3), "%s - %s", d0, d1);
    gfx->setTextColor(0x8410);
    setSize(1);
    setCur(ALBUM_X + 76, y + 52);
    printT(l3);
  }
  if (albumPages() > 1) {
    char pg[8];
    snprintf(pg, sizeof(pg), "%d/%d", albumPage + 1, albumPages());
    gfx->setTextColor(UI_INK);
    setSize(2);
    setCur(centerX(pg, 2), 396);
    printT(pg);
  }
  drawToast();
  gfx->flush();
}

void albumSwipe(int dir) {
  int np = albumPage - dir;  // izquierda avanza
  if (np < 0) {              // retroceder desde la primera = volver a la granja
    albumOpen = false;
    farmDirty = true;
    return;
  }
  if (np > albumPages() - 1) np = albumPages() - 1;
  if (np != albumPage) {
    albumPage = np;
    farmDirty = true;
  }
}

void albumTap(int16_t x, int16_t y) {
  if (y < 72) {  // la cabecera vuelve a la granja
    albumOpen = false;
    farmDirty = true;
  }
}
