// Golden-master regression + benchmark harness for the Mom Jeans voice core.
//
// Exercises mj_voice_process across a sweep of scenarios that cover every
// waveform, density/torque/cadence setting, coupling/quantization combination,
// and the sync path. Used to prove that CPU optimizations preserve the audio
// output (golden-master comparison) and to measure relative speedup.
//
//   perf_regression gen   <golden.bin>           write golden output
//   perf_regression check <golden.bin> [tol]     compare current output to golden
//   perf_regression bench <reps>                  time <reps> sweeps, print ns/sample
//
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mom_jeans_voice.h"

#define SAMPLE_RATE 48000.0f
#define SAMPLES_PER_SCENARIO 2000

// Build a scenario input. Varies the parameters that drive every branch in the
// hot path. Sync is pulsed periodically when sync_period > 0.
typedef struct {
    float pitch_param;
    float density_param;
    float torque_param;
    float cadence_param;
    float shape_param;
    uint8_t coupling;
    uint8_t quantization;
    uint8_t pitch_mode;
    float v_oct_cv;
    float density_cv;
    int   sync_period; // 0 = no sync
} scenario_t;

static const scenario_t scenarios[] = {
    // Sweep all six waveforms at a musical pitch / moderate density.
    {0.62f, 0.7f, 0.3f, 0.2f, 0.00f, 0,0,0, 0.0f, 0.0f, 0},
    {0.62f, 0.7f, 0.3f, 0.2f, 0.20f, 0,0,0, 0.0f, 0.0f, 0},
    {0.62f, 0.7f, 0.3f, 0.2f, 0.40f, 0,0,0, 0.0f, 0.0f, 0},
    {0.62f, 0.7f, 0.3f, 0.2f, 0.60f, 0,0,0, 0.0f, 0.0f, 0},
    {0.62f, 0.7f, 0.3f, 0.2f, 0.80f, 0,0,0, 0.0f, 0.0f, 0},
    {0.62f, 0.7f, 0.3f, 0.2f, 1.00f, 0,0,0, 0.0f, 0.0f, 0},
    // Density extremes.
    {0.62f, 0.0f, 0.5f, 0.5f, 0.30f, 0,0,0, 0.0f, 0.0f, 0},
    {0.62f, 1.0f, 0.5f, 0.5f, 0.30f, 0,0,0, 0.0f, 0.0f, 0},
    // High torque (mod depth) + cadence with coupling on/off.
    {0.55f, 0.6f, 0.9f, 0.7f, 0.50f, 1,0,0, 0.0f, 0.0f, 0},
    {0.55f, 0.6f, 0.9f, 0.7f, 0.50f, 0,0,0, 0.0f, 0.0f, 0},
    // Quantization (ratio lock) on, coupled and uncoupled.
    {0.50f, 0.5f, 0.4f, 0.6f, 0.45f, 1,1,0, 0.0f, 0.0f, 0},
    {0.50f, 0.5f, 0.4f, 0.6f, 0.45f, 0,1,0, 0.0f, 0.0f, 0},
    // Low pitch, extended range.
    {0.20f, 0.5f, 0.3f, 0.3f, 0.10f, 0,0,1, 0.0f, 0.0f, 0},
    // High pitch via V/oct.
    {0.62f, 0.5f, 0.3f, 0.3f, 0.35f, 0,0,1, 2.0f, 0.0f, 0},
    // With density CV applied.
    {0.55f, 0.5f, 0.6f, 0.4f, 0.55f, 0,0,0, 0.0f, 3.0f, 0},
    // Sync path exercised at a couple of rates.
    {0.62f, 0.6f, 0.4f, 0.3f, 0.30f, 0,0,0, 0.0f, 0.0f, 240},
    {0.45f, 0.6f, 0.4f, 0.3f, 0.70f, 1,0,0, 0.0f, 0.0f, 137},
};
#define N_SCENARIOS ((int)(sizeof(scenarios)/sizeof(scenarios[0])))
#define TOTAL_SAMPLES (N_SCENARIOS * SAMPLES_PER_SCENARIO)

