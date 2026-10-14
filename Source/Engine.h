#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <vector>

// PTZR CLOUD - sound sources + granular engine
namespace ptzr
{
//==============================================================================
// 1) Sound libraries: rendered procedurally (no external samples needed).
//    Each library is an 8 s seamless stereo loop recorded at C3 (130.81 Hz).
//==============================================================================
struct SourceBuffer
{
    std::vector<float> L, R;
    double sampleRate = 44100.0;
    int size() const { return (int) L.size(); }
};

struct Formant { float f, bw, g; };

struct Spec
{
    std::vector<std::pair<float, float>> partials;      // ratio, amplitude
    std::vector<Formant> v1, v2;                        // vowel formants (choir)
    float vowelRate = 0.f;
    int   unison = 1;      float unisonCents = 0.f;
    float vibRate = 0.f,   vibCents = 0.f;
    float ampLfoRate = 0.f, ampLfoDepth = 0.f;
    float strikeEvery = 0.f, decay = 1.f, decayHF = 0.f; // struck sounds (piano, bells, keys)
    float hiss = 0.f, tuned = 0.f;                       // tape hiss / breath noise tuned to harmonics
    std::vector<int> tunedHarm;
    float tonalGain = 1.f;
};

inline Spec makeSpec (int lib)
{
    Spec s;
    auto saw = [&s] (int n, float roll) { for (int k = 1; k <= n; ++k) s.partials.push_back ({ (float) k, 1.f / std::pow ((float) k, roll) }); };
    switch (lib)
    {
        case 0: // Ethereal Choir (morphing ahh <-> ooh)
            saw (20, 0.7f);
            s.v1 = { { 800, 160, 1.f }, { 1150, 200, 0.6f }, { 2900, 300, 0.25f } };
            s.v2 = { { 350, 100, 1.f }, { 800, 150, 0.35f }, { 2700, 300, 0.1f } };
            s.vowelRate = 0.125f; s.unison = 3; s.unisonCents = 14; s.vibRate = 5.125f; s.vibCents = 9;
            s.ampLfoRate = 0.25f; s.ampLfoDepth = 0.25f; s.hiss = 0.03f; break;
        case 1: // Celestial Strings
            saw (18, 1.0f); s.unison = 4; s.unisonCents = 11; s.vibRate = 5.125f; s.vibCents = 7;
            s.ampLfoRate = 0.375f; s.ampLfoDepth = 0.2f; s.hiss = 0.02f; break;
        case 2: // Warm Halo Pad
            for (int k = 1; k <= 10; ++k) s.partials.push_back ({ (float) k, 1.f / std::pow ((float) k, 1.6f) });
            s.partials.push_back ({ 0.5f, 0.6f });
            s.unison = 5; s.unisonCents = 18; s.ampLfoRate = 0.125f; s.ampLfoDepth = 0.3f; break;
        case 3: // Glass Piano (struck, decaying - granulated into clouds)
            for (int k = 1; k <= 12; ++k) s.partials.push_back ({ (float) k * std::sqrt (1.f + 0.0004f * k * k), 1.f / std::pow ((float) k, 1.1f) });
            s.strikeEvery = 2.f; s.decay = 2.6f; s.decayHF = 0.6f; s.unison = 2; s.unisonCents = 3; break;
        case 4: // Tape Cloud (wow + hiss)
            s.partials = { { 1, 1.f }, { 2, 0.5f }, { 3, 0.35f }, { 4, 0.2f }, { 5, 0.15f } };
            s.unison = 4; s.unisonCents = 22; s.vibRate = 0.5f; s.vibCents = 25;
            s.ampLfoRate = 0.75f; s.ampLfoDepth = 0.25f; s.hiss = 0.5f; break;
        case 5: // Morning Breeze (tuned breath noise)
            s.partials = { { 1, 0.25f }, { 2, 0.2f } }; s.tonalGain = 0.35f; s.unison = 2; s.unisonCents = 6;
            s.tuned = 0.8f; s.tunedHarm = { 1, 2, 3, 4, 5, 6, 8, 10 }; s.hiss = 0.1f;
            s.ampLfoRate = 0.25f; s.ampLfoDepth = 0.4f; break;
        case 6: // Grand Cathedral (organ stack)
            s.partials = { { 0.5f, 0.6f }, { 1, 1.f }, { 2, 0.8f }, { 3, 0.5f }, { 4, 0.45f }, { 5, 0.25f }, { 6, 0.2f }, { 8, 0.18f } };
            s.unison = 3; s.unisonCents = 6; s.ampLfoRate = 0.125f; s.ampLfoDepth = 0.1f; s.hiss = 0.02f; break;
        case 7: // Angel Bells
            s.partials = { { 1, 1.f }, { 2.f, 0.5f }, { 2.76f, 0.6f }, { 5.4f, 0.45f }, { 8.93f, 0.3f }, { 11.34f, 0.2f } };
            s.strikeEvery = 4.f; s.decay = 5.f; s.decayHF = 1.f; s.unison = 2; s.unisonCents = 4; break;
        case 8: // Sunrise Keys (electric piano)
            s.partials = { { 1, 1.f }, { 2, 0.5f }, { 3, 0.2f }, { 4, 0.1f }, { 7, 0.15f } };
            s.strikeEvery = 1.f; s.decay = 1.8f; s.decayHF = 0.8f; s.unison = 2; s.unisonCents = 6;
            s.ampLfoRate = 4.5f; s.ampLfoDepth = 0.15f; break;
        default: // Shimmer Air
            s.partials = { { 2, 0.5f }, { 3, 0.5f }, { 4, 0.6f }, { 6, 0.5f }, { 8, 0.5f }, { 12, 0.4f }, { 16, 0.3f } };
            s.unison = 5; s.unisonCents = 9; s.vibRate = 0.625f; s.vibCents = 6;
            s.ampLfoRate = 0.375f; s.ampLfoDepth = 0.5f; s.hiss = 0.04f; s.tuned = 0.2f; s.tunedHarm = { 8, 12, 16 }; break;
    }
    return s;
}

inline float formantAmp (const std::vector<Formant>& fs, float f)
{
    float a = 0.03f;
    for (auto& x : fs) { const float d = (f - x.f) / x.bw; a += x.g * std::exp (-d * d); }
    return a;
}

inline void normaliseRms (std::vector<float>& a, std::vector<float>& b, float target)
{
    double e = 0; for (size_t i = 0; i < a.size(); ++i) e += (double) a[i] * a[i] + (double) b[i] * b[i];
    const double rms = std::sqrt (e / (2.0 * (double) a.size()));
    if (rms < 1e-9) return;
    const float g = (float) (target / rms);
    for (auto& x : a) x *= g;
    for (auto& x : b) x *= g;
}

inline std::shared_ptr<const SourceBuffer> renderLibrary (int lib)
{
    constexpr double sr = 44100.0, twoPi = 6.283185307179586, pi = 3.141592653589793, f0 = 130.8128;
    const int Lsz = (int) (8.0 * sr), X = (int) (0.75 * sr), N = Lsz + X;
    const Spec sp = makeSpec (lib);
    std::vector<float> l ((size_t) N, 0.f), r ((size_t) N, 0.f);
    juce::Random rng (7000 + lib);
    auto r8 = [] (double f) { return std::round (f * 8.0) / 8.0; };   // whole cycles per 8 s => seamless loop
    const double vibD = std::pow (2.0, sp.vibCents / 1200.0) - 1.0;

    for (int u = 0; u < sp.unison; ++u)
    {
        const double upos = sp.unison > 1 ? (double) u / (sp.unison - 1) * 2.0 - 1.0 : 0.0;
        const double det = std::pow (2.0, (upos * sp.unisonCents + (rng.nextFloat() - 0.5f) * 3.0) / 1200.0);
        const double ang = (upos * 0.8 + 1.0) * pi * 0.25;
        const float gl = (float) std::cos (ang), gr = (float) std::sin (ang);
        const double vp = rng.nextFloat() * twoPi, ap = rng.nextFloat() * twoPi;

        for (auto& pr : sp.partials)
        {
            const double ratio = pr.first, f = r8 (ratio * f0 * det);
            if (f > 15000.0) continue;
            float a1 = pr.second, a2 = pr.second;
            if (! sp.v1.empty())
            {
                a1 = pr.second * formantAmp (sp.v1, (float) (ratio * f0));
                a2 = pr.second * formantAmp (sp.v2.empty() ? sp.v1 : sp.v2, (float) (ratio * f0));
            }
            const double ph0 = rng.nextFloat() * twoPi;
            const double beta = sp.vibRate > 0.f ? f * vibD / sp.vibRate : 0.0;
            const double tau = sp.strikeEvery > 0.f ? std::max (0.05, sp.decay / (1.0 + sp.decayHF * (ratio - 1.0) * 0.35)) : 1.0;

            for (int n = 0; n < N; ++n)
            {
                const double t = n / sr;
                double ph = twoPi * f * t + ph0;
                if (beta > 0.0) ph += beta * std::sin (twoPi * sp.vibRate * t + vp);
                double a = a1;
                if (sp.vowelRate > 0.f) a = a1 + (a2 - a1) * (0.5 + 0.5 * std::sin (twoPi * sp.vowelRate * t));
                if (sp.ampLfoDepth > 0.f) a *= 1.0 - sp.ampLfoDepth + sp.ampLfoDepth * 0.5 * (1.0 + std::sin (twoPi * sp.ampLfoRate * t + ap));
                if (sp.strikeEvery > 0.f)
                {
                    const double tm = std::fmod (t, (double) sp.strikeEvery);
                    a *= std::exp (-tm / tau) * (1.0 - std::exp (-tm / 0.004));
                }
                const double s = std::sin (ph) * a;
                l[(size_t) n] += (float) (s * gl);
                r[(size_t) n] += (float) (s * gr);
            }
        }
    }
    normaliseRms (l, r, 0.25f * sp.tonalGain);

    // hiss (tape / air)
    if (sp.hiss > 0.f)
    {
        std::vector<float> nl ((size_t) N), nr ((size_t) N);
        const float a = (float) std::exp (-twoPi * 3000.0 / sr);
        float yl = 0, yr = 0;
        for (int n = 0; n < N; ++n)
        {
            yl = (1.f - a) * (rng.nextFloat() * 2.f - 1.f) + a * yl;
            yr = (1.f - a) * (rng.nextFloat() * 2.f - 1.f) + a * yr;
            nl[(size_t) n] = yl; nr[(size_t) n] = yr;
        }
        normaliseRms (nl, nr, sp.hiss * 0.5f);
        for (int n = 0; n < N; ++n) { l[(size_t) n] += nl[(size_t) n]; r[(size_t) n] += nr[(size_t) n]; }
    }

    // breath noise tuned to harmonics (narrow band-pass resonators)
    if (sp.tuned > 0.f)
    {
        std::vector<float> tl ((size_t) N, 0.f), tr ((size_t) N, 0.f);
        const double Q = 50.0;
        for (int h : sp.tunedHarm)
        {
            const double w0 = twoPi * (h * f0) / sr, alpha = std::sin (w0) / (2.0 * Q), a0 = 1.0 + alpha;
            const double b0 = alpha / a0, b2 = -alpha / a0, a1 = -2.0 * std::cos (w0) / a0, a2 = (1.0 - alpha) / a0;
            for (int ch = 0; ch < 2; ++ch)
            {
                auto& dst = ch == 0 ? tl : tr;
                double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
                for (int n = 0; n < N; ++n)
                {
                    const double x = rng.nextFloat() * 2.0 - 1.0;
                    const double y = b0 * x + b2 * x2 - a1 * y1 - a2 * y2;
                    x2 = x1; x1 = x; y2 = y1; y1 = y;
                    dst[(size_t) n] += (float) y / (float) (1.0 + 0.15 * h);
                }
            }
        }
        normaliseRms (tl, tr, sp.tuned * 0.5f);
        for (int n = 0; n < N; ++n) { l[(size_t) n] += tl[(size_t) n]; r[(size_t) n] += tr[(size_t) n]; }
    }

    // seamless loop: crossfade the tail into the head
    auto out = std::make_shared<SourceBuffer>();
    out->sampleRate = sr;
    out->L.assign ((size_t) Lsz, 0.f);
    out->R.assign ((size_t) Lsz, 0.f);
    for (int i = 0; i < Lsz; ++i)
    {
        float vl = l[(size_t) i], vr = r[(size_t) i];
        if (i < X)
        {
            const float w = (float) i / (float) X;
            vl = vl * w + l[(size_t) (Lsz + i)] * (1.f - w);
            vr = vr * w + r[(size_t) (Lsz + i)] * (1.f - w);
        }
        out->L[(size_t) i] = vl; out->R[(size_t) i] = vr;
    }
    normaliseRms (out->L, out->R, 0.18f);
    for (int i = 0; i < Lsz; ++i) { out->L[(size_t) i] = std::tanh (out->L[(size_t) i]); out->R[(size_t) i] = std::tanh (out->R[(size_t) i]); }
    return out;
}

//==============================================================================
// 2) Granular engine: hold ONE note -> evolving pad + melodic blinks
//==============================================================================
struct EngineParams
{
    float grainMs = 220, density = 24, position = 0.3f, scan = 0.5f, spread = 0.8f, pitch = 0, detune = 12,
          harmony = 0.35f, blinkRate = 1.2f, blinkAmt = 0.5f, octave = 0.25f, tape = 0.3f, glitch = 0.05f,
          attack = 1.5f, release = 3.f;
    int color = 0;
};

class GranularEngine
{
public:
    static constexpr int kVoices = 6, kGrains = 40;

