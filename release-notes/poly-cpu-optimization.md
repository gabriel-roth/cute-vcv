# Mom Jeans — polyphony CPU optimization

Branch: `poly-cpu-optimization`

Polyphonic Mom Jeans worked on MetaModule hardware but 4 voices @ 48 kHz with a
64-sample block exceeded the CPU budget. These changes cut the per-sample math
cost — concentrated on the expensive libm transcendental calls that dominate on
the MetaModule's Cortex-A7 software math library — while keeping the audio output
effectively unchanged.

## What the hardware actually is

The MetaModule target is a **Cortex-A7** (STM32MP15x, `arch_mp15xa7.cmake`), not a
Cortex-M7. Two consequences:

- `ARM_MATH_CM7` is never defined, so the DSP cores were silently falling back to
  libm `sinf`/`tanhf` instead of the fast LUT path the hardware was meant to use.
- The build is already `-O3` (RelWithDebInfo, set by the SDK) — this was *not* an
  accidental `-O0`. The A7 has hardware VFP/NEON for add/mul/div/sqrt, but
  `sinf`/`cosf`/`powf`/`tanhf`/`exp2f` are software routines costing tens to
  hundreds of cycles each. Removing them is the lever.

`arm_sin_f32` (a 512-entry LUT sine with linear interpolation) is plain C in
`pulsar_lut.c`, compiled into every build, and is exactly the path the hardware
`#ifdef ARM_MATH_CM7` would have selected — so switching to it is
character-preserving relative to the intended hardware DSP path.

## Changes (each verified against a golden-master capture)

1. **Integer/half-integer `powf` → multiplication.** `powf(x, 2.0f)` → `x*x`
   (8 sites in `_calc_mod_ratio`/`_calc_mod_depth`, all per-sample in
   `pulsar_configure`), and `powf(density, 1.5f)` → `density*sqrtf(density)`
   (`sqrtf` is a single VFP instruction). **Bit-identical output.**

2. **Base-2 powers → `exp2f`.** Pitch `powf(2.f, x)` → `exp2f(x)` (per sample),
   and per-grain `powf(1.15f, y)` → `exp2f(y*log2(1.15))`. Inaudible
   (RMS diff −111 dB).

3. **libm `sinf`/`cosf` → `arm_sin_f32` LUT** in the hot path: the sinc waveform,
   the `contained_square` window (`cos(x)=sin(x+π/2)`), the grain-trigger LFO, and
   the per-sample cadence-LED LFO (which was computed every sample on every
   channel just to drive one LED). RMS diff −85 dB.

4. **Per-sample `tanhf` output saturator → order-7 Padé `fast_tanh`.** Accurate to
   ~1e-4 vs libm `tanhf` for |x| ≤ 4.9, clamped to the ±1 asymptote beyond. A few
   mults + one divide instead of an exp-based software call. The saturation curve
   is preserved closely enough that it contributes essentially nothing to the
   measured deviation.

**Cumulative output deviation vs the pre-optimization build:** max |diff| 6.9e-4,
RMS −85 dB across a 17-scenario sweep covering all six waveforms, density/torque/
cadence extremes, coupling/quantization, extended pitch range, and the sync path.

## Verification

- `tests/run_tests.sh` — existing unit tests, PASS.
- `tests/perf_regression.c` + `tests/run_perf.sh` — new golden-master + benchmark
  harness:
  - `./tests/run_perf.sh gen <file>` captures golden output.
  - `./tests/run_perf.sh check <file> [tol]` compares current output to golden.
  - `./tests/run_perf.sh bench [reps]` times the sweep.
- Both the MetaModule `.mmplugin` and the VCV Rack `plugin.dylib` build clean.

**Host-benchmark caveat:** the host is x86 with a fast/optimized libm, so it
*cannot* show the transcendental savings (its `sinf`/`tanhf` are hardware-fast and
`powf(x,2)` is folded at `-O3`). The win lands on the A7's software libm. Host
timing stayed flat (~35 ns/sample) throughout, as expected; the relevant proof
here is the golden-master equivalence plus the count of transcendentals removed
from the per-sample path. Real confirmation is the on-hardware CPU meter.

## Deliberately not done (scope / character protection)

- Waveform `fmodf` (per active voice): moderate cost, but changing modulo sign
  semantics risks the waveform shape.
- Moving `pulsar_configure` to block-rate: the largest structural win, but it
  would downsample audio-rate CV on density/torque/cadence/shape and change the
  sound. The `powf` removal already eliminates most of configure's per-sample
  cost.
- Compiler flags: left to the SDK (`-O3` already on; `-ffast-math` not enabled).

## Artifact

`metamodule/metamodule-plugins/CuteLab-v2.0-perf-1.mmplugin` — install and check
the CPU meter with 4 voices. (The pre-optimization poly build remains as
`CuteLab-v2.0.mmplugin` for A/B comparison.)
