#ifndef MOM_JEANS_VOICE_H
#define MOM_JEANS_VOICE_H

#include <stdint.h>
#include "pulsar.h"

#ifdef __cplusplus
extern "C" {
#endif

// Sync gate (legacy, stateless level detection). Retained only until call
// sites migrate to mj_sync_trigger; removed in the same change.
uint8_t mj_sync_gate(float volts);

// Rising-edge sync detector. Single source of truth for both platforms.
// Fires (returns 1) only on the LOW->HIGH transition; resets at MJ_SYNC_LOW.
// Thresholds match the original dsp::SchmittTrigger defaults.
#define MJ_SYNC_HIGH 1.0f
#define MJ_SYNC_LOW  0.0f

typedef struct { uint8_t high; } mj_sync_state_t;

uint8_t mj_sync_trigger(mj_sync_state_t *st, float volts);

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

typedef struct { float pulse, trigger, internal_lfo, mod_rate; } mj_voice_out_t;

void mj_voice_map(const mj_voice_in_t *in, mj_voice_config_t *out);

// Runs mj_voice_map, then pulsar_configure + pulsar_process for one sample.
// `trigger` is the raw pulsar sync output (caller scales x10 like `pulse`).
mj_voice_out_t mj_voice_process(ps_t *pulsar, const mj_voice_in_t *in, uint8_t sync_gate);

#ifdef __cplusplus
}
#endif

#endif // MOM_JEANS_VOICE_H
