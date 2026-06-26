#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "mom_jeans_voice.h"

static int failures = 0;
#define CHECK(cond) do { if(!(cond)){ printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); failures++; } } while(0)

static int approx(float a, float b, float eps) { return fabsf(a-b) <= eps; }

static void test_voice_map(void) {
    mj_voice_config_t out;

    // Default C4: pitch_param = 16/26, normal range -> scaleLin->39 semitones -> ~261.6 Hz
    mj_voice_in_t in = {0};
    in.pitch_param = 16.0f/26.0f;
    in.fm_index_connected = 0;
    mj_voice_map(&in, &out);
    CHECK(approx(out.pulse_frequency, 261.6f, 1.0f));

    // +1V V/oct doubles the frequency
    in.v_oct_cv = 1.0f;
    mj_voice_map(&in, &out);
    CHECK(approx(out.pulse_frequency, 523.2f, 2.0f));

    // Extended range, pitch_param 0 -> -12 semitones -> 13.75 Hz
    mj_voice_in_t in2 = {0};
    in2.pitch_mode = 1;
    mj_voice_map(&in2, &out);
    CHECK(approx(out.pulse_frequency, 13.75f, 0.1f));

    // Density center (param 0.5, no CV) -> ratio 0; param 1.0 -> clamps to 1
    mj_voice_in_t in3 = {0};
    in3.pitch_param = 0.5f; in3.density_param = 0.5f;
    mj_voice_map(&in3, &out);
    CHECK(approx(out.density_ratio, 0.0f, 1e-4f));
    in3.density_param = 1.0f;
    mj_voice_map(&in3, &out);
    CHECK(approx(out.density_ratio, 1.0f, 1e-4f));

    // Waveform: shape_param 1.0 -> 4.9999
    mj_voice_in_t in4 = {0};
    in4.shape_param = 1.0f;
    mj_voice_map(&in4, &out);
    CHECK(approx(out.waveform, 4.9999f, 1e-3f));

    // Pass-through flags
    mj_voice_in_t in5 = {0};
    in5.coupling = 1; in5.quantization = 1;
    mj_voice_map(&in5, &out);
    CHECK(out.frequency_couple == 1 && out.ratio_lock == 1);
}

static void test_voice_process(void) {
    ps_t p;
    pulsar_init(&p, 48000.0f);
    mj_voice_in_t in = {0};
    in.pitch_param = 16.0f/26.0f;     // C4
    in.density_param = 0.5f;
    mj_voice_out_t o = {0};
    // Run a block of samples; output must stay finite and bounded.
    for (int i = 0; i < 4096; i++) {
        o = mj_voice_process(&p, &in, 0);
        CHECK(isfinite(o.pulse) && isfinite(o.trigger));
        CHECK(o.pulse >= -2.0f && o.pulse <= 2.0f);
    }
    CHECK(isfinite(o.mod_rate));
}

static void test_sync_trigger(void) {
    mj_sync_state_t st = {0};
    CHECK(mj_sync_trigger(&st, 0.0f) == 0);   // below high threshold
    CHECK(mj_sync_trigger(&st, 0.9f) == 0);   // still below
    CHECK(mj_sync_trigger(&st, 1.0f) == 1);   // rising edge fires once
    CHECK(mj_sync_trigger(&st, 5.0f) == 0);   // held high: no re-fire (the bug)
    CHECK(mj_sync_trigger(&st, 1.0f) == 0);   // still held high
    CHECK(mj_sync_trigger(&st, 0.5f) == 0);   // between thresholds: holds, no fire
    CHECK(mj_sync_trigger(&st, 0.0f) == 0);   // drops to low threshold -> reset
    CHECK(mj_sync_trigger(&st, 1.0f) == 1);   // next rising edge fires again
    mj_sync_state_t st2 = {0};                // state is per-voice
    CHECK(mj_sync_trigger(&st2, 5.0f) == 1);  // first sample already high -> fires
}

int main(void) {
    test_sync_trigger();
    test_voice_map();
    test_voice_process();
    if (failures) { printf("%d failures\n", failures); return 1; }
    printf("all tests passed\n");
    return 0;
}
