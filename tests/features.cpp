// Feature tests for Slate: open/closed levels for every style, ratio, range, knee, upward expansion,
// attack, hold and release timing, hysteresis, look-ahead and latency, oversampling transparency,
// external sidechain, sidechain filters and level, audition, MIDI triggering, wet/dry/pan, linking, mid/side.
#include "../plugins/Slate/ParamDefs.hpp"
#include "../plugins/Slate/Presets.hpp"
#include <cstdio>
#include <memory>
#include <functional>
#include <vector>
using namespace slate;

static int fails = 0;
static void check(bool ok, const char* fmt, double a = 0, double b = 0, double c = 0) { printf("%s  ", ok ? "PASS" : "FAIL"); printf(fmt, a, b, c); printf("\n"); fails += !ok; }
static const float fs = 48000;
typedef std::unique_ptr<Engine> EP;
typedef std::vector<float> V;
static EP make(int style = kClassic) { EP e(new Engine()); e->p[kStyle] = (float)style; e->p[kKnee] = 0; e->p[kHold] = 0; return e; }
static void run(Engine& e, const V& l, const V& r, V& L, V& R, const V* sc = nullptr, std::function<void(Engine&, size_t)> each = nullptr) {
    L.assign(l.size(), 0); R.assign(l.size(), 0);
    for (size_t i = 0; i < l.size(); i += 64) { const uint32_t n = (uint32_t)std::min<size_t>(64, l.size() - i); if (each) each(e, i);
        e.process(l.data() + i, r.data() + i, sc ? sc->data() + i : nullptr, sc ? sc->data() + i : nullptr, L.data() + i, R.data() + i, n); }
}
static V sine(float hz, float db, int n) { V v(n); const float a = dbToGain(db); for (int i = 0; i < n; ++i) v[i] = a * std::sin(2 * kPi * hz * i / fs); return v; }
static double rmsDb(const V& v, size_t a, size_t b) { double s = 0; for (size_t i = a; i < b; ++i) s += v[i] * v[i]; return 10 * std::log10(s / (b - a) + 1e-30); }
static double gainOf(Engine& e, float hz, float db, int n = 48000) { V in = sine(hz, db, n), L, R; e.setSampleRate(fs); run(e, in, in, L, R); return rmsDb(L, n / 2, n) - rmsDb(in, n / 2, n); }
static const char* sn(int s) { static const char* n[] = { "Classic", "Clean", "Vocal", "Guitar", "Upward" }; return n[s]; }

