#ifndef MOM_JEANS_VOICE_H
#define MOM_JEANS_VOICE_H

#include <stdint.h>
#include "pulsar.h"

#ifdef __cplusplus
extern "C" {
#endif

// Sync gate: stateless level detection matching main's `sync > 2.5f`.
// Single source of truth for sync on both platforms.
uint8_t mj_sync_gate(float volts);

#ifdef __cplusplus
}
#endif

#endif // MOM_JEANS_VOICE_H
