#include "mqtt_transport.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "time_sync.h"
#include "wifi_manager.h"

namespace {
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
ValveCommandCallback valveCmdCallback = nullptr;
FanCommandCallback fanCmdCallback = nullptr;
unsigned long lastMqttAttemptMs = 0;

void handleIncomingMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) {
    msg += static_cast<char>(payload[i]);
  }

  Serial.print("[RX] ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  if (String(topic) != Config::actuatorsStateTopic()) {
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, msg)) {
    return;
  }

  if (doc["valve_state"].is<const char*>() && valveCmdCallback != nullptr) {
    String desired = doc["valve_state"].as<const char*>();
    desired.trim();
    desired.toUpperCase();
    if (desired == "OPEN") {
      valveCmdCallback(true);
    } else if (desired == "CLOSED") {
      valveCmdCallback(false);
    }
  }

  if (doc["fan_state"].is<const char*>() && fanCmdCallback != nullptr) {
    String desiredFan = doc["fan_state"].as<const char*>();
    desiredFan.trim();
    desiredFan.toUpperCase();
    if (desiredFan == "ON") {
      fanCmdCallback(true);
    } else if (desiredFan == "OFF") {
      fanCmdCallback(false);
    }
  }
}

void ensureMqtt(const WaterNodeState& state) {
  if (!isWiFiConnected() || mqtt.connected()) {
    return;
  }
  unsigned long now = millis();
  if (lastMqttAttemptMs != 0 && (now - lastMqttAttemptMs < Config::MQTT_RETRY_MS)) {
    return;
  }
  lastMqttAttemptMs = now;

  String clientId = String(DEVICE_ID) + "-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
  Serial.print("[MQTT] Conectando a ");
  Serial.print(MQTT_HOST);
  Serial.print(":");
  Serial.print(MQTT_PORT);
  Serial.print(" como ");
  Serial.print(clientId);
  Serial.println("...");

  bool ok = (strlen(MQTT_USER) > 0)
                ? mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)
                : mqtt.connect(clientId.c_str());

  if (ok) {
    Serial.println("[MQTT] Conectado.");
    String stateTopic = Config::actuatorsStateTopic();
    mqtt.subscribe(stateTopic.c_str(), 1);
    Serial.print("[MQTT] Suscrito a ");
    Serial.println(stateTopic);
    publishAccess(state);
    publishTelemetry(state);
  } else {
    Serial.print("[MQTT] Error de conexión, rc=");
    Serial.println(mqtt.state());
  }
}
}  // namespace

void initMqtt(ValveCommandCallback onValveCmd, FanCommandCallback onFanCmd) {
  valveCmdCallback = onValveCmd;
  fanCmdCallback = onFanCmd;
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(Config::MQTT_BUFFER_SIZE);
  mqtt.setCallback(handleIncomingMessage);
}

void loopMqtt(const WaterNodeState& state) {
  ensureMqtt(state);
  if (mqtt.connected()) {
    mqtt.loop();
  }
}

bool isMqttConnected() {
  return mqtt.connected();
}

void publishTelemetry(const WaterNodeState& state) {
  String ts = getIsoTimestamp();

  // 1. Telemetría hídrica (SEN0193)
  {
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["tank_level_pct"] = roundf(state.tankLevelPct * 10.0f) / 10.0f;
    doc["leak_detected"] = state.leakDetected;
    doc["valve_state"] = state.valveStateStr();
    doc["simulated"] = !state.useHardwareSensors;
    doc["timestamp"] = ts;

    String topic = Config::telemetryTopic();
    String payload;
    serializeJson(doc, payload);

    Serial.print("[TX] ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(payload);

    if (mqtt.connected()) {
      mqtt.publish(topic.c_str(), payload.c_str(), false);
    } else {
      Serial.println("[!] MQTT desconectado (solo Serial)");
    }
  }

  // 2. Telemetría ambiental (SCD30: temperatura, humedad y CO2)
  if (state.scd30Ready) {
    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["temperature_c"] = roundf(state.temperatureC * 10.0f) / 10.0f;
    doc["humidity_pct"] = roundf(state.humidityPct * 10.0f) / 10.0f;
    doc["gas_ppm"] = roundf(state.co2Ppm);
    doc["fan_state"] = state.fanStateStr();
    doc["simulated"] = false;
    doc["timestamp"] = ts;

    String topic = Config::environmentTopic();
    String payload;
    serializeJson(doc, payload);

    Serial.print("[TX] ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(payload);

    if (mqtt.connected()) {
      mqtt.publish(topic.c_str(), payload.c_str(), false);
    }
  }
}

void publishAccess(const WaterNodeState& state) {
  JsonDocument doc;
  doc["device_id"] = DEVICE_ID;
  doc["hatch_state"] = state.hatchStateStr();
  doc["simulated"] = false;
  doc["timestamp"] = getIsoTimestamp();

  String topic = Config::accessTopic();
  String payload;
  serializeJson(doc, payload);

  Serial.print("[TX] ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  if (mqtt.connected()) {
    mqtt.publish(topic.c_str(), payload.c_str(), true);
  }
}