int main() {
    const int N = 48000;
    // 1. every downward style: open above the threshold, closed (range) well below it
    for (int s = 0; s < 4; ++s) {
        EP a = make(s); const double open = gainOf(*a, 1000, -20);
        EP b = make(s); const double closed = gainOf(*b, 1000, -70, 144000);
        check(std::fabs(open) < 0.1 && std::fabs(closed + 40) < 0.5, "open %.2f dB, closed %.1f dB (range 40)", open, closed); printf("        (%s)\n", sn(s)); }
    // 2. ratio, range and knee
    { EP a = make(); a->p[kRatio] = 2; a->p[kRange] = 80; const double r2 = gainOf(*a, 1000, -50);
      EP b = make(); b->p[kRatio] = 100; b->p[kRange] = 12; const double cap = gainOf(*b, 1000, -60);
      EP c = make(kClean); c->p[kRatio] = 3; c->p[kRange] = 80; c->p[kKnee] = 12; const double kn = gainOf(*c, 1000, -40);
      check(std::fabs(r2 + 10) < 0.6, "ratio 2:1, 10 dB under the threshold: %.2f dB (expect -10)", r2);
      check(std::fabs(cap + 12) < 0.3, "range 12 dB caps the attenuation: %.2f dB", cap);
      check(std::fabs(kn + 3) < 0.6, "12 dB knee at the threshold, 3:1: %.2f dB (expect -3)", kn); }
    // 3. upward expansion
    { EP a = make(kUpward); a->p[kRatio] = 1.5f; a->p[kRange] = 6; a->p[kKnee] = 0; const double capd = gainOf(*a, 1000, -20);
      EP b = make(kUpward); b->p[kRatio] = 1.2f; b->p[kRange] = 20; b->p[kKnee] = 0; const double free_ = gainOf(*b, 1000, -20);
      EP c = make(kUpward); c->p[kRatio] = 2; const double below = gainOf(*c, 1000, -60);
      check(std::fabs(capd - 6) < 0.4 && std::fabs(free_ - 4) < 0.6 && std::fabs(below) < 0.1, "upward: +%.2f dB (capped at 6), +%.2f dB (1.2:1, 20 dB over), %.2f dB below the threshold", capd, free_, below); }
    // 4. attack, hold and release
    { EP e = make(kClean); e->p[kAttack] = 5; e->p[kHold] = 100; e->p[kRelease] = 50; e->p[kRatio] = 100; e->p[kRange] = 60; e->setSampleRate(fs);
      V in(N); for (int i = 0; i < N; ++i) in[i] = (i >= 12000 && i < 24000 ? 0.3f : 0.0003f) * std::sin(2 * kPi * 1000 * i / fs);
      V L, R; run(*e, in, in, L, R);
      auto g = [&](int i) { return rmsDb(L, i - 96, i) - rmsDb(in, i - 96, i); };
      const double a2 = g(12000 + 96), a20 = g(12000 + 960), h50 = g(24000 + 2400), r300 = g(24000 + 4800 + 14400);
      check(a2 < -3 && a20 > -0.5, "attack 5 ms: %.1f dB after 2 ms, %.2f dB after 20 ms", a2, a20);
      check(std::fabs(h50) < 0.5 && r300 < -40, "hold 100 ms: %.2f dB 50 ms after the signal drops; closed (%.0f dB) 300 ms after the hold", h50, r300); }
    // 5. hysteresis: Vocal stays open 2 dB under the threshold once opened; Clean closes
    { auto tail = [&](int s) { EP e = make(s); e->p[kRatio] = 100; e->p[kRange] = 40; e->p[kRelease] = 20; e->setSampleRate(fs);
          V in(N); for (int i = 0; i < N; ++i) in[i] = dbToGain(i < 12000 ? -30.f : -42.f) * std::sin(2 * kPi * 1000 * i / fs);
          V L, R; run(*e, in, in, L, R); return rmsDb(L, 36000, N) - rmsDb(in, 36000, N); };
      const double vocal = tail(kVocal), clean = tail(kClean);
      check(std::fabs(vocal) < 0.5 && clean < -30, "2 dB under the threshold after opening: Vocal %.2f dB (hysteresis), Clean %.1f dB", vocal, clean); }
    // 6. latency and look-ahead
    { EP a = make(), b = make(), c = make(), d = make(); b->p[kOversampling] = 1; c->p[kOversampling] = 2; d->p[kLookahead] = 5; d->p[kOversampling] = 2;
      for (Engine* x : { a.get(), b.get(), c.get(), d.get() }) x->setSampleRate(fs);
      check(a->latency() == 0 && b->latency() == 32 && c->latency() == 48 && d->latency() == 288, "latency: %.0f / 32 / 48 samples; 5 ms look-ahead + 4x: %.0f (expect 288)", a->latency(), d->latency());
      // the reported latency is the real one
      EP h = make(); h->p[kThreshold] = -80; h->p[kLookahead] = 3; h->p[kOversampling] = 1; h->setSampleRate(fs);
      V in(8192, 0.f), L, R; in[1000] = 0.5f; run(*h, in, in, L, R); int pk = 0; for (int i = 0; i < 8192; ++i) if (std::fabs(L[i]) > std::fabs(L[pk])) pk = i;
      check(pk - 1000 == (int)h->latency(), "an impulse comes out %.0f samples late, reported %.0f", pk - 1000, h->latency());
      auto onset = [&](float la) { EP e = make(kClean); e->p[kAttack] = 2; e->p[kRatio] = 100; e->p[kRange] = 60; e->p[kLookahead] = la; e->setSampleRate(fs);
          V x(N); for (int i = 0; i < N; ++i) x[i] = (i >= 24000 ? 0.5f : 0.f) * std::sin(2 * kPi * 1000 * i / fs);
          V L2, R2; run(*e, x, x, L2, R2); const int lat = (int)e->latency(); return rmsDb(L2, 24000 + lat, 24000 + lat + 96) - rmsDb(x, 24000, 24096); };
      const double without = onset(0), with = onset(5);
      check(with > -1 && without < -3, "first 2 ms of a sudden note: %.1f dB with 5 ms look-ahead, %.1f dB without", with, without); }
    // 7. oversampling is transparent when the gate is open
    { double worst = 0; for (int os = 1; os < 3; ++os) for (float hz : { 100.f, 1000.f, 10000.f, 18000.f }) { EP e = make(); e->p[kOversampling] = (float)os; worst = std::max(worst, std::fabs(gainOf(*e, hz, -12))); }
      check(worst < 0.05, "2x/4x oversampling with the gate open: level within %.3f dB up to 18 kHz", worst); }
    // 8. external sidechain, sidechain level, filters, audition
    { EP e = make(kClean); e->p[kSidechain] = 1; e->p[kRatio] = 100; e->p[kRange] = 40; e->setSampleRate(fs);
      V s = sine(1000, -20, N), key = sine(200, -10, N), z(N, 0.f), L, R;
      run(*e, s, s, L, R, &z); const double noKey = rmsDb(L, N / 2, N) - rmsDb(s, N / 2, N);
      e->setSampleRate(fs); run(*e, s, s, L, R, &key); const double withKey = rmsDb(L, N / 2, N) - rmsDb(s, N / 2, N);
      check(noKey < -35 && std::fabs(withKey) < 0.2, "external key: silent key %.1f dB, -10 dB key %.2f dB", noKey, withKey);
      EP f = make(kClean); f->p[kScGain] = -40; const double lowered = gainOf(*f, 1000, -20);
      EP g = make(kClean); g->p[kScHpOn] = 1; g->p[kScHpFreq] = 2000; const double hp = gainOf(*g, 100, -20);
      check(lowered < -35 && hp < -35, "sidechain level -40 dB closes the gate (%.1f dB); 2 kHz high-pass ignores a 100 Hz tone (%.1f dB)", lowered, hp);
      EP h = make(); h->p[kScListen] = 1; h->p[kScLpOn] = 1; h->p[kScLpFreq] = 200; h->setSampleRate(fs);
      V mix(N); V a = sine(100, -20, N), b = sine(5000, -20, N); for (int i = 0; i < N; ++i) mix[i] = a[i] + b[i];
      run(*h, mix, mix, L, R); const double lo = rmsDb(L, N / 2, N) - rmsDb(a, N / 2, N);
      check(std::fabs(lo) < 0.5, "audition plays the filtered sidechain: 100 Hz + 5 kHz through a 200 Hz low-pass leaves %.2f dB vs the 100 Hz part", lo); }
    // 9. MIDI triggering
    { EP e = make(kClean); e->p[kMidiTrigger] = 1; e->p[kThreshold] = 0; e->p[kRatio] = 100; e->p[kRange] = 60; e->setSampleRate(fs);
      V s = sine(1000, -20, N), L, R;
      run(*e, s, s, L, R, nullptr, [](Engine& en, size_t i) { if (i == 12032) en.noteOn(60); if (i == 24000) en.noteOff(60); });
      const double before = rmsDb(L, 6000, 12000) - rmsDb(s, 6000, 12000), during = rmsDb(L, 16000, 23900) - rmsDb(s, 16000, 23900), after = rmsDb(L, 44000, N) - rmsDb(s, 44000, N);
      check(before < -50 && std::fabs(during) < 0.1 && after < -50, "MIDI trigger: %.0f dB before the note, %.2f dB while held, %.0f dB after", before, during, after); }
    // 10. wet, dry, pan
    { EP a = make(kClean); a->p[kDry] = -6; a->p[kRange] = 80; a->p[kRatio] = 100; const double dry = gainOf(*a, 1000, -70);
      EP b = make(); b->p[kWet] = -6; const double wet = gainOf(*b, 1000, -20);
      EP c = make(); c->p[kPan] = 1; c->setSampleRate(fs); V s = sine(1000, -20, N), L, R; run(*c, s, s, L, R);
      const double left = rmsDb(L, N / 2, N) - rmsDb(s, N / 2, N), right = rmsDb(R, N / 2, N) - rmsDb(s, N / 2, N);
      check(std::fabs(dry + 6) < 0.3 && std::fabs(wet + 6) < 0.1, "dry -6 dB with the gate shut: %.2f dB; wet -6 dB, open: %.2f dB", dry, wet);
      check(left < -100 && std::fabs(right) < 0.1, "wet pan right: left %.0f dB, right %.2f dB", left, right); }
    // 11. stereo link and mid/side
    { auto quietR = [&](float link) { EP e = make(kClean); e->p[kLink] = link; e->p[kRatio] = 100; e->p[kRange] = 40; e->setSampleRate(fs);
          V l = sine(1000, -10, N), r = sine(1000, -70, N), L, R; run(*e, l, r, L, R); return rmsDb(R, N / 2, N) - rmsDb(r, N / 2, N); };
      const double un = quietR(0), lk = quietR(1);
      check(un < -35 && std::fabs(lk) < 0.2, "quiet right channel: unlinked %.1f dB (gated), linked %.2f dB (follows the left)", un, lk);
      EP e = make(kClean); e->p[kChannelMode] = 1; e->p[kLink] = 0; e->p[kRatio] = 100; e->p[kRange] = 40; e->setSampleRate(fs);
      V m = sine(1000, -10, N), sd = sine(3000, -70, N), l(N), r(N), L, R; for (int i = 0; i < N; ++i) { l[i] = m[i] + sd[i]; r[i] = m[i] - sd[i]; }
      run(*e, l, r, L, R); V side(N), mid(N); for (int i = 0; i < N; ++i) { side[i] = 0.5f * (L[i] - R[i]); mid[i] = 0.5f * (L[i] + R[i]); }
      const double sideG = rmsDb(side, N / 2, N) - rmsDb(sd, N / 2, N), midG = rmsDb(mid, N / 2, N) - rmsDb(m, N / 2, N);
      check(sideG < -35 && std::fabs(midG) < 0.2, "mid/side, unlinked: quiet side gated %.1f dB, loud mid %.2f dB", sideG, midG); }
    // 12. ducking (2.0)
    { // depth: a key above the threshold ducks by Range (100:1), or by (ratio-1) dB per dB over the threshold
      auto duckGain = [&](float keyDb, float ratio, float range, float thr = -30) {
          EP e = make(kDucking); e->p[kSidechain] = 1; e->p[kThreshold] = thr; e->p[kRatio] = ratio; e->p[kRange] = range; e->setSampleRate(fs);
          V s = sine(1000, -20, N), key = sine(200, keyDb, N), L, R; run(*e, s, s, L, R, &key); return rmsDb(L, N / 2, N) - rmsDb(s, N / 2, N); };
      const double full = duckGain(-10, 100, 18), none = duckGain(-50, 100, 18), half = duckGain(-24, 2, 40);
      check(std::fabs(full + 18) < 0.3 && std::fabs(none) < 0.1, "ducking, 100:1: key 20 dB over the threshold ducks %.2f dB (range 18); key under it: %.2f dB", full, none);
      check(std::fabs(half + 6) < 0.6, "ducking, 2:1: key 6 dB over the threshold ducks %.2f dB (expect -6)", half);
      // the dynamics: attack turns it down when the key arrives, hold keeps it down, release brings it back
      EP e = make(kDucking); e->p[kSidechain] = 1; e->p[kThreshold] = -30; e->p[kRatio] = 100; e->p[kRange] = 20;
      e->p[kAttack] = 5; e->p[kHold] = 100; e->p[kRelease] = 50; e->setSampleRate(fs);
      V s = sine(1000, -20, N), key(N), L, R; for (int i = 0; i < N; ++i) key[i] = (i >= 12000 && i < 24000 ? 0.3f : 0.f) * std::sin(2 * kPi * 200 * i / fs);
      run(*e, s, s, L, R, &key);
      auto g = [&](int i) { return rmsDb(L, i - 96, i) - rmsDb(s, i - 96, i); };
      const double pre = g(11900), a2 = g(12000 + 96), a30 = g(12000 + 1440), h50 = g(24000 + 2400), back = g(24000 + 4800 + 14400);
      check(std::fabs(pre) < 0.1 && a2 > -14 && a30 < -19, "ducking attack 5 ms: %.2f dB before the key, %.1f dB 2 ms in, %.1f dB 30 ms in", pre, a2, a30);
      check(h50 < -19 && std::fabs(back) < 0.3, "ducking hold 100 ms: still %.1f dB 50 ms after the key stops; back to %.2f dB 300 ms after the hold", h50, back);
      // starts at unity (no dip before the key has been heard), a held MIDI note ducks fully
      EP m = make(kDucking); m->p[kMidiTrigger] = 1; m->p[kThreshold] = 0; m->p[kRatio] = 100; m->p[kRange] = 30; m->p[kAttack] = 0.5f; m->p[kRelease] = 20; m->setSampleRate(fs);
      run(*m, s, s, L, R, nullptr, [](Engine& en, size_t i) { if (i == 12032) en.noteOn(60); if (i == 24000) en.noteOff(60); });
      const double mb = rmsDb(L, 0, 4800) - rmsDb(s, 0, 4800), md = rmsDb(L, 16000, 23900) - rmsDb(s, 16000, 23900), ma = rmsDb(L, 40000, N) - rmsDb(s, 40000, N);
      check(std::fabs(mb) < 0.1 && std::fabs(md + 30) < 0.3 && std::fabs(ma) < 0.1, "MIDI duck: %.2f dB from the start, %.2f dB while a note is held, %.2f dB after", mb, md, ma); }
    // 13. presets stay finite
    { bool ok = true; for (uint32_t i = 0; i < kNumPresets; ++i) { EP e(new Engine()); std::copy(presets()[i].v, presets()[i].v + kParamCount, e->p); ok &= std::isfinite(gainOf(*e, 1000, -20, 12000)); }
      check(ok, "all %.0f presets produce finite output", kNumPresets); }
    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASSED", fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
