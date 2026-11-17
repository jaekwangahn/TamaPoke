#if defined(TAMAPOKE_SMALLTV_PRO)
#include "smalltv_ota.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_ota_ops.h>
#include <stdlib.h>

namespace {
portMUX_TYPE otaLock = portMUX_INITIALIZER_UNLOCKED;
uint32_t pin = 0;
uint32_t expiresAt = 0;
uint8_t failedPins = 0;
bool busy = false;
bool rebootPending = false;
uint32_t rebootAt = 0;

WebServer *server = nullptr;
const esp_partition_t *destination = nullptr;
esp_ota_handle_t writer = 0;
size_t expected = 0;
bool writing = false;
int result = 400;
const char *message = "Send an application BIN with Content-Type: application/octet-stream.";

const char uploadPage[] PROGMEM = R"html(
<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>TamaPoke OTA</title>
<h1>TamaPoke OTA</h1>
<p>Open OTA UPDATE under Settings on the SmallTV PRO to get a PIN.</p>
<form id="up">
<label>PIN <input id="pin" inputmode="numeric" pattern="[0-9]{6}" required></label>
<label>App firmware <input type="file" id="bin" accept=".bin" required></label>
<button id="send">Update</button>
</form>
<p id="status" role="status"></p>
<script>
const up=document.querySelector('#up'), send=document.querySelector('#send'), statusLine=document.querySelector('#status');
up.onsubmit=async (event)=>{
  event.preventDefault(); send.disabled=true; statusLine.textContent='Uploading...';
  try {
    const res=await fetch('/update',{method:'POST',headers:{
      'Content-Type':'application/octet-stream',
      'X-TamaPoke-Pin':document.querySelector('#pin').value
    },body:document.querySelector('#bin').files[0]});
    statusLine.textContent=await res.text();
    if (!res.ok) send.disabled=false;
  } catch {
    statusLine.textContent='Connection lost. Check the device before retrying.';
    send.disabled=false;
  }
};
</script></html>
)html";

void failUpdate(int code, const char *reason) {
  if (writing) {
    esp_ota_abort(writer);
    writing = false;
  }
  result = code;
  message = reason;
  portENTER_CRITICAL(&otaLock);
  busy = false;
  portEXIT_CRITICAL(&otaLock);
}

bool beginUpload() {
  result = 400;
  message = "Send an application BIN with Content-Type: application/octet-stream.";
  writing = false;
  if (server->header("Content-Type") != "application/octet-stream") return false;
  const String token = server->header("X-TamaPoke-Pin");
  if (token.length() != 6) {
    result = 403;
    message = "Enter the PIN shown on the device.";
    return false;
  }
  for (char c : token) {
    if (c < '0' || c > '9') {
      result = 403;
      message = "Enter the PIN shown on the device.";
      return false;
    }
  }
  const uint32_t entered = token.toInt();
  const uint32_t now = millis();
  portENTER_CRITICAL(&otaLock);
  const bool authorized = !busy && !rebootPending && pin && entered == pin &&
                          static_cast<int32_t>(expiresAt - now) > 0;
  if (authorized) {
    pin = 0;
    busy = true;
  } else if (pin && static_cast<int32_t>(expiresAt - now) > 0 && ++failedPins >= 5) {
    pin = 0;
  }
  portEXIT_CRITICAL(&otaLock);
  if (!authorized) {
    result = 403;
    message = "PIN unavailable, expired, or incorrect.";
    return false;
  }

  const String lengthHeader = server->header("Content-Length");
  char *end;
  expected = strtoul(lengthHeader.c_str(), &end, 10);
  destination = esp_ota_get_next_update_partition(nullptr);
  if (!destination || destination == esp_ota_get_running_partition()) {
    failUpdate(409, "No inactive OTA application partition.");
    return false;
  }
  if (*end || expected < 256 || expected > destination->size) {
    failUpdate(413, "Application image does not fit the OTA partition.");
    return false;
  }
  if (esp_ota_begin(destination, expected, &writer) != ESP_OK) {
    failUpdate(500, "Could not start the OTA write. Check the current firmware and retry.");
    return false;
  }
  writing = true;
  return true;
}

