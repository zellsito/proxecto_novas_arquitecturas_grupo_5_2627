#include <Arduino.h>

#include "config.h"
#include "mqtt_transport.h"
#include "node_state.h"
#include "sensors_hw.h"
#include "sensors_sim.h"
#include "serial_cli.h"
#include "time_sync.h"
#include "valve_actuator.h"
#include "wifi_manager.h"

namespace {
WaterNodeState nodeState;
unsigned long lastTelemetryMs = 0;

void onValveCommand(bool open) {
  if (open == nodeState.valveOpen) {
    return;
  }
  setValveState(nodeState, open);
  publishTelemetry(nodeState);
}

void onFanCommand(bool on) {
  if (on == nodeState.fanOn) {
    setFanLed(on);
    return;
  }
  nodeState.fanOn = on;
  setFanLed(on);
  Serial.print("[ACTUADOR] Extractor (LED GPIO 21) -> ");
  Serial.println(nodeState.fanStateStr());
  publishTelemetry(nodeState);
}
}  // namespace

void setup() {
  Serial.begin(Config::SERIAL_BAUD);
  delay(1500);

  Serial.println("\nBunkerOS - Nodo ESP32-S3 (SEN0193 + SCD30 + Escotilla/LED)");

  initSensorsSim();
  initSensorsHw();
  initTimeSync();
  initMqtt(onValveCommand, onFanCommand);

  ensureWiFi();
  readSensorsHw(nodeState);
  printCliHelp();
}

void loop() {
  pollSerialCli(nodeState);
  if (pollHatchButton(nodeState)) {
    publishAccess(nodeState);
  }
  ensureWiFi();
  loopMqtt(nodeState);

  unsigned long now = millis();
  if (now - lastTelemetryMs >= Config::TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;
    if (nodeState.useHardwareSensors) {
      readSensorsHw(nodeState);
    } else {
      stepSensorsSim(nodeState);
      readSensorsHw(nodeState);
    }
    publishTelemetry(nodeState);
  }
}
