#pragma once

#if defined(TAMAPOKE_SMALLTV_PRO)
#include <stdint.h>

class WebServer;

uint32_t smallTvOtaPin();
bool smallTvOtaArm();
void smallTvOtaRoutes(WebServer &server);
bool smallTvOtaRebootReady();
#endif
