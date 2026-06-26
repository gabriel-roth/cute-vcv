#!/usr/bin/env bash
# Build the perf_regression harness with the same -O3 the MetaModule target uses
# and forward args to it. Usage: run_perf.sh gen|check|bench ...
set -euo pipefail
cd "$(dirname "$0")/.."
BIN=/tmp/mj_perf
cc -std=c11 -O3 -w \
  -Icores/pulsar -Icores/phasor -Icores/oscillator -Icores/oscillator/lut -Icores/mom_jeans \
  tests/perf_regression.c \
  cores/mom_jeans/mom_jeans_voice.c \
  cores/pulsar/pulsar.c cores/pulsar/pulsar_oscillator.c cores/pulsar/biquad.c \
  cores/pulsar/minblep.c cores/pulsar/minblep_lut.c cores/pulsar/pulsar_lut.c \
  cores/pulsar/antialiased_oscillator.c \
  cores/oscillator/oscillator.c cores/phasor/phasor.c \
  cores/oscillator/lut/lut_saw_uint8.c cores/oscillator/lut/lut_saw_uint16.c \
  cores/oscillator/lut/lut_sin_uint8.c cores/oscillator/lut/lut_sin_uint16.c \
  cores/oscillator/lut/lut_square_uint8.c cores/oscillator/lut/lut_square_uint16.c \
  cores/oscillator/lut/lut_tri_uint8.c cores/oscillator/lut/lut_tri_uint16.c \
  -lm -o "$BIN"
"$BIN" "$@"
