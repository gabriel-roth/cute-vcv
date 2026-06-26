#pragma once
#include "CoreModules/elements/element_info.hh"

using namespace MetaModule;

// Helper element subtypes. Each slices into its base Element variant alternative
// (Knob / LatchingButton / JackInput / JackOutput / MonoLight), carrying the
// configured base fields. Pattern follows the SDK docs' DaviesLargeKnob example.
struct CuteKnob : Knob {
    constexpr CuteKnob() = default;
    constexpr CuteKnob(BaseElement b, float def = 0.f,
                       std::string_view img = "4ms/comp/knob_x.png")
        : Knob{{{b, img}, def, 0.f, 1.f}} {}
};
struct CuteButton : LatchingButton {
    constexpr CuteButton() = default;
    constexpr CuteButton(BaseElement b)
        : LatchingButton{{{b, "4ms/comp/button_x.png"}}} {}
};
struct CuteJackIn : JackInput {
    constexpr CuteJackIn() = default;
    constexpr CuteJackIn(BaseElement b)
        : JackInput{{{b, "4ms/comp/jack_x.png"}}} {}
};
struct CuteJackOut : JackOutput {
    constexpr CuteJackOut() = default;
    constexpr CuteJackOut(BaseElement b)
        : JackOutput{{{b, "4ms/comp/jack_x.png"}}} {}
};
struct CuteLight : MonoLight {
    constexpr CuteLight() = default;
    constexpr CuteLight(BaseElement b)
        : MonoLight{{{b, "4ms/comp/led_x.png"}}} {}
};

struct MomJeansInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"mom-jeans"};
    static constexpr std::string_view description{"Pulsar synthesis oscillator"};
    static constexpr uint32_t width_hp = 8;   // must be non-zero; ~40mm fits the panel
    static constexpr std::string_view png_filename{"CuteLab/mom-jeans.png"};

    enum class Elem {
        // Params: knobs
        PitchKnob, DensityKnob, TorqueKnob, CadenceKnob, ShapeKnob,
        // Params: latching toggles
        PitchModeButton, CouplingButton, QuantizationButton,
        // Inputs
        DensityIn, ShapeIn, TorqueIn, CadenceIn, FmIndexIn, LinearFmIn, VOctIn, SyncIn,
        // Outputs
        Out, TriggerOut,
        // Lights
        CadenceLed,
    };

    static constexpr std::array<Element, 19> Elements{{
        CuteKnob{{10.329f, 19.253f, Coords::Center, "Pitch"},   16.0f/26.0f},
        CuteKnob{{31.296f, 22.843f, Coords::Center, "Density"}, 0.0f, "4ms/comp/knob_large_x.png"},
        CuteKnob{{31.374f, 44.988f, Coords::Center, "Torque"},  0.0f},
        CuteKnob{{13.963f, 49.862f, Coords::Center, "Cadence"}, 0.0f},
        CuteKnob{{23.967f, 68.655f, Coords::Center, "Shape"},   0.0f},

        CuteButton{{19.477f, 12.053f, Coords::Center, "Range"}},
        CuteButton{{ 8.795f, 62.626f, Coords::Center, "Couple"}},
        CuteButton{{ 8.795f, 73.254f, Coords::Center, "Quantize"}},

        CuteJackIn{{19.785f, 33.45f,  Coords::Center, "Density"}},
        CuteJackIn{{32.793f, 82.255f, Coords::Center, "Shape"}},
        CuteJackIn{{20.821f, 84.253f, Coords::Center, "Torque"}},
        CuteJackIn{{ 8.795f, 86.255f, Coords::Center, "Cadence"}},
        CuteJackIn{{32.793f, 95.253f, Coords::Center, "FM Index"}},
        CuteJackIn{{20.821f, 97.255f, Coords::Center, "Lin FM"}},
        CuteJackIn{{ 8.795f, 99.253f, Coords::Center, "V/Oct"}},
        CuteJackIn{{ 8.795f,112.254f, Coords::Center, "Sync"}},

        CuteJackOut{{32.793f,108.255f, Coords::Center, "Out"}},
        CuteJackOut{{20.821f,110.253f, Coords::Center, "Trigger"}},

        CuteLight{{6.25f, 42.25f, Coords::Center, "Cadence"}},
    }};
};