    void prepare (double sr)
    {
        sampleRate = sr;
        for (int i = 0; i <= 1024; ++i) win[(size_t) i] = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * (float) i / 1024.f);
        reset();
    }
    void reset() { for (auto& v : voices) { v.active = false; for (auto& g : v.g) g.active = false; } }

    std::shared_ptr<const SourceBuffer> swapSource (std::shared_ptr<const SourceBuffer> s) { std::swap (src, s); return s; }

    void noteOn (int note, float vel)
    {
        Voice* t = nullptr;
        for (auto& v : voices) if (! v.active) { t = &v; break; }
        if (t == nullptr) { t = &voices[0]; for (auto& v : voices) if (v.order < t->order) t = &v; }
        const bool stolen = t->active;
        t->active = true; t->held = true; t->sustained = false; t->releasing = false; t->note = note;
        t->vel = 0.3f + 0.7f * std::pow (vel, 0.8f);
        if (! stolen) t->env = 0.f;
        t->order = ++orderCounter; t->nextGrain = 0.0; t->nextBlink = sampleRate * (0.3 + 0.9 * rnd());
        t->blinkIdx = rng.nextInt (3); t->stutterLeft = 0;
    }
    void noteOff (int note)
    {
        for (auto& v : voices)
            if (v.active && v.held && v.note == note) { v.held = false; if (sustainDown) v.sustained = true; else v.releasing = true; }
    }
    void allNotesOff() { for (auto& v : voices) if (v.active) { v.held = false; v.sustained = false; v.releasing = true; } }
    void setSustain (bool d)
    {
        sustainDown = d;
        if (! d) for (auto& v : voices) if (v.active && v.sustained) { v.sustained = false; v.releasing = true; }
    }
    void setPitchBend (float semis) { bend = semis; }

