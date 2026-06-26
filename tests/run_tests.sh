#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c11 -O0 -g -Wall \
  -Icores/pulsar -Icores/phasor -Icores/oscillator -Icores/oscillator/lut -Icores/mom_jeans \
  tests/test_mom_jeans_voice.c \
  cores/mom_jeans/mom_jeans_voice.c \
  cores/pulsar/pulsar.c cores/pulsar/pulsar_oscillator.c cores/pulsar/biquad.c \
  cores/pulsar/minblep.c cores/pulsar/minblep_lut.c cores/pulsar/pulsar_lut.c \
  cores/pulsar/antialiased_oscillator.c \
  cores/oscillator/oscillator.c cores/phasor/phasor.c \
  cores/oscillator/lut/lut_saw_uint8.c cores/oscillator/lut/lut_saw_uint16.c \
  cores/oscillator/lut/lut_sin_uint8.c cores/oscillator/lut/lut_sin_uint16.c \
  cores/oscillator/lut/lut_square_uint8.c cores/oscillator/lut/lut_square_uint16.c \
  cores/oscillator/lut/lut_tri_uint8.c cores/oscillator/lut/lut_tri_uint16.c \
  -lm -o /tmp/mj_core_tests
/tmp/mj_core_tests
