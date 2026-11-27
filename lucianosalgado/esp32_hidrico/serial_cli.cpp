#include "serial_cli.h"

#include <Arduino.h>

#include "mqtt_transport.h"
#include "sensors_hw.h"
#include "sensors_sim.h"
#include "wifi_manager.h"

namespace {
String serialBuffer;

void handleCommand(WaterNodeState& state, String cmd) {
  cmd.trim();
  if (cmd.isEmpty()) return;

  String lower = cmd;
  lower.toLowerCase();

  if (lower == "help") {
    printCliHelp();
  } else if (lower == "status") {
    printCliStatus(state);
  } else if (lower == "fuga") {
    setSimulatedLeak(state, true);
    publishTelemetry(state);
  } else if (lower == "secar") {
    setSimulatedLeak(state, false);
    publishTelemetry(state);
  } else if (lower.startsWith("nivel ")) {
    state.useHardwareSensors = false;
    setSimulatedLevel(state, lower.substring(6).toFloat());
    publishTelemetry(state);
  } else if (lower == "sim on") {
    state.useHardwareSensors = false;
    state.autoSimEnabled = true;
    Serial.println("[SIM] Modo simulado ON (SEN0193 pausado)");
  } else if (lower == "sim off" || lower == "hw") {
    state.autoSimEnabled = false;
    state.useHardwareSensors = true;
    Serial.println("[HW] Lectura real de sensores ON");
    readSensorsHw(state);
    publishTelemetry(state);
  } else if (lower == "pub") {
    readSensorsHw(state);
    publishAccess(state);
    publishTelemetry(state);
  } else if (lower == "escotilla") {
    state.hatchOpen = !state.hatchOpen;
    Serial.print("[CLI] Escotilla -> ");
    Serial.println(state.hatchStateStr());
    publishAccess(state);
  } else {
    Serial.print("[?] Comando no reconocido: ");
    Serial.println(cmd);
    Serial.println("Escribe 'help' para ver los comandos.");
  }
}
}  // namespace

void printCliHelp() {
  Serial.println("Comandos Monitor Serie:");
  Serial.println("  status | fuga | secar | escotilla | nivel <0-100> | hw | sim on | sim off | pub | help");
}

void printCliStatus(const WaterNodeState& state) {
  Serial.print("WiFi: ");
  if (isWiFiConnected()) {
    Serial.print("OK (");
    Serial.print(getLocalIpStr());
    Serial.println(")");
  } else {
    Serial.println("DESCONECTADO");
  }
  Serial.print("MQTT: ");
  Serial.println(isMqttConnected() ? "OK" : "DESCONECTADO");
  Serial.print("Tanque (SEN0193): ");
  Serial.print(state.tankLevelPct, 1);
  Serial.print("% (ADC=");
  Serial.print(state.soilRawAdc);
  Serial.print(") | Fuga: ");
  Serial.print(state.leakDetected ? "SI" : "NO");
  Serial.print(" | Válvula: ");
  Serial.println(state.valveStateStr());
  Serial.print("SCD30: ");
  if (state.scd30Ready) {
    Serial.print(state.temperatureC, 1);
    Serial.print(" °C | ");
    Serial.print(state.humidityPct, 1);
    Serial.print("% HR | ");
    Serial.print(state.co2Ppm, 0);
    Serial.print(" ppm CO2 | Extractor: ");
    Serial.println(state.fanStateStr());
  } else {
    Serial.println("esperando lectura...");
  }
  Serial.print("Modo sensores: ");
  Serial.println(state.useHardwareSensors ? "HARDWARE" : "SIMULADO");
}

void pollSerialCli(WaterNodeState& state) {
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      handleCommand(state, serialBuffer);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}