    float getActivity() const { return activity.load(); }
    std::atomic<int> blinkCount { 0 };

    void render (float* L, float* R, int n, const EngineParams& p)
    {
        auto s = src.get();
        if (s == nullptr || s->size() < 1024) { activity.store (0.f); return; }
        const int N = s->size();
        const float* sl = s->L.data();
        const float* sr_ = s->R.data();
        const double rateConv = s->sampleRate / sampleRate;
        const float attC = 1.f - std::exp (-4.6f / (std::max (0.01f, p.attack) * (float) sampleRate));
        const float relC = std::exp (-6.9f / (std::max (0.05f, p.release) * (float) sampleRate));
        const double effDensity = std::min ((double) p.density, 30.0 / std::max (0.02, (double) p.grainMs * 0.001));
        const float padAmp = (float) (1.1 / std::sqrt (std::max (1.0, effDensity * p.grainMs * 0.001)));
        const double twoPi = juce::MathConstants<double>::twoPi;
        int alive = 0;

        for (auto& v : voices)
        {
            if (! v.active) continue;
            for (int i = 0; i < n; ++i)
            {
                if (v.releasing) v.env *= relC; else v.env += (1.f - v.env) * attC;
                if (v.releasing && v.env < 1e-4f) { v.active = false; for (auto& g : v.g) g.active = false; break; }

                v.scanPh += twoPi * 0.05 / sampleRate; if (v.scanPh > twoPi) v.scanPh -= twoPi;

                if ((v.nextGrain -= 1.0) <= 0.0)
                {
                    if (v.stutterLeft > 0)
                    {
                        spawnGrain (v, v.stPos, v.stInc, v.stLen, v.stAmp, v.stPan);
                        --v.stutterLeft; v.nextGrain += (double) v.stLen * 0.95;
                    }
                    else
                    {
                        spawnPad (v, p, N, rateConv, padAmp);
                        v.nextGrain += (sampleRate / std::max (0.5, effDensity)) * (0.6 + 0.8 * rnd());
                    }
                }
                if ((v.nextBlink -= 1.0) <= 0.0)
                {
                    if (p.blinkAmt > 0.001f) fireBlink (v, p, N, rateConv);
                    v.nextBlink += (sampleRate / std::max (0.05, (double) p.blinkRate)) * (0.5 + rnd());
                }

                const double mod = 1.0 + p.tape * (0.010 * std::sin (v.tapePh) + 0.004 * std::sin (v.tapePh * 3.7 + 1.3));
                v.tapePh += twoPi * 0.55 / sampleRate; if (v.tapePh > twoPi) v.tapePh -= twoPi;

                float oL = 0.f, oR = 0.f;
                for (auto& g : v.g)
                {
                    if (! g.active) continue;
                    const float w = win[(size_t) (int) ((float) g.age / (float) g.len * 1024.f)];
                    int i0 = (int) g.pos; if (i0 >= N) i0 = N - 1;
                    int i1 = i0 + 1; if (i1 >= N) i1 = 0;
                    const float fr = (float) (g.pos - i0);
                    const float a = sl[i0] + (sl[i1] - sl[i0]) * fr;
                    const float b = sr_[i0] + (sr_[i1] - sr_[i0]) * fr;
                    const float ga = w * g.amp;
                    oL += a * ga * g.gl; oR += b * ga * g.gr;
                    g.pos += g.inc * mod;
                    if (g.pos >= N) g.pos -= N; else if (g.pos < 0.0) g.pos += N;
                    if (++g.age >= g.len) g.active = false;
                    ++alive;
                }
                const float e = v.env * v.vel * 1.4f;
                L[i] += oL * e; R[i] += oR * e;
            }
        }
        const float act = juce::jlimit (0.f, 1.f, (float) alive / (float) (std::max (1, n) * 40));
        activity.store (act);
    }

private:
    struct Grain { bool active = false; double pos = 0, inc = 1; int age = 0, len = 1; float amp = 0, gl = 1, gr = 1; };
    struct Voice
    {
        bool active = false, held = false, sustained = false, releasing = false;
        int note = 48; float vel = 1.f, env = 0.f; uint64_t order = 0;
        std::array<Grain, kGrains> g;
        double nextGrain = 0, nextBlink = 0, scanPh = 0, tapePh = 0;
        int blinkIdx = 0, stutterLeft = 0, stLen = 0; double stPos = 0, stInc = 1; float stAmp = 0, stPan = 0;
    };

