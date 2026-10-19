// Slate — stereo gate / expander engine: five styles (four downward expanders and an upward one),
// threshold, ratio, range, knee, attack, hold, release, look-ahead, stereo or mid/side with linking,
// external sidechain with 48 dB/oct detector filters and audition, MIDI note triggering, separate
// wet and dry levels, and 2x/4x linear-phase oversampling of the gain stage.
// Clean-room implementation from public, generic techniques (log-domain gain computer with quadratic soft
// knee and hysteresis, peak and RMS detectors, branching attack/hold/release smoothing, TPT state-variable
// filters, windowed-sinc halfband FIR oversampling). No third-party product code, assets or disassembly
// were used. License: MIT (see LICENSE).

#pragma once
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <vector>

namespace slate {

static constexpr float kPi = 3.14159265358979f;
inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
inline float dbToGain(float db) { return std::exp(db * 0.11512925465f); }
inline float gainToDb(float g) { return 8.68588963807f * std::log(std::max(g, 1e-9f)); }

// ------------------------------------------------------------------ params (append-only)
enum Param : uint32_t {
    kStyle, kThreshold, kRatio, kRange, kAttack, kHold, kRelease, kKnee, kLookahead, kOversampling,
    kChannelMode, kLink, kSidechain, kScGain, kScHpOn, kScHpFreq, kScLpOn, kScLpFreq, kScListen,
    kWet, kDry, kPan, kMidiTrigger, kExpert, kDisplayRange, kMidiOn,
    // outputs (read-only, for the UI)
    kOutLearn, kOutInL, kOutInR, kOutOutL, kOutOutR, kOutGr,
    kParamCount
};
enum Style { kClassic, kClean, kVocal, kGuitar, kUpward, kNumStyles };
static constexpr float kDryOff = -60.f;              // the dry level's bottom position means "off"
static constexpr float kMaxBoost = 36.f;             // upward expansion never boosts more than this

// per-style detector and behaviour
struct StyleDef { bool rms; float detMs; float hyst; float kneeExtra; bool upward; };
inline StyleDef styleDef(int s) {
    static const StyleDef d[kNumStyles] = {
        { true,  4.f, 0.f, 2.f, false },     // Classic: smooth RMS expander, release slows after long openings
        { false, 8.f, 0.f, 0.f, false },     // Clean: fast peak detector, timing exactly as set
        { true, 12.f, 4.f, 3.f, false },     // Vocal: slower RMS, 4 dB hysteresis so breaths don't chatter
        { false, 5.f, 8.f, 0.f, false },     // Guitar: peak detector, wide hysteresis, closes quickly as notes die
        { true,  4.f, 0.f, 2.f, true } };    // Upward: boosts what rises above the threshold
    return d[s < 0 ? 0 : (s >= kNumStyles ? kNumStyles - 1 : s)];
}

// soft-knee "distance past a point"
inline float pastKnee(float d, float K) {
    if (K < 0.01f) return d > 0 ? d : 0.f;
    if (d <= -0.5f * K) return 0.f;
    if (d >= 0.5f * K) return d;
    const float t = d + 0.5f * K; return t * t / (2.f * K);
}
// static gain (dB) for a detector level: negative = attenuation (downward), positive = boost (upward)
inline float staticGain(float lvl, float thr, float ratio, float range, float knee, bool upward) {
    ratio = std::max(ratio, 1.f); range = std::max(range, 0.f);
    if (upward) return std::min(pastKnee(lvl - thr, knee) * (ratio - 1.f), std::min(range, kMaxBoost));
    return -std::min(pastKnee(thr - lvl, knee) * (ratio - 1.f), range);
}

// ------------------------------------------------------------------ filters
// 48 dB/oct Butterworth: four cascaded TPT SVF stages
struct Filter48 {
    float ic1[4] = {}, ic2[4] = {};
    void reset() { for (int i = 0; i < 4; ++i) ic1[i] = ic2[i] = 0; }
    inline float process(float x, float g, bool highpass) {
        static const float k[4] = { 1.96157056f, 1.66293922f, 1.11114047f, 0.39018064f };
        for (int i = 0; i < 4; ++i) {
            const float a1 = 1.f / (1.f + g * (g + k[i]));
            const float v3 = x - ic2[i], v1 = a1 * ic1[i] + g * a1 * v3, v2 = ic2[i] + g * v1;
            ic1[i] = 2.f * v1 - ic1[i]; ic2[i] = 2.f * v2 - ic2[i];
            x = highpass ? x - k[i] * v1 - v2 : v2;
        }
        return x;
    }
};

// ------------------------------------------------------------------ halfband oversampling (65 taps, Kaiser)
static constexpr int kHbHalf = 16, kHbC = 2 * kHbHalf;
inline const float* halfbandTaps() {
    static float t[kHbHalf]; static bool done = false;
    if (!done) {
        done = true;
        auto bessel0 = [](double x) { double s = 1, term = 1; for (int k = 1; k < 30; ++k) { term *= (x / (2 * k)) * (x / (2 * k)); s += term; } return s; };
        const double beta = 8.0, N = 2 * kHbC, PI = 3.14159265358979323846; double sum = 0.5;
        for (int j = 0; j < kHbHalf; ++j) {
            const int k = 2 * j + 1; const double sinc = std::sin(PI * k / 2.0) / (PI * k / 2.0);
            const double r = (double)(k + kHbC) / N * 2 - 1, w = bessel0(beta * std::sqrt(std::max(0.0, 1 - r * r))) / bessel0(beta);
            t[j] = (float)(0.5 * sinc * w); sum += 2 * t[j];
        }
        for (int j = 0; j < kHbHalf; ++j) t[j] = (float)(t[j] / sum);
    }
    return t;
}
struct HbUp {
    float buf[128] = {}; int pos = 0;
    void reset() { std::memset(buf, 0, sizeof buf); pos = 0; }
    inline void process(float x, float& y0, float& y1) {
        pos = (pos + 1) & 63; buf[pos] = buf[pos + 64] = x;
        const float* h = halfbandTaps(); const float* b = buf + pos + 64;
        y0 = b[-kHbHalf]; float s = 0;
        for (int j = 0; j < kHbHalf; ++j) s += h[j] * (b[-kHbHalf - j] + b[-kHbHalf + j + 1]);
        y1 = 2.f * s;
    }
};
struct HbDown {
    float buf[256] = {}; int pos = 0;
    void reset() { std::memset(buf, 0, sizeof buf); pos = 0; }
    inline float process(float x0, float x1) {
        pos = (pos + 1) & 127; buf[pos] = buf[pos + 128] = x0;
        pos = (pos + 1) & 127; buf[pos] = buf[pos + 128] = x1;
        const float* h = halfbandTaps(); const float* c = buf + pos + 128 - kHbC - 1;
        float s = 0.5f * c[0];
        for (int j = 0; j < kHbHalf; ++j) s += h[j] * (c[-(2 * j + 1)] + c[2 * j + 1]);
        return s;
    }
};
// 1x, 2x or 4x; latency in base-rate samples 0 / 32 / 48 (half of it in the up-sampler)
struct Oversampler {
    HbUp u1, u2; HbDown d1, d2;
    void reset() { u1.reset(); u2.reset(); d1.reset(); d2.reset(); }
    static int latency(int os) { return os == 1 ? 0 : (os == 2 ? 32 : 48); }
    inline void up(float x, int os, float* y) {
        if (os == 1) { y[0] = x; return; }
        float a, b; u1.process(x, a, b);
        if (os == 2) { y[0] = a; y[1] = b; return; }
        u2.process(a, y[0], y[1]); u2.process(b, y[2], y[3]);
    }
    inline float down(const float* y, int os) {
        if (os == 1) return y[0];
        if (os == 2) return d1.process(y[0], y[1]);
        const float a = d2.process(y[0], y[1]), b = d2.process(y[2], y[3]);
        return d1.process(a, b);
    }
};

// ------------------------------------------------------------------ engine
class Engine {
public:
    float p[kParamCount];
    // level history for the display: one point every 10 ms, written by the audio thread
    static constexpr int kHist = 1024;
    float hIn[kHist], hOut[kHist], hGain[kHist], hDet[kHist];
    struct AtomicIndex {
        std::atomic<uint32_t> v { 0 };
        AtomicIndex() {}
        AtomicIndex(const AtomicIndex& o) : v(o.v.load()) {}
        AtomicIndex& operator=(const AtomicIndex& o) { v.store(o.v.load()); return *this; }
        uint32_t load() const { return v.load(std::memory_order_acquire); }
        void store(uint32_t x) { v.store(x, std::memory_order_release); }
    } histWrite;