static void fill_input(mj_voice_in_t *in, const scenario_t *s) {
    memset(in, 0, sizeof(*in));
    in->pitch_param   = s->pitch_param;
    in->density_param = s->density_param;
    in->torque_param  = s->torque_param;
    in->cadence_param = s->cadence_param;
    in->shape_param   = s->shape_param;
    in->coupling      = s->coupling;
    in->quantization  = s->quantization;
    in->pitch_mode    = s->pitch_mode;
    in->v_oct_cv      = s->v_oct_cv;
    in->density_cv    = s->density_cv;
}

// Run the full sweep, writing one float (pulse output) per sample into out[].
static void run_sweep(float *out) {
    int idx = 0;
    for (int sc = 0; sc < N_SCENARIOS; sc++) {
        const scenario_t *s = &scenarios[sc];
        ps_t p;
        pulsar_init(&p, SAMPLE_RATE);
        mj_voice_in_t in;
        fill_input(&in, s);
        for (int n = 0; n < SAMPLES_PER_SCENARIO; n++) {
            uint8_t sync = 0;
            if (s->sync_period > 0 && (n % s->sync_period) == 0 && n > 0) {
                sync = 1;
            }
            mj_voice_out_t o = mj_voice_process(&p, &in, sync);
            out[idx++] = o.pulse;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s gen|check|bench ...\n", argv[0]);
        return 2;
    }

    if (strcmp(argv[1], "bench") == 0) {
        int reps = argc > 2 ? atoi(argv[2]) : 200;
        float *buf = malloc(sizeof(float) * TOTAL_SAMPLES);
        // warm up
        run_sweep(buf);
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int r = 0; r < reps; r++) run_sweep(buf);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
        double total_samples = (double)reps * TOTAL_SAMPLES;
        printf("bench: %d reps, %.4f s, %.2f ns/sample\n",
               reps, secs, secs / total_samples * 1e9);
        free(buf);
        return 0;
    }

    if (strcmp(argv[1], "gen") == 0 && argc >= 3) {
        float *buf = malloc(sizeof(float) * TOTAL_SAMPLES);
        run_sweep(buf);
        FILE *f = fopen(argv[2], "wb");
        if (!f) { perror("fopen"); return 1; }
        fwrite(buf, sizeof(float), TOTAL_SAMPLES, f);
        fclose(f);
        printf("gen: wrote %d samples to %s\n", TOTAL_SAMPLES, argv[2]);
        free(buf);
        return 0;
    }

    if (strcmp(argv[1], "check") == 0 && argc >= 3) {
        float tol = argc > 3 ? (float)atof(argv[3]) : 1e-4f;
        float *golden = malloc(sizeof(float) * TOTAL_SAMPLES);
        FILE *f = fopen(argv[2], "rb");
        if (!f) { perror("fopen"); return 1; }
        size_t rd = fread(golden, sizeof(float), TOTAL_SAMPLES, f);
        fclose(f);
        if (rd != (size_t)TOTAL_SAMPLES) {
            fprintf(stderr, "golden size mismatch: %zu != %d\n", rd, TOTAL_SAMPLES);
            return 1;
        }
        float *cur = malloc(sizeof(float) * TOTAL_SAMPLES);
        run_sweep(cur);
        double sumsq = 0.0, sumsq_sig = 0.0;
        float maxabs = 0.0f;
        int worst = -1;
        for (int i = 0; i < TOTAL_SAMPLES; i++) {
            float d = cur[i] - golden[i];
            float ad = fabsf(d);
            if (ad > maxabs) { maxabs = ad; worst = i; }
            sumsq += (double)d * d;
            sumsq_sig += (double)golden[i] * golden[i];
        }
        double rms = sqrt(sumsq / TOTAL_SAMPLES);
        double sig_rms = sqrt(sumsq_sig / TOTAL_SAMPLES);
        printf("check: max|diff|=%.3e (scenario %d sample %d), RMS diff=%.3e, "
               "signal RMS=%.3e, RMS ratio=%.3e\n",
               maxabs, worst >= 0 ? worst / SAMPLES_PER_SCENARIO : -1,
               worst >= 0 ? worst % SAMPLES_PER_SCENARIO : -1,
               rms, sig_rms, sig_rms > 0 ? rms / sig_rms : 0.0);
        free(golden); free(cur);
        if (maxabs > tol) {
            printf("FAIL: max|diff| %.3e exceeds tolerance %.3e\n", maxabs, tol);
            return 1;
        }
        printf("PASS: within tolerance %.3e\n", tol);
        return 0;
    }

    fprintf(stderr, "bad args\n");
    return 2;
}
