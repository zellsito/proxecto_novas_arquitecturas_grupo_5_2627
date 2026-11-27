#pragma once

#include "node_state.h"

using ValveCommandCallback = void (*)(bool open);
using FanCommandCallback = void (*)(bool on);

void initMqtt(ValveCommandCallback onValveCmd, FanCommandCallback onFanCmd);
void loopMqtt(const WaterNodeState& state);
bool isMqttConnected();
void publishTelemetry(const WaterNodeState& state);
void publishAccess(const WaterNodeState& state);
