// Galeria de la Pokedex: una portada por region (paisaje y nombre) y, al tocarla, sus pokemon.
// (pestaña de TamaPoke.ino: Arduino la une al final del principal)

// ---------- galeria pokedex ----------

#define THUMB_CELL 80
#if defined(TAMAPOKE_SMALLTV_PRO)
#define GAL_X (CX - 2 * GAL_CELL)
#define GAL_Y 68
#define GAL_CELL 98
#define GAL_ROW 82
#else
#define GAL_X 73
#define GAL_Y 84
#define GAL_CELL 80
#define GAL_ROW GAL_CELL
#endif

// dibuja una miniatura centrada en su celda; sil=true la pinta en tinta
void drawThumb(const uint8_t *b, int x, int y, int s, bool sil) { drawThumbIn(b, x, y, s, sil, THUMB_CELL); }

// centrada en una casilla de `cell` px
void drawThumbIn(const uint8_t *b, int x, int y, int s, bool sil, int cell) {
  uint8_t w = b[0], h = b[1], n = b[2];
  const uint8_t *pal = b + 3;
  const uint8_t *d = pal + n * 2;
  int ox = x + (cell - w * s) / 2;
  int oy = y + (cell - h * s) / 2;
  for (int r = 0; r < h; r++) {
    const uint8_t *row = d + r * w;
    forRuns(w, [&](int c) { return indexedKey(row[c], sil); },
            [&](int c0, int len, int k) {
              uint16_t col = sil ? INK_K : (uint16_t)(pal[k * 2] | (pal[k * 2 + 1] << 8));
              gfx->fillRect(ox + c0 * s, oy + r * s, len * s, s, col);
            });
  }
}

void renderGallery() {
  if (galleryDetail) {  // vista detalle: se redibuja siempre (animada)
    drawPageBackground();
    const DexEntry &d = DEX_TBL[galleryDetail];
    bool reg = galleryShowAll || pet.isRegistered(galleryDetail);
    char head[32];
    snprintf(head, sizeof(head), "N.%03d %s%s", galleryDetail,
             pet.isShinyRegistered(galleryDetail) ? "*" : "", reg ? dexName(galleryDetail) : "???");
    gfx->setTextColor(reg ? d.accent : UI_INK);
    // auto-encoge nombres largos (no caben a t3), midiendo en vez de contar
    // bytes; 13 caracteres a tamano 3 son los 234 px de antes.
    setSize(3);
    int gts = (textW(head, 3) <= 234) ? 3 : 2;
    setSize(gts);
    setCur(centerX(head, gts), gts == 3 ? 56 : 60);
    printT(head);
    if (galleryPmd.loaded) {
      // animado y a color si esta registrado; silueta estatica si no (estilo "?")
      drawPmdActM(galleryPmd, PMD_IDLE, CX, 300, reg ? millis() : 0, true, !reg, 6);
    } else {
      const uint8_t *t = thumbs.get(galleryDetail);
      if (t) drawThumb(t, CX - THUMB_CELL, 135, 4, !reg);
    }
    if (reg) {  // su tipo, sus stats de especie y en que pruebas del torneo destaca
      uint8_t ty = typeOf(galleryDetail);
      gfx->setTextColor(TYPE_COL[ty]);
      setSize(2);
      setCur(centerX(T((StrId)(S_TY_NORMAL + ty)), 2), 96);
      printT(T((StrId)(S_TY_NORMAL + ty)));
      char l[64];
      snprintf(l, sizeof(l), T(S_BASE_FMT), d.bHp, d.bAtk, d.bDef, d.bSpe);
      gfx->setTextColor(UI_INK);
      setCur(centerX(l, 2), 316);
      printT(l);
      Athlete a = athleteOf(galleryDetail);
      uint8_t e1 = 0, e2 = 1;  // las dos mejores
      for (uint8_t e = 0; e < EV_COUNT; e++) {
        float r = athleteRating(a, e);
        if (r > athleteRating(a, e1)) { e2 = e1; e1 = e; }
        else if (e != e1 && r > athleteRating(a, e2)) e2 = e;
      }
      if (e2 == e1) e2 = e1 ? 0 : 1;
      snprintf(l, sizeof(l), T(S_GOOD_AT_FMT), T(EV_NAME[e1]), T(EV_NAME[e2]));
      setCur(centerX(l, 2), 344);
      printT(l);
    }
    gfx->setTextColor(UI_HINT);
    setSize(2);
    setCur(centerX(T(S_DETAIL_BACK), 2), 396);
    printT(T(S_DETAIL_BACK));
    gfx->flush();
    return;
  }

  if (!galleryDirty) return;  // la rejilla es estatica
  galleryDirty = false;

  drawPageBackground();
  drawRegionDots();
  if (!galleryGrid) {
    renderRegionCover();
    gfx->flush();
    return;
  }
  const Region &rg = REGIONS[galleryRegion];
  char head[40];
  snprintf(head, sizeof(head), "%s %u/%d", T((StrId)(S_REGION_1 + galleryRegion)),
           regionRegistered(galleryRegion), rg.last - rg.first + 1);
  gfx->setTextColor(UI_INK);
  setSize(3);
  int hs = (textW(head, 3) <= 300) ? 3 : 2;
  setSize(hs);
  setCur(centerX(head, hs), 36);
  printT(head);

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int16_t dex = rg.first + galleryPage * 16 + r * 4 + c;
      if (dex > rg.last) break;
      int x = GAL_X + c * GAL_CELL, y = GAL_Y + r * GAL_ROW;
#if defined(TAMAPOKE_SMALLTV_PRO)
      gfx->fillRoundRect(x + 2, y + 2, GAL_CELL - 4, GAL_ROW - 4, 8, UI_WHITE);
      const int spriteInset = (GAL_CELL - THUMB_CELL) / 2;
#else
      const int spriteInset = 0;
#endif
      const uint8_t *t = thumbs.get(dex);
      if (t) {
        drawThumb(t, x + spriteInset, y, 2, !galleryShowAll && !pet.isRegistered(dex));
        if (pet.isShinyRegistered(dex)) {
          gfx->setTextColor(UI_BAR_WARN);
          setSize(2);
          setCur(x + 62, y + 4);
          printT("*");
        }
      } else {
        char num[6];
        snprintf(num, sizeof(num), "%d", dex);
        gfx->setTextColor(UI_HINT);
        setSize(2);
        setCur(x + 24, y + 32);
        printT(num);
      }
    }
  }
  if (pet.regionDone & (1 << galleryRegion)) drawCrown(CX + textW(head, hs) / 2 + 16, 30);
  // pagina dentro de la region (arriba/abajo), con flechas si hay mas
  char pg[8];
  snprintf(pg, sizeof(pg), "%d/%d", galleryPage + 1, regionPages(galleryRegion));
  gfx->setTextColor(UI_INK);
  setSize(2);
  setCur(centerX(pg, 2), 410);
  printT(pg);
  int w = textW(pg, 2);
  if (galleryPage > 0) gfx->fillTriangle(CX - w / 2 - 28, 424, CX - w / 2 - 20, 410, CX - w / 2 - 12, 424, UI_INK);
  if (galleryPage < regionPages(galleryRegion) - 1)
    gfx->fillTriangle(CX + w / 2 + 12, 410, CX + w / 2 + 28, 410, CX + w / 2 + 20, 424, UI_INK);
  gfx->flush();
}

