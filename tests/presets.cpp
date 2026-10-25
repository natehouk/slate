// Play a drum-and-bass test mix with a noise floor through every factory preset (with a kick on the
// sidechain); report level change (Slow Swell is meant to mute short hits), peak and how far the gate closes between hits.
#include "../plugins/Slate/ParamDefs.hpp"
#include "../plugins/Slate/Presets.hpp"
#include <cstdio>
#include <memory>
#include <vector>
using namespace slate;
int main() {
    const int SR = 48000, N = SR * 4; int bad = 0;
    std::vector<float> l(N), r(N), sc(N); uint32_t s = 3;
    for (int i = 0; i < N; ++i) { s = s * 1664525u + 1013904223u; const float w = (int32_t)s * (1.f / 2147483648.f);
        const float t = std::fmod(i / (float)SR, 0.5f), hit = std::exp(-t * 25.f);
        sc[i] = 0.6f * hit * std::sin(2 * kPi * 60 * t);
        l[i] = 0.4f * hit * w + 0.4f * hit * std::sin(2 * kPi * 70 * i / (float)SR) + 0.003f * w; r[i] = 0.9f * l[i]; }
    double si = 0; for (int i = SR; i < N; ++i) si += l[i] * l[i] + r[i] * r[i];
    for (uint32_t p = 0; p < kNumPresets; ++p) {
        std::unique_ptr<Engine> e(new Engine()); for (uint32_t k = 0; k < kParamCount; ++k) e->p[k] = presets()[p].v[k]; e->setSampleRate(SR);
        if (presets()[p].v[kMidiTrigger] > 0.5f && (int)presets()[p].v[kStyle] != kDucking) e->noteOn(60);   // a held note ducks a ducker fully
        std::vector<float> L(N), R(N); for (int i = 0; i < N; i += 512) e->process(l.data() + i, r.data() + i, sc.data() + i, sc.data() + i, L.data() + i, R.data() + i, std::min(512, N - i));
        double ss = 0; float pk = 0; bool nf = false;
        for (int i = SR; i < N; ++i) { ss += L[i] * L[i] + R[i] * R[i]; pk = std::max(pk, std::max(std::fabs(L[i]), std::fabs(R[i]))); nf |= !std::isfinite(L[i]); }
        const double rel = 10 * std::log10(ss / si + 1e-20);
        // noise floor between hits (the last 100 ms of each half second)
        double qn = 0, qi = 0; for (int i = SR; i < N; ++i) if (std::fmod(i / (float)SR, 0.5f) > 0.4f) { qn += L[i] * L[i]; qi += l[i] * l[i]; }
        const double floorDb = 10 * std::log10(qn / qi + 1e-20);
        const bool ok = !nf && pk < 2.f && rel > -18 && rel < 7; bad += !ok;
        printf("%-20s %-12s level %+5.1f dB  peak %.2f  between hits %+6.1f dB %s\n", presets()[p].name, presets()[p].category, rel, pk, floorDb, ok ? "" : "<-- CHECK");
    }
    printf("%u presets, %d flagged\n", kNumPresets, bad); return bad;
}
