#include "office_timer.h"

static bool officeMode = false;
// Session and bonus share one NVS record so a reset cannot replay a reward.
static constexpr uint32_t STUDY_UNIT_SECONDS = 3600;
static constexpr int STUDY_DURATIONS[] = {1, 2, 4, 8};
static StudyProgress study;
static bool studyStopAsk = false;

static void loadStudy() {
  Preferences prefs;
  if (!prefs.begin("study", true)) return;
  study = StudyProgress::decode(prefs.getULong64("session", 0));
  prefs.end();
  pet.deepgongStudyLevels = study.bonus;
}

static bool saveStudy(const StudyProgress &next) {
  Preferences prefs;
  if (!prefs.begin("study", false)) return false;
  const uint64_t record = next.encode();
  const bool saved = prefs.putULong64("session", record) == sizeof(record);
  prefs.end();
  if (saved) {
    study = next;
    pet.deepgongStudyLevels = study.bonus;
  }
  return saved;
}

static bool finishStudy(uint32_t now) {
  StudyProgress next = study;
  return !next.complete(now) || saveStudy(next);
}

static void updateStudy(uint32_t ms) {
  if (!study.endsAt || study.awarded) return;
  static uint32_t checkedAt = 0;
  if (ms - checkedAt < 1000) return;
  checkedAt = ms;
  finishStudy(rtcEpoch());
}

void studyEnter() {
  studyStopAsk = false;
  officeMode = false;
}

static void studyText(const char *text, int y, int size, uint16_t color) {
  setSize(size);
  gfx->setTextColor(color);
  setCur(centerX(text, size), y);
  printT(text);
}

static void studyBook(int x, int y, int scale, uint16_t cover) {
  gfx->fillRoundRect(x, y, 22 * scale, 13 * scale, 2 * scale, cover);
  gfx->fillRoundRect(x + scale, y + scale, 9 * scale, 10 * scale, scale, UI_WHITE);
  gfx->fillRoundRect(x + 12 * scale, y + scale, 9 * scale, 10 * scale, scale, UI_WHITE);
  gfx->fillRect(x + 10 * scale, y + 2 * scale, 2 * scale, 9 * scale, cover);
  for (int row = 0; row < 3; ++row) {
    gfx->fillRect(x + 3 * scale, y + (4 + row * 2) * scale, 5 * scale, scale, 0xA514);
    gfx->fillRect(x + 14 * scale, y + (4 + row * 2) * scale, 5 * scale, scale, 0xA514);
  }
}

void drawStudyIcon(int x, int y) {
  studyBook(x + 2, y + 10, 2, C565(0x21, 0x82, 0x86));
}

static void drawDeepgongAtDesk(bool writing) {
  const uint16_t hood = C565(0xf4, 0xbf, 0x53);
  const uint16_t gold = C565(0xbe, 0x80, 0x39);
  const uint16_t skin = C565(0xff, 0xe2, 0xbb);
  const uint16_t light = C565(0xff, 0xe1, 0x7c);
  const uint32_t now = millis();
  const int pen = writing ? int((now / 145) % 6) : 0;

  // Desktop lamp and chair behind the character.
  gfx->fillRoundRect(178, 230, 108, 93, 22, C565(0x33, 0x4f, 0x60));
  gfx->fillRoundRect(294, 278, 71, 10, 4, gold);
  gfx->fillRoundRect(347, 209, 7, 72, 3, gold);
  gfx->fillTriangle(312, 217, 363, 217, 344, 186, light);
  gfx->fillRoundRect(311, 214, 53, 8, 3, hood);

  drawDeepgongSprite(CX, 277, 153, deepgongLevelForStudy());

  gfx->fillRoundRect(94, 276, 278, 24, 10, C565(0x9b, 0x60, 0x45));
  gfx->fillRoundRect(94, 272, 278, 14, 7, C565(0xca, 0x8b, 0x5c));
  gfx->fillRect(116, 295, 12, 28, C565(0x83, 0x50, 0x40));
  gfx->fillRect(338, 295, 12, 28, C565(0x83, 0x50, 0x40));
  studyBook(CX - 33, 243, 3, C565(0x21, 0x82, 0x86));
  gfx->fillCircle(202, 259, 10, skin);
  gfx->fillCircle(264 + pen, 254, 9, skin);
  gfx->drawLine(252 + pen, 263, 281 + pen, 236, gold);
  gfx->drawLine(253 + pen, 263, 282 + pen, 236, light);
}