    Engine() {
        std::fill(p, p + kParamCount, 0.f);
        p[kStyle] = kClassic; p[kThreshold] = -40; p[kRatio] = 10; p[kRange] = 40; p[kAttack] = 1; p[kHold] = 50; p[kRelease] = 150;
        p[kKnee] = 6; p[kLookahead] = 0; p[kOversampling] = 0; p[kChannelMode] = 0; p[kLink] = 1; p[kSidechain] = 0; p[kScGain] = 0;
        p[kScHpOn] = 0; p[kScHpFreq] = 80; p[kScLpOn] = 0; p[kScLpFreq] = 10000; p[kScListen] = 0; p[kWet] = 0; p[kDry] = kDryOff; p[kPan] = 0;
        p[kMidiTrigger] = 0; p[kExpert] = 0; p[kDisplayRange] = 1; p[kMidiOn] = 1;
        for (int i = 0; i < kHist; ++i) { hIn[i] = hOut[i] = hDet[i] = -120; hGain[i] = 0; }
    }
    void setSampleRate(float sr) {
        fs = sr;
        int n = 1; while (n < (int)(0.010f * sr) + 64 + 8) n <<= 1;
        histN = n; for (auto& h : hist) h.assign(histN, 0.f);
        smooth = 1.f - std::exp(-1.f / (0.015f * sr));
        meterFall = std::exp(-1.f / (0.3f * sr));
        histSamples = std::max(1, (int)(0.01f * sr));
        // start closed (a downward gate shouldn't let a burst of noise through before it has listened)
        const bool up = styleDef((int)std::round(p[kStyle])).upward;
        for (int c = 0; c < 2; ++c) { gdb[c] = up ? 0.f : -clampf(p[kRange], 0.f, 80.f); holdLeft[c] = 0; det[c] = 0; lvlDb[c] = lvlSlow[c] = -150; }
        configure();
        for (int c = 0; c < 2; ++c) { mIn[c] = mOut[c] = 0; }
        mGain = 0;
    }
    void reset() { configure(); }
    uint32_t latency() const { return (uint32_t)(laSamples + Oversampler::latency(os)); }
    float sampleRate() const { return fs; }
    float meterIn(int c) const { return gainToDb(mIn[c]); }
    float meterOut(int c) const { return gainToDb(mOut[c]); }
    float meterGain() const { return mGain; }               // dB, negative = attenuation, positive = boost
    float detectorDb() const { return std::max(lvlDb[0], lvlDb[1]); }
    float gainDb() const { return std::fabs(gdb[0]) >= std::fabs(gdb[1]) ? gdb[0] : gdb[1]; }

