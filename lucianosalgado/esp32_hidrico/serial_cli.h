#pragma once

#include "node_state.h"

void printCliHelp();
void printCliStatus(const WaterNodeState& state);
void pollSerialCli(WaterNodeState& state);
