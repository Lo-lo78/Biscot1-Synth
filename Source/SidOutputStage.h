#pragma once
#include <algorithm>
#include <cmath>

namespace biscot {
// Exact port of the JSFX v2.38 final amplifier, ~16kHz external RC lowpass
// and 1.6Hz highpass/DC blocker. Keep the MASTER VOLUME after process().
// This reproduces the FINAL stage only, not the upstream 6581/8580 filter DSP.
struct SidOutputStage final {
    double lowpass = 0.0;
    double previousLowpass = 0.0;
    double previousHighpass = 0.0;
    double lowpassAlpha = 0.0;
    double highpassR = 0.0;
    static constexpr double pi = 3.1415926535897932384626433832795;

    static double softclip(double x) noexcept {
        return x / (1.0 + std::abs(x)); // JSFX softclip, NOT generic VST saturate
    }
    static double amplifier(double x, bool model6581) noexcept {
        const double bias = model6581 ? 0.012 : 0.003;
        const double posDrive = model6581 ? 1.10 : 1.025;
        const double negDrive = model6581 ? 0.96 : 0.99;
        const double wet = model6581 ? 0.085 : 0.022;
        const double dry = 1.0 - wet;
        const double makeup = model6581 ? 1.022 : 1.004;
        const double z = x + bias;
        const double drive = z >= 0.0 ? posDrive : negDrive;
        const double clipped = softclip(z * drive) / drive;
        return (z * dry + clipped * wet) * makeup;
    }
    void prepare(double sampleRate) noexcept {
        const double sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        lowpassAlpha = 1.0 - std::exp(-2.0*pi*std::min(16000.0,sr*0.45)/sr);
        highpassR = std::exp(-2.0*pi*1.6/sr);
        reset();
    }
    void reset() noexcept { lowpass = previousLowpass = previousHighpass = 0.0; }
    double process(double input, bool model6581) noexcept {
        const double amp = amplifier(input, model6581);
        lowpass += (amp-lowpass) * lowpassAlpha;
        const double output = lowpass-previousLowpass+highpassR*previousHighpass;
        previousLowpass = lowpass;
        previousHighpass = output;
        return output; // JSFX multiplies by master_gain *AFTER* the DC blocker
    }
};
} // namespace biscot