    // MIDI triggering: any held note opens the gate
    void noteOn(int note) { if (note >= 0 && note < 128 && !held[note]) { held[note] = true; ++numHeld; } }
    void noteOff(int note) { if (note >= 0 && note < 128 && held[note]) { held[note] = false; --numHeld; } }
    void allNotesOff() { std::fill(held, held + 128, false); numHeld = 0; }

    // sc may be null when the host gives no sidechain
    void process(const float* inL, const float* inR, const float* scL, const float* scR, float* outL, float* outR, uint32_t n) {
        const int wantOs = osOf(p[kOversampling]), wantLa = laOf(p[kLookahead]);
        if ((wantOs != os || wantLa != laSamples) && fadeState == 0) fadeState = 1;
        const int style = (int)clampf(std::round(p[kStyle]), 0, kNumStyles - 1);
        const StyleDef sd = styleDef(style);
        const bool ms = p[kChannelMode] > 0.5f, external = p[kSidechain] > 0.5f && scL && scR, listen = p[kScListen] > 0.5f;
        const bool hpOn = p[kScHpOn] > 0.5f, lpOn = p[kScLpOn] > 0.5f, midiTrig = p[kMidiTrigger] > 0.5f && p[kMidiOn] > 0.5f;
        const float gHp = std::tan(kPi * clampf(p[kScHpFreq], 10.f, 0.45f * fs) / fs), gLp = std::tan(kPi * clampf(p[kScLpFreq], 10.f, 0.45f * fs) / fs);
        const float detC = 1.f - std::exp(-1.f / (sd.detMs * 0.001f * fs));
        const float thr = clampf(p[kThreshold], -80.f, 0.f), ratio = clampf(p[kRatio], 1.f, 100.f), range = clampf(p[kRange], 0.f, 80.f);
        const float knee = clampf(p[kKnee], 0.f, 30.f) + sd.kneeExtra, link = clampf(p[kLink], 0.f, 1.f);
        const float attC = coef(clampf(p[kAttack], 0.005f, 250.f)), relMs = clampf(p[kRelease], 5.f, 5000.f);
        const int holdN = (int)(clampf(p[kHold], 0.f, 500.f) * 0.001f * fs);
        const float tScGain = dbToGain(clampf(p[kScGain], -36.f, 36.f));
        const float tWet = dbToGain(clampf(p[kWet], -36.f, 12.f)), tDry = p[kDry] <= kDryOff + 0.01f ? 0.f : dbToGain(clampf(p[kDry], kDryOff, 12.f));
        const float pan = clampf(p[kPan], -1.f, 1.f), tPanL = pan > 0 ? 1.f - pan : 1.f, tPanR = pan < 0 ? 1.f + pan : 1.f;
        const int upLat = Oversampler::latency(os) / 2, lat = (int)latency();
        const float slowC = 1.f - std::exp(-1.f / (0.5f * fs));
        for (uint32_t i = 0; i < n; ++i) {
            sScGain += (tScGain - sScGain) * smooth; sWet += (tWet - sWet) * smooth; sDry += (tDry - sDry) * smooth;
            sPanL += (tPanL - sPanL) * smooth; sPanR += (tPanR - sPanR) * smooth;
            const float lst = listen ? 1.f : 0.f; sListen += (lst - sListen) * smooth;
            hist[0][hp] = inL[i]; hist[1][hp] = inR[i]; hist[2][hp] = external ? scL[i] : 0.f; hist[3][hp] = external ? scR[i] : 0.f;
            for (int c = 0; c < 2; ++c) mIn[c] = std::max(mIn[c] * meterFall, std::fabs(c ? inR[i] : inL[i]));
            // detector: the (sidechain) signal, earlier than the audio by the look-ahead
            const int di = (hp - upLat) & histMask, base = external ? 2 : 0;
            float d[2] = { hist[base][di], hist[base + 1][di] };
            if (ms) { const float m = 0.5f * (d[0] + d[1]), s = 0.5f * (d[0] - d[1]); d[0] = m; d[1] = s; }
            float target[2];
            for (int c = 0; c < 2; ++c) {
                float s = d[c] * sScGain;
                if (hpOn) s = hpf[c].process(s, gHp, true);
                if (lpOn) s = lpf[c].process(s, gLp, false);
                scOut[c] = s;
                if (sd.rms) { det[c] += (s * s - det[c]) * detC; lvlDb[c] = 10.f * std::log10(std::max(2.f * det[c], 1e-15f)); }   // RMS, a sine reads its peak
                else { const float a = std::fabs(s); det[c] = a > det[c] ? a : det[c] + (a - det[c]) * detC; lvlDb[c] = gainToDb(det[c]); }
            }
            if (link > 0) { const float mx = std::max(lvlDb[0], lvlDb[1]); for (int c = 0; c < 2; ++c) lvlDb[c] += (mx - lvlDb[c]) * link; }
            for (int c = 0; c < 2; ++c) {
                // hysteresis: once open, the gate closes only this far below the threshold
                const float t = sd.upward ? thr : thr - (open[c] ? sd.hyst : 0.f);
                target[c] = (midiTrig && numHeld > 0) ? (sd.upward ? gdb[c] : 0.f) : staticGain(lvlDb[c], t, ratio, range, knee, sd.upward);
                if (!sd.upward) open[c] = target[c] > -1.f;
            }
            for (int c = 0; c < 2; ++c) {
                float& g = gdb[c];
                // "opening" moves towards more gain: attack (linear in level). Closing: hold first, then release (linear in dB).
                const bool opening = target[c] > g;
                if (opening) { float gl = dbToGain(g); gl += (dbToGain(target[c]) - gl) * attC; g = gainToDb(gl); holdLeft[c] = holdN; }   // open as a smooth fade in level
                else if (holdLeft[c] > 0) { --holdLeft[c]; }
                else {
                    float rel = relMs;
                    if (style == kClassic) rel *= 1.f + clampf(openTime[c] * 2.f, 0.f, 1.f);                 // long openings close more gently
                    if (style == kGuitar && lvlDb[c] < lvlSlow[c] - 6.f) rel *= 0.5f;                        // notes dying fast: close quicker
                    if (style == kVocal) rel *= 1.2f;
                    g += (target[c] - g) * coef(rel);
                }
                lvlSlow[c] += (lvlDb[c] - lvlSlow[c]) * slowC;
                openTime[c] = g > -1.f ? openTime[c] + 1.f / fs : 0.f;
                if (!(std::fabs(g) < 200.f)) g = 0;
            }
            const float gNow[2] = { dbToGain(gdb[0]), dbToGain(gdb[1]) };
            // audio: delayed by the look-ahead, gain applied at the oversampled rate (interpolated)
            const int ai = (hp - laSamples) & histMask;
            float a[2] = { hist[0][ai], hist[1][ai] };
            if (ms) { const float m = 0.5f * (a[0] + a[1]), s = 0.5f * (a[0] - a[1]); a[0] = m; a[1] = s; }
            float w[2];
            for (int c = 0; c < 2; ++c) {
                float up[4], y[4];
                ovs[c].up(a[c], os, up);
                for (int k = 0; k < os; ++k) { const float t = (k + 1.f) / os; y[k] = up[k] * (gPrev[c] + (gNow[c] - gPrev[c]) * t); }
                w[c] = ovs[c].down(y, os);
                gPrev[c] = gNow[c];
            }
            if (ms) { const float l = w[0] + w[1], r = w[0] - w[1]; w[0] = l; w[1] = r; }
            // dry, aligned with the latency; wet level and pan; sidechain audition
            const int dri = (hp - lat) & histMask;
            const float dry[2] = { hist[0][dri], hist[1][dri] };
            float o[2];
            for (int c = 0; c < 2; ++c) {
                float v = w[c] * sWet * (c ? sPanR : sPanL) + dry[c] * sDry;
                const float aud = ms ? (c ? scOut[0] - scOut[1] : scOut[0] + scOut[1]) : scOut[c];
                v += (aud - v) * sListen;
                v *= fadeGain;
                if (!(std::fabs(v) < 64.f)) v = std::isfinite(v) ? std::copysign(64.f, v) : 0.f;
                o[c] = v; mOut[c] = std::max(mOut[c] * meterFall, std::fabs(v));
            }
            outL[i] = o[0]; outR[i] = o[1];
            const float gd = gainDb();
            mGain = std::fabs(gd) > std::fabs(mGain) ? gd : mGain * 0.9995f;
            // display history
            hPkIn = std::max(hPkIn, std::max(std::fabs(inL[i]), std::fabs(inR[i])));
            hPkOut = std::max(hPkOut, std::max(std::fabs(o[0]), std::fabs(o[1])));
            hDetMax = std::max(hDetMax, detectorDb());
            if (std::fabs(gd) > std::fabs(hGainEx)) hGainEx = gd;
            if (++hCount >= histSamples) {
                const uint32_t wi = histWrite.load() % kHist;
                hIn[wi] = gainToDb(hPkIn); hOut[wi] = gainToDb(hPkOut); hGain[wi] = hGainEx; hDet[wi] = hDetMax;
                histWrite.store(histWrite.load() + 1);
                hPkIn = hPkOut = 0; hGainEx = 0; hDetMax = -200; hCount = 0;
            }
            hp = (hp + 1) & histMask;
            if (fadeState == 1) { fadeGain -= 1.f / 96.f; if (fadeGain <= 0) { configure(); fadeGain = 0; fadeState = 2; } }
            else if (fadeState == 2) { fadeGain += 1.f / 96.f; if (fadeGain >= 1) { fadeGain = 1; fadeState = 0; } }
        }
        for (int c = 0; c < 2; ++c) if (!(std::fabs(det[c]) < 1e6f)) { det[c] = 0; hpf[c].reset(); lpf[c].reset(); }
    }

private:
    float coef(float ms) const { return 1.f - std::exp(-1.f / (std::max(ms, 0.001f) * 0.001f * fs)); }
    static int osOf(float v) { const int k = (int)clampf(std::round(v), 0, 2); return k == 0 ? 1 : (k == 1 ? 2 : 4); }
    int laOf(float ms) const { return (int)std::round(clampf(ms, 0.f, 10.f) * 0.001f * fs); }
    void configure() {
        os = osOf(p[kOversampling]); laSamples = laOf(p[kLookahead]);
        histMask = histN - 1;
        for (int c = 0; c < 2; ++c) { ovs[c].reset(); gPrev[c] = dbToGain(gdb[c]); }
        sScGain = dbToGain(p[kScGain]); sWet = dbToGain(p[kWet]); sDry = p[kDry] <= kDryOff + 0.01f ? 0.f : dbToGain(p[kDry]);
        const float pan = clampf(p[kPan], -1.f, 1.f); sPanL = pan > 0 ? 1.f - pan : 1.f; sPanR = pan < 0 ? 1.f + pan : 1.f;
        sListen = p[kScListen] > 0.5f ? 1.f : 0.f;
        fadeState = 0; fadeGain = 1;
    }

    float fs = 48000.f, smooth = 0.01f, meterFall = 0.999f;
    int os = 1, laSamples = 0, fadeState = 0; float fadeGain = 1;
    std::vector<float> hist[4]; int histN = 1, histMask = 0, hp = 0;
    Oversampler ovs[2];
    Filter48 hpf[2], lpf[2];
    float det[2] = {}, lvlDb[2] = { -150, -150 }, lvlSlow[2] = { -150, -150 }, gdb[2] = {}, gPrev[2] = { 1, 1 }, openTime[2] = {}, scOut[2] = {};
    int holdLeft[2] = {}; bool open[2] = {};
    bool held[128] = {}; int numHeld = 0;
    float sScGain = 1, sWet = 1, sDry = 0, sPanL = 1, sPanR = 1, sListen = 0;
    float mIn[2] = {}, mOut[2] = {}, mGain = 0;
    float hPkIn = 0, hPkOut = 0, hGainEx = 0, hDetMax = -200; int hCount = 0, histSamples = 480;
};

} // namespace slate