static constexpr int STUDY_BUTTON_X = 125;
static constexpr int STUDY_BUTTON_W = 100;
static constexpr int STUDY_BUTTON_H = 41;
static constexpr int STUDY_BUTTON_Y = 337;
static constexpr int STUDY_BUTTON_GAP = 8;

static void drawOfficeMode() {
  const OfficeView state = officeView();
  const uint16_t mint = C565(0x7d, 0xd9, 0xd6);
  drawDeepgongCampus(false);
  gfx->fillRoundRect(CX - 120, 22, 240, 105, 15, C565(0x17, 0x28, 0x38));
  studyText("CNS OFFICE", 36, 2, UI_WHITE);
  char label[24];
  if (state.code >= 0) {
    snprintf(label, sizeof(label), "%08d", state.code);
    studyText(label, 64, 4, mint);
  } else if (state.remaining) {
    snprintf(label, sizeof(label), "%02u:%02u", unsigned(state.remaining / 60), unsigned(state.remaining % 60));
    studyText(label, 64, 4, mint);
  } else {
    studyText(state.pending ? "SYNC" : state.linked ? "25:00" : "CONNECT", 64, 3, mint);
  }
  studyText(state.online ? "Wi-Fi" : "Offline", 104, 1, UI_WHITE);
  drawDeepgongAtDesk(state.remaining > 0);
  gfx->fillRoundRect(CX - 100, 299, 200, 42, 10, C565(0x32, 0x57, 0x66));
  studyText(!state.linked ? "Get code" : state.checked ? "Checked in" : "Check in", 312, 2, UI_WHITE);
  gfx->fillRoundRect(CX - 100, 346, 200, 42, 10, C565(0x32, 0x57, 0x66));
  studyText(state.remaining ? "Cancel" : state.pending ? "Sync pending" : "Start 25 min", 359, 2, UI_WHITE);
  studyText("Study", 407, 2, mint);
}

void renderStudy() {
  if (officeMode) {
    drawOfficeMode();
    return;
  }
  const uint32_t now = rtcEpoch();
  const bool active = !study.awarded && study.endsAt && (!now || study.endsAt > now);
  const uint16_t mint = C565(0x7d, 0xd9, 0xd6);
  drawDeepgongCampus(false);
  gfx->fillRoundRect(CX - 104, 18, 208, 88, 18, C565(0x17, 0x28, 0x38));
  studyText(T(S_DEEPGONG), 30, 3, UI_WHITE);
  drawDeepgongAtDesk(active);

  if (active) {
    const uint32_t remaining = study.endsAt - now;
    char clock[20];
    snprintf(clock, sizeof(clock), "%02lu:%02lu:%02lu", (unsigned long)(remaining / 3600),
             (unsigned long)(remaining / 60 % 60), (unsigned long)(remaining % 60));
    studyText(clock, 63, 4, mint);
    const uint32_t duration = uint32_t(study.hours) * STUDY_UNIT_SECONDS;
    if (duration) {
      const int progress = remaining >= duration ? 0 : (duration - remaining) * 182 / duration;
      gfx->fillRoundRect(CX - 92, 318, 184, 7, 3, C565(0x37, 0x59, 0x67));
      if (progress) gfx->fillRect(CX - 91, 319, progress, 5, mint);
    }
    gfx->fillRoundRect(CX - 100, 359, 200, 46, 12, C565(0x32, 0x57, 0x66));
    studyText(T(S_STUDY_END), 373, 2, UI_WHITE);
  } else {
    studyText(T(study.awarded ? S_STUDY_DONE : S_STUDY_PICK), 75, 2, mint);
    for (int i = 0; i < 4; ++i) {
      const int x = STUDY_BUTTON_X + i % 2 * (STUDY_BUTTON_W + STUDY_BUTTON_GAP);
      const int y = STUDY_BUTTON_Y + i / 2 * (STUDY_BUTTON_H + STUDY_BUTTON_GAP);
      gfx->fillRoundRect(x, y, STUDY_BUTTON_W, STUDY_BUTTON_H, 9, C565(0x32, 0x57, 0x66));
      char duration[24];
      snprintf(duration, sizeof(duration), T(STUDY_UNIT_SECONDS == 60 ? S_MINS_FMT : S_HOURS_FMT),
               (unsigned)STUDY_DURATIONS[i]);
      setSize(2);
      gfx->setTextColor(UI_WHITE);
      setCur(x + (STUDY_BUTTON_W - textW(duration, 2)) / 2, y + 12);
      printT(duration);
    }
  }

  studyText("CNS OFFICE", 440, 1, mint);
  if (studyStopAsk)
    drawAskBox(T(S_STUDY_STOP_Q), "");
}

