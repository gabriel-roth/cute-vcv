#include "CoreModules/SmartCoreProcessorPoly.hh"
#include "CoreModules/register_module.hh"
#include "mom_jeans_info.hh"
#include <algorithm>
extern "C" {
#include "mom_jeans_voice.h"
}
using namespace MetaModule;

class MomJeansCore : public SmartCoreProcessorPoly<MomJeansInfo> {
    using Info = MomJeansInfo;
    using enum Info::Elem;

    ps_t pulsars[CoreProcessor::MaxPolyChannels];
    float sampleRate = 48000.f;

    static uint8_t isDown(LatchingButton::State_t s) {
        return s == LatchingButton::State_t::DOWN ? 1 : 0;
    }

public:
    MomJeansCore() {
        for (auto &p : pulsars) pulsar_init(&p, sampleRate);
    }

    void set_samplerate(float sr) override {
        sampleRate = sr;
        for (auto &p : pulsars) pulsar_init(&p, sr);
    }

    void update() override {
        // Max channel count across all inputs, capped at MaxPolyChannels.
        unsigned maxCh = 1;
        auto bump = [&](unsigned n){ if (n > maxCh) maxCh = n; };
        bump(numChannels<DensityIn>());  bump(numChannels<ShapeIn>());
        bump(numChannels<TorqueIn>());   bump(numChannels<CadenceIn>());
        bump(numChannels<FmIndexIn>());  bump(numChannels<LinearFmIn>());
        bump(numChannels<VOctIn>());     bump(numChannels<SyncIn>());
        if (maxCh > CoreProcessor::MaxPolyChannels) maxCh = CoreProcessor::MaxPolyChannels;

        setChannels<Out>(maxCh);
        setChannels<TriggerOut>(maxCh);

        // Params. getState<Knob> returns float in 0..1 (knobs declared 0..1, so
        // these are already normalized — do NOT divide again). getState<LatchingButton>
        // returns LatchingButton::State_t (UP/DOWN), so use isDown().
        float pitch_param   = getState<PitchKnob>();
        float density_param = getState<DensityKnob>();
        float torque_param  = getState<TorqueKnob>();
        float cadence_param = getState<CadenceKnob>();
        float shape_param   = getState<ShapeKnob>();
        uint8_t coupling     = isDown(getState<CouplingButton>());
        uint8_t quantization = isDown(getState<QuantizationButton>());
        uint8_t pitch_mode   = isDown(getState<PitchModeButton>());

        bool fm_connected = numChannels<FmIndexIn>() > 0;

        for (unsigned c = 0; c < maxCh; c++) {
            mj_voice_in_t in = {};
            in.pitch_param = pitch_param;
            in.density_param = density_param;
            in.torque_param = torque_param;
            in.cadence_param = cadence_param;
            in.shape_param = shape_param;
            in.coupling = coupling;
            in.quantization = quantization;
            in.pitch_mode = pitch_mode;
            in.density_cv = getInput<DensityIn>(c).value_or(0.f);
            in.shape_cv   = getInput<ShapeIn>(c).value_or(0.f);
            in.torque_cv  = getInput<TorqueIn>(c).value_or(0.f);
            in.cadence_cv = getInput<CadenceIn>(c).value_or(0.f);
            in.fm_index_cv = getInput<FmIndexIn>(c).value_or(0.f);
            in.fm_index_connected = fm_connected ? 1 : 0;
            in.linear_fm_cv = getInput<LinearFmIn>(c).value_or(0.f);
            in.v_oct_cv = getInput<VOctIn>(c).value_or(0.f);

            uint8_t sync_gate = mj_sync_gate(getInput<SyncIn>(c).value_or(0.f));
            mj_voice_out_t o = mj_voice_process(&pulsars[c], &in, sync_gate);

            setOutput<TriggerOut>(o.trigger * 10.0f, c);
            setOutput<Out>(o.pulse * 10.0f, c);

            if (c == 0) {
                float b = (o.mod_rate < 15.0f)
                    ? std::clamp(o.internal_lfo, 0.0f, 1.0f)
                    : std::clamp((o.mod_rate - 15.0f) / (50.0f - 15.0f) * 0.5f + 0.5f, 0.0f, 1.0f);
                setLED<CadenceLed>(b);
            }
        }
    }
};

// init() must see the complete MomJeansCore type (register_module calls
// std::make_unique<MomJeansCore>), so it lives in this translation unit.
extern "C" void init() {
    register_module<MomJeansCore, MomJeansInfo>("CuteLab");
}