    static constexpr int kChord[4][6]  = { { 2, 7, 12, 14, -5, -12 }, { 4, 7, 12, 16, -5, -12 }, { 3, 7, 12, 15, -5, -12 }, { 6, 7, 11, 14, 19, -12 } };
    static constexpr int kMelody[4][6] = { { 0, 2, 7, 12, 14, 19 },   { 0, 4, 7, 12, 16, 19 },   { 0, 3, 7, 12, 15, 19 },   { 0, 4, 6, 7, 11, 14 } };

    float rnd() { return rng.nextFloat(); }
    static double wrapPos (double x, int N) { x = std::fmod (x, (double) N); if (x < 0) x += N; return x; }

    void spawnGrain (Voice& v, double pos, double inc, int len, float amp, float pan)
    {
        Grain* t = nullptr;
        for (auto& g : v.g) if (! g.active) { t = &g; break; }
        if (t == nullptr) { int best = -1; for (auto& g : v.g) if (g.age > best) { best = g.age; t = &g; } }
        t->active = true; t->pos = pos; t->inc = inc; t->age = 0; t->len = std::max (16, len); t->amp = amp;
        t->gl = pan > 0.f ? 1.f - pan : 1.f; t->gr = pan < 0.f ? 1.f + pan : 1.f;
    }