void studyTap(int16_t x, int16_t y) {
  if (officeMode) {
    if (abs(x - CX) > 100) return;
    const OfficeView state = officeView();
    if (y >= 390 && y < 445) officeMode = false;
    else if (y >= 299 && y < 341 && state.online)
      officeCommand(state.linked ? OfficeCommand::Checkin : OfficeCommand::Pair);
    else if (y >= 346 && y < 388 && state.linked && !state.pending)
      officeCommand(state.remaining ? OfficeCommand::Stop : OfficeCommand::Start);
    return;
  }
  if (!studyStopAsk && abs(x - CX) <= 95 && y >= 425 && y < 465) {
    officeMode = true;
    officeBegin();
    return;
  }
  const uint32_t now = rtcEpoch();
  if (!finishStudy(now)) {
    toast(T(S_STUDY_SAVE_FAIL));
    return;
  }
  if (studyStopAsk) {
    studyStopAsk = false;
    if (askTap(x, y) && !study.awarded) {
      StudyProgress next = study;
      next.endsAt = 0;
      next.hours = 0;
      if (!saveStudy(next)) toast(T(S_STUDY_SAVE_FAIL));
    }
    return;
  }
  if (!study.awarded && study.endsAt && (!now || study.endsAt > now)) {
    if (abs(x - CX) <= 100 && y >= 359 && y < 405) studyStopAsk = true;
    return;
  }
  for (int i = 0; i < 4; ++i) {
    const int bx = STUDY_BUTTON_X + i % 2 * (STUDY_BUTTON_W + STUDY_BUTTON_GAP);
    const int by = STUDY_BUTTON_Y + i / 2 * (STUDY_BUTTON_H + STUDY_BUTTON_GAP);
    if (x < bx || x >= bx + STUDY_BUTTON_W || y < by || y >= by + STUDY_BUTTON_H) continue;
    if (!now) {
      toast(T(S_TIME_UNSET));
      return;
    }
    StudyProgress next = study;
    next.hours = STUDY_DURATIONS[i];
    next.endsAt = now + next.hours * STUDY_UNIT_SECONDS;
    next.awarded = false;
    if (saveStudy(next)) sfxPlay(SFX_TAP);
    else toast(T(S_STUDY_SAVE_FAIL));
    return;
  }
}

bool studyCancelPrompt() {
  const bool open = studyStopAsk;
  studyStopAsk = false;
  return open;
}

static uint16_t deepgongLevelForStudy() {
  if (!pet.vacant && pet.speciesId == DEEPGONG_ID) return pet.level();
  for (int i = 0; i < farmCount(); ++i) {
    const FarmEntry &entry = farmAt(i);
    if (entry.speciesId == DEEPGONG_ID) return farmLevel(entry, pet);
  }
  return levelOfAge(0, pet.deepgongStudyLevels);
}

static void drawDeepgongSprite(int x, int ground, int size, uint16_t level) {
  const auto &sprite = NOTI_SPRITES[deepgongForm(level)];
  const int left = x - size / 2;
  const int top = ground - size;
  uint16_t *const canvas = gfx->getFramebuffer();
  const int firstX = max(0, left);
  const int lastX = min(LCD_WIDTH, left + size);
  for (int y = max(0, top); y < min(LCD_HEIGHT, ground); ++y) {
    const int sourceY = (y - top) * NOTI_SIZE / size;
    uint16_t *const row = canvas + y * LCD_WIDTH;
    for (int col = firstX; col < lastX; ++col) {
      const uint16_t pixel = sprite[sourceY * NOTI_SIZE + (col - left) * NOTI_SIZE / size];
      if (pixel != NOTI_TRANSPARENT) row[col] = pixel;
    }
  }
}

void drawDeepgongPet(int x, int ground, int scale, bool moving, uint16_t level) {
  const int bounce = moving ? (millis() / 180 % 2) * scale : 0;
  drawDeepgongSprite(x, ground - bounce, 25 * scale, level);
}