// nueve puntos abajo: en que region se esta (relleno), las abiertas en tinta,
// las cerradas en gris
void drawRegionDots() {
  uint8_t abiertas = pet.unlockedRegions();
  for (int r = 0; r < REGION_COUNT; r++) {
    int x = CX - (REGION_COUNT - 1) * 8 + r * 16;
    uint16_t c = r < abiertas ? UI_INK : UI_TRACK;
    if (r == galleryRegion) gfx->fillCircle(x, 444, 5, c);
    else gfx->drawCircle(x, 444, 4, c);
  }
}

// las de huevo (comunes y raras) de una region: cuantas hay y cuantas registradas
static void regionHatchCount(int r, unsigned &got, unsigned &total) {
  got = total = 0;
  for (int d = REGIONS[r].first; d <= REGIONS[r].last; d++) {
    uint8_t ra = DEX_TBL[d].rarity;
    if (ra != R_COMUN && ra != R_RARO) continue;
    total++;
    if (pet.isRegistered(d)) got++;
  }
}

static void drawLock(int x, int y) {  // candado
  gfx->fillRoundRect(x - 12, y - 22, 24, 26, 12, UI_INK);
  gfx->fillRoundRect(x - 7, y - 17, 14, 22, 7, UI_BG_DAY);
  gfx->fillRoundRect(x - 18, y, 36, 28, 6, UI_INK);
  gfx->fillCircle(x, y + 12, 4, UI_BG_DAY);
  gfx->fillRect(x - 1, y + 14, 3, 7, UI_BG_DAY);
}

// paisaje apagado (region cerrada): a la mitad y algo gris
static uint16_t dimRegionPx(uint16_t v) {
  return (uint16_t)((((v >> 11) & 31) / 2 + 6) << 11 | (((v >> 5) & 63) / 2 + 12) << 5 | ((v & 31) / 2 + 6));
}