    void spawnPad (Voice& v, const EngineParams& p, int N, double rateConv, float amp)
    {
        const int color = juce::jlimit (0, 3, p.color);
        float semis = 0.f;
        const float r = rnd();
        if (r < p.octave * 0.35f) semis = 12.f;
        else if (r < p.octave * 0.35f + p.harmony) semis = (float) kChord[color][rng.nextInt (6)];

        const double cents = (rnd() - 0.5) * 2.0 * p.detune;
        const double rate = rateConv * std::pow (2.0, ((v.note - 48) + p.pitch + bend + semis + cents / 100.0) / 12.0);
        int len = (int) std::max (64.0, p.grainMs * 0.001 * sampleRate * (0.75 + 0.5 * rnd()));
        double pos = wrapPos (p.position * N + (rnd() - 0.5) * p.scan * N * 0.6 + std::sin (v.scanPh) * p.scan * N * 0.1, N);
        const float pan = (rnd() * 2.f - 1.f) * p.spread;
        const bool rev = rnd() < p.glitch * 0.5f;

        if (p.glitch > 0.f && rnd() < p.glitch * 0.12f)   // stutter: repeat a tiny slice
        {
            len = (int) (sampleRate * (0.02 + 0.05 * rnd()));
            v.stutterLeft = 2 + rng.nextInt (5); v.stLen = len; v.stPos = pos; v.stInc = rev ? -rate : rate;
            v.stAmp = amp; v.stPan = pan;
        }
        if (rev) pos = wrapPos (pos + len * rate, N);
        spawnGrain (v, pos, rev ? -rate : rate, len, amp, pan);
    }

