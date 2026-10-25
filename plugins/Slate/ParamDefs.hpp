// Parameter metadata shared by the DSP wrapper and the UI. License: MIT.
#pragma once
#include "SlateEngine.hpp"
#include <cstdio>

namespace slate {

static constexpr uint32_t kHintInteger = 1, kHintBoolean = 2, kHintLog = 4, kHintOutput = 8, kHintNoAuto = 16;
struct ParamDef { const char* name; const char* sym; const char* unit; float min, max; uint32_t hints; };

inline const ParamDef& paramDef(uint32_t i) {
    static ParamDef t[kParamCount];
    static bool done = false;
    if (!done) {
        done = true;
        auto set = [](uint32_t i, const char* n, const char* s, const char* u, float lo, float hi, uint32_t h) { t[i] = { n, s, u, lo, hi, h }; };
        const uint32_t I = kHintInteger, B = kHintBoolean | kHintInteger, O = kHintOutput, L = kHintLog;
        set(kStyle,        "Style",              "style",    "",   0, kNumStyles - 1, I);
        set(kThreshold,    "Threshold",          "thresh",   "dB", -80, 0, 0);
        set(kRatio,        "Ratio",              "ratio",    "",   1, 100, L);
        set(kRange,        "Range",              "range",    "dB", 0, 80, 0);
        set(kAttack,       "Attack",             "attack",   "ms", 0.005f, 250, L);
        set(kHold,         "Hold",               "hold",     "ms", 0, 500, 0);
        set(kRelease,      "Release",            "release",  "ms", 5, 5000, L);
        set(kKnee,         "Knee",               "knee",     "dB", 0, 30, 0);
        set(kLookahead,    "Lookahead",          "lookahead","ms", 0, 10, 0);
        set(kOversampling, "Oversampling",       "oversamp", "",   0, 2, I);
        set(kChannelMode,  "Channel Mode",       "chmode",   "",   0, 1, I);
        set(kLink,         "Stereo Link",        "link",     "",   0, 1, 0);
        set(kSidechain,    "Sidechain",          "sc",       "",   0, 1, I);
        set(kScGain,       "Sidechain Level",    "scgain",   "dB", -36, 36, 0);
        set(kScHpOn,       "Sidechain High Pass","schpon",   "",   0, 1, B);
        set(kScHpFreq,     "Sidechain HP Freq",  "schp",     "Hz", 20, 20000, L);
        set(kScLpOn,       "Sidechain Low Pass", "sclpon",   "",   0, 1, B);
        set(kScLpFreq,     "Sidechain LP Freq",  "sclp",     "Hz", 20, 20000, L);
        set(kScListen,     "Sidechain Audition", "sclisten", "",   0, 1, B);
        set(kWet,          "Wet Level",          "wet",      "dB", -36, 12, 0);
        set(kDry,          "Dry Level",          "dry",      "dB", kDryOff, 12, 0);
        set(kPan,          "Wet Pan",            "pan",      "",   -1, 1, 0);
        set(kMidiTrigger,  "MIDI Trigger",       "miditrig", "",   0, 1, B);
        set(kExpert,       "Expert Mode",        "expert",   "",   0, 1, B | kHintNoAuto);
        set(kDisplayRange, "Display Range",      "disprange","",   0, 2, I | kHintNoAuto);
        set(kMidiOn,       "MIDI",               "midion",   "",   0, 1, B);
        set(kOutLearn, "MIDI Learn",     "o_learn", "",   0, 10000000, O);
        set(kOutInL,   "Input Level L",  "o_inl",   "dB", -120, 24, O);
        set(kOutInR,   "Input Level R",  "o_inr",   "dB", -120, 24, O);
        set(kOutOutL,  "Output Level L", "o_outl",  "dB", -120, 24, O);
        set(kOutOutR,  "Output Level R", "o_outr",  "dB", -120, 24, O);
        set(kOutGr,    "Gain Change",    "o_gain",  "dB", -80, 36, O);
    }
    return t[i];
}

inline const char* const* enumLabels(uint32_t p, int& count) {
    static const char* const style[] = { "Classic", "Clean", "Vocal", "Guitar", "Upward", "Ducking" };
    static const char* const os[] = { "Off", "2x", "4x" };
    static const char* const chm[] = { "Stereo", "Mid/Side" };
    static const char* const sc[] = { "Internal", "External" };
    static const char* const range[] = { "30 dB", "60 dB", "90 dB" };
    switch (p) {
    case kStyle: count = kNumStyles; return style;
    case kOversampling: count = 3; return os;
    case kChannelMode: count = 2; return chm;
    case kSidechain: count = 2; return sc;
    case kDisplayRange: count = 3; return range;
    default: count = 0; return nullptr;
    }
}

inline float toNorm(uint32_t p, float v) {
    const ParamDef& d = paramDef(p);
    if (d.hints & kHintLog) return std::log(clampf(v, d.min, d.max) / d.min) / std::log(d.max / d.min);
    return (clampf(v, d.min, d.max) - d.min) / (d.max - d.min);
}
inline float fromNorm(uint32_t p, float n) {
    const ParamDef& d = paramDef(p);
    n = clampf(n, 0.f, 1.f);
    float v = (d.hints & kHintLog) ? d.min * std::pow(d.max / d.min, n) : d.min + n * (d.max - d.min);
    if (d.hints & kHintInteger) v = std::round(v);
    return v;
}
inline float displayRangeDb(float v) { const int k = (int)clampf(std::round(v), 0, 2); return k == 0 ? 30.f : (k == 1 ? 60.f : 90.f); }

} // namespace slate