// portada de una region (una pagina por region): su paisaje grande y su nombre.
// Abierta: cuantas hay registradas y "tocar para ver"; cerrada: apagada, con
// candado y lo que falta de la anterior; completa: corona
#define COVER_W 418  // paisaje a x3.6, centrado en la pantalla: lo mas grande
#define COVER_H 166  // que cabe entero en el circulo (209^2 + 83^2 < 231^2)
#define COVER_Y (CY - COVER_H / 2)
void renderRegionCover() {
  bool abierta = galleryShowAll || galleryRegion < pet.unlockedRegions();
  const char *nm = T((StrId)(S_REGION_1 + galleryRegion));
  gfx->setTextColor(abierta ? UI_INK : UI_HINT);
  setSize(4);
  setCur(centerX(nm, 4), 96);
  printT(nm);
  if (pet.regionDone & (1 << galleryRegion)) drawCrown(CX + textW(nm, 4) / 2 + 18, 100);
  // escalado al vecino mas proximo: cada fila de origen se calcula una vez
  const uint16_t *img = REGION_IMG[galleryRegion];
  uint16_t fila[COVER_W];
  int sy = -1;
  for (int y = 0; y < COVER_H; y++) {
    int ny = y * REGION_IMG_H / COVER_H;
    if (ny != sy) {
      sy = ny;
      for (int x = 0; x < COVER_W; x++) {
        uint16_t v = img[sy * REGION_IMG_W + x * REGION_IMG_W / COVER_W];
        fila[x] = abierta ? v : dimRegionPx(v);
      }
    }
    gfx->draw16bitRGBBitmap(CX - COVER_W / 2, COVER_Y + y, fila, COVER_W, 1);
  }
  int bajo = COVER_Y + COVER_H;  // 316
  if (abierta) {
    const Region &rg = REGIONS[galleryRegion];
    char cnt[16];
    snprintf(cnt, sizeof(cnt), "%u/%d", regionRegistered(galleryRegion), rg.last - rg.first + 1);
    gfx->setTextColor(UI_INK);
    setSize(3);
    setCur(centerX(cnt, 3), bajo + 18);
    printT(cnt);
    gfx->setTextColor(UI_HINT);
    setSize(2);
    setCur(centerX(T(S_TAP_OPEN), 2), bajo + 64);
    printT(T(S_TAP_OPEN));
    return;
  }
  drawLock(CX, CY - 3);
  gfx->setTextColor(UI_INK);
  setSize(3);
  setCur(centerX(T(S_REGION_LOCKED), 3), bajo + 14);
  printT(T(S_REGION_LOCKED));
  const char *pista = T(S_UNLOCK_HINT);
  setSize(2);
  int ps = textW(pista, 2) <= 330 ? 2 : 1;  // mas abajo el circulo se estrecha
  setSize(ps);
  setCur(centerX(pista, ps), bajo + 50);
  printT(pista);
  unsigned got, total;
  regionHatchCount(galleryRegion - 1, got, total);
  char l[48];
  snprintf(l, sizeof(l), "%s %u/%u", T((StrId)(S_REGION_1 + galleryRegion - 1)), got, total);
  gfx->setTextColor(got < total ? UI_BAR_BAD : UI_BAR_OK);
  setSize(2);
  setCur(centerX(l, 2), bajo + 78);
  printT(l);
}

// ---------- pokedex por regiones ----------

int regionPages(int r) { return (REGIONS[r].last - REGIONS[r].first + 1 + 15) / 16; }

unsigned regionRegistered(int r) {
  unsigned n = 0;
  for (int d = REGIONS[r].first; d <= REGIONS[r].last; d++)
    if (pet.isRegistered(d)) n++;
  return n;
}

void galleryTap(int16_t x, int16_t y) {
  if (galleryDetail) {  // volver a la rejilla
    galleryDetail = 0;
    galleryPmd.unload();
    galleryDirty = true;
    return;
  }
  if (!galleryGrid) {  // portada de la region
    if (y < 72) {       // cabecera = salir de la pokedex
      galleryOpen = false;
      return;
    }
    if (!galleryShowAll && galleryRegion >= pet.unlockedRegions()) return;  // cerrada
    galleryGrid = true;  // tocar = ver sus pokemon
    galleryPage = 0;
    galleryDirty = true;
    return;
  }
  if (y < 72) {  // cabecera de la rejilla = volver a la portada
    galleryGrid = false;
    galleryDirty = true;
    return;
  }
  // mismo motivo que en el teclado: filtrar antes de dividir
  if (x < GAL_X || y < GAL_Y) return;
  int c = (x - GAL_X) / GAL_CELL, r = (y - GAL_Y) / GAL_ROW;
  if (c > 3 || r > 3) return;
  int16_t dex = REGIONS[galleryRegion].first + galleryPage * 16 + r * 4 + c;
  if (dex > REGIONS[galleryRegion].last) return;
  galleryDetail = dex;
  galleryPmd.load(dex, pet.isShinyRegistered(dex));
}

#if defined(TAMAPOKE_SMALLTV_PRO)
void smallTvGalleryTargets(SmallTvTargets &list) {
  if (galleryDetail) {
    list.context = 0x40000 + galleryDetail;
    list.add(96, 384, 274, 44);
    return;
  }
  list.context = 0x50000 + galleryRegion * 100 + galleryPage * 2 + galleryGrid;
  if (galleryGrid) {
    const auto &region = REGIONS[galleryRegion];
    for (int i = 0; i < 16 && region.first + galleryPage * 16 + i <= region.last; ++i)
      list.add(GAL_X + i % 4 * GAL_CELL, GAL_Y + i / 4 * GAL_ROW, GAL_CELL, GAL_ROW);
  } else if (galleryShowAll || galleryRegion < pet.unlockedRegions()) {
    list.add(CX - COVER_W / 2, COVER_Y, COVER_W, COVER_H);
  }
  list.add(20, 12, 426, 54);
}
#endif