class OtaUpload : public RequestHandler {
public:
  bool canHandle(WebServer &, HTTPMethod method, const String &uri) override {
    return method == HTTP_POST && uri == "/update";
  }
  bool canRaw(WebServer &, const String &uri) override { return uri == "/update"; }

  void raw(WebServer &web, const String &, HTTPRaw &body) override {
    switch (body.status) {
      case RAW_START:
        if (!beginUpload()) {
          web.send(result, "text/plain", message);
          web.client().stop();
        }
        break;
      case RAW_WRITE:
        if (writing && (body.totalSize > expected ||
            esp_ota_write(writer, body.buf, body.currentSize) != ESP_OK)) {
          failUpdate(422, "Application image could not be written.");
          web.send(result, "text/plain", message);
          web.client().stop();
        }
        break;
      case RAW_END:
        if (!writing) break;
        if (body.totalSize != expected) {
          failUpdate(422, "Incomplete application image.");
          break;
        }
        writing = false;  // esp_ota_end releases the handle even on failure.
        if (esp_ota_end(writer) != ESP_OK || esp_ota_set_boot_partition(destination) != ESP_OK) {
          failUpdate(422, "Invalid or incompatible application image.");
          break;
        }
        result = 200;
        message = "Update accepted. Restarting...";
        break;
      case RAW_ABORTED:
        if (writing) failUpdate(400, "Upload interrupted.");
        break;
    }
  }

  bool handle(WebServer &web, HTTPMethod, const String &) override {
    web.sendHeader("Cache-Control", "no-store");
    web.send(result, "text/plain", message);
    if (result == 200) {
      const auto restartAfter = millis() + 1200;
      portENTER_CRITICAL(&otaLock);
      busy = false;
      rebootAt = restartAfter;
      rebootPending = true;
      portEXIT_CRITICAL(&otaLock);
    }
    result = 400;
    message = "Send an application BIN with Content-Type: application/octet-stream.";
    return true;
  }
};
}

uint32_t smallTvOtaPin() {
  const uint32_t now = millis();
  portENTER_CRITICAL(&otaLock);
  const uint32_t active = static_cast<int32_t>(expiresAt - now) > 0 ? pin : 0;
  portEXIT_CRITICAL(&otaLock);
  return active;
}

bool smallTvOtaArm() {
  if (WiFi.status() != WL_CONNECTED || !esp_ota_get_next_update_partition(nullptr)) return false;
  const uint32_t nextPin = 100000 + esp_random() % 900000;
  const uint32_t deadline = millis() + 180000;
  portENTER_CRITICAL(&otaLock);
  const bool armed = !busy && !rebootPending;
  if (armed) {
    pin = nextPin;
    expiresAt = deadline;
    failedPins = 0;
  }
  portEXIT_CRITICAL(&otaLock);
  return armed;
}

void smallTvOtaRoutes(WebServer &web) {
  server = &web;
  static const char *headers[] = { "X-TamaPoke-Pin", "Content-Length", "Content-Type" };
  web.collectHeaders(headers, sizeof(headers) / sizeof(headers[0]));
  web.addHandler(new OtaUpload());
  web.on("/update", HTTP_GET, [&web] {
    web.sendHeader("Cache-Control", "no-store");
    web.send_P(200, "text/html; charset=utf-8", uploadPage);
  });
}

bool smallTvOtaRebootReady() {
  const uint32_t now = millis();
  portENTER_CRITICAL(&otaLock);
  if (pin && static_cast<int32_t>(now - expiresAt) >= 0) pin = 0;
  const bool ready = rebootPending && static_cast<int32_t>(now - rebootAt) >= 0;
  portEXIT_CRITICAL(&otaLock);
  return ready;
}
#endif
