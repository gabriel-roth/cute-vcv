#include "mom_jeans_voice.h"
#include <math.h>

uint8_t mj_sync_gate(float volts) {
    return volts > 2.5f ? 1 : 0;
}

static float mj_clampf(float x, float a, float b) {
    return fmaxf(fminf(x, b), a);
}
static float mj_scaleLin(float x, float a, float b, float c, float d) {
    return (x - a) / (b - a) * (d - c) + c;
}

void mj_voice_map(const mj_voice_in_t *in, mj_voice_config_t *out) {
    float pitch_min = 23.0f, pitch_max = 49.0f;
    if (in->pitch_mode) { pitch_min = -12.0f; pitch_max = 84.0f; }

    float fm_index = in->fm_index_connected ? in->fm_index_cv : 1.0f;

    float pitch = mj_scaleLin(in->pitch_param, 0.0f, 1.0f, pitch_min, pitch_max);
    pitch += in->v_oct_cv * 12.0f;
    pitch = 27.5f * powf(2.0f, pitch / 12.0f);
    pitch += in->linear_fm_cv * fm_index * pitch * 0.2f;
    out->pulse_frequency = mj_clampf(pitch, 1.0f, 20000.0f);

    float density_input_voltage = in->density_cv + (in->density_param - 0.5f) * 10.0f;
    out->density_ratio = mj_clampf(density_input_voltage / 5.0f, -1.0f, 1.0f);
    out->mod_ratio = mj_clampf(in->cadence_cv / 5.0f + in->cadence_param, 0.0f, 1.0f);
    out->mod_depth = mj_clampf(in->torque_cv / 5.0f + in->torque_param, 0.0f, 1.0f);
    out->waveform = mj_clampf(in->shape_cv / 5.0f + in->shape_param, 0.0f, 1.0f) * 4.9999f;

    out->ratio_lock = in->quantization;
    out->frequency_couple = in->coupling;
}
