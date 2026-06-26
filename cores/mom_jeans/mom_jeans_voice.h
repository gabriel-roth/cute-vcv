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

typedef struct {
    float   pitch_param;     // 0..1 (raw PITCH_PARAM)
    float   density_param;   // 0..1 (VCV DENSITY_PARAM/100)
    float   torque_param;    // 0..1 (VCV TORQUE_PARAM/100)
    float   cadence_param;   // 0..1 (VCV CADENCE_PARAM/100)
    float   shape_param;     // 0..1 (VCV SHAPE_PARAM/5)
    uint8_t coupling;        // 0/1
    uint8_t quantization;    // 0/1
    uint8_t pitch_mode;      // 0/1 (extended range)
    float   density_cv, shape_cv, torque_cv, cadence_cv;  // volts
    float   fm_index_cv;  uint8_t fm_index_connected;     // 0/1
    float   linear_fm_cv, v_oct_cv;                       // volts
} mj_voice_in_t;

typedef struct {
    float   pulse_frequency, density_ratio, mod_ratio, mod_depth, waveform;
    uint8_t ratio_lock, frequency_couple;
} mj_voice_config_t;

void mj_voice_map(const mj_voice_in_t *in, mj_voice_config_t *out);

#ifdef __cplusplus
}
#endif

#endif // MOM_JEANS_VOICE_H
