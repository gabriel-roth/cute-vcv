#include "mom_jeans_voice.h"

uint8_t mj_sync_gate(float volts) {
    return volts > 2.5f ? 1 : 0;
}