    void fireBlink (Voice& v, const EngineParams& p, int N, double rateConv)
    {
        const int color = juce::jlimit (0, 3, p.color);
        v.blinkIdx = juce::jlimit (0, 5, v.blinkIdx + rng.nextInt (4) - 1);
        float semis = (float) kMelody[color][v.blinkIdx];
        if (rnd() < p.octave * 0.4f) semis += 12.f;
        const double rate = rateConv * std::pow (2.0, ((v.note - 48) + p.pitch + bend + semis) / 12.0);
        const int len = (int) (sampleRate * (0.55 + 0.8 * rnd()));
        const double pos = wrapPos (p.position * N + (rnd() - 0.5) * p.scan * N * 0.5, N);
        spawnGrain (v, pos, rate, len, p.blinkAmt * 0.9f, (rnd() * 2.f - 1.f) * p.spread);
        blinkCount.fetch_add (1);
    }

    double sampleRate = 44100.0;
    std::array<float, 1025> win {};
    std::array<Voice, kVoices> voices;
    std::shared_ptr<const SourceBuffer> src;
    juce::Random rng { 1337 };
    uint64_t orderCounter = 0;
    bool sustainDown = false;
    float bend = 0.f;
    std::atomic<float> activity { 0.f };
};
} // namespace ptzr
