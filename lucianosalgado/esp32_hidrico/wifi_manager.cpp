#include "wifi_manager.h"

#include <WiFi.h>
#include "config.h"

namespace {
unsigned long lastWifiAttemptMs = 0;
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  unsigned long now = millis();
  if (lastWifiAttemptMs != 0 && (now - lastWifiAttemptMs < Config::WIFI_RETRY_MS)) {
    return;
  }
  lastWifiAttemptMs = now;

  Serial.print("[WiFi] Conectando a ");
  Serial.print(WIFI_SSID);
  Serial.println("...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String getLocalIpStr() {
  return WiFi.localIP().toString();
}
