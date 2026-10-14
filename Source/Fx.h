#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <vector>

// PTZR CLOUD - effects: tremolo, delay (stereo / ping-pong / tape), reverb (6 types) + granular-style pitch shimmer
namespace ptzr
{
constexpr double kTwoPi = 6.283185307179586, kPi = 3.141592653589793;

//------------------------------------------------------------------ granular pitch shifter (2 windowed taps)
struct PitchShifter
{
    std::vector<float> buf; int w = 0, win = 4096; float phase = 0.f;
    void prepare (double sr) { win = (int) (sr * 0.09); buf.assign ((size_t) win * 2 + 8, 0.f); w = 0; phase = 0.f; }
    float rd (float d) const
    {
        const int size = (int) buf.size();
        float p = (float) w - d; while (p < 0.f) p += (float) size;
        int i0 = (int) p; const float fr = p - (float) i0; if (i0 >= size) i0 -= size;
        int i1 = i0 + 1; if (i1 >= size) i1 = 0;
        return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * fr;
    }
    float process (float x, float ratio)
    {
        buf[(size_t) w] = x;
        float p2 = phase + 0.5f; if (p2 >= 1.f) p2 -= 1.f;
        const float w1 = 0.5f - 0.5f * std::cos ((float) kTwoPi * phase);
        const float y = rd (phase * (float) win) * w1 + rd (p2 * (float) win) * (1.f - w1);
        phase += (1.f - ratio) / (float) win;
        while (phase >= 1.f) phase -= 1.f;
        while (phase < 0.f) phase += 1.f;
        if (++w >= (int) buf.size()) w = 0;
        return y;
    }
};

//------------------------------------------------------------------ tremolo / auto-pan
struct Tremolo
{
    double ph = 0.0;
    void process (float* L, float* R, int n, float rate, float depth, float stereo, int shape, double sr)
    {
        if (depth < 0.001f) return;
        const double inc = kTwoPi * rate / sr, off = stereo * kPi;
        auto lfo = [shape] (double p) -> double
        {
            const double s = std::sin (p);
            if (shape == 1) return (2.0 / kPi) * std::asin (s);
            if (shape == 2) return std::tanh (5.0 * s) / std::tanh (5.0);
            return s;
        };
        for (int i = 0; i < n; ++i)
        {
            L[i] *= (float) (1.0 - depth * 0.5 * (1.0 + lfo (ph)));
            R[i] *= (float) (1.0 - depth * 0.5 * (1.0 + lfo (ph + off)));
            ph += inc; if (ph > kTwoPi) ph -= kTwoPi;
        }
    }
};

//------------------------------------------------------------------ delay
struct StereoDelay
{
    std::vector<float> bl, br; int size = 0, w = 0; double curT = 0, lfo = 0; float lpL = 0, lpR = 0, mixS = 0;
    void prepare (double sr) { size = (int) (sr * 3.5); bl.assign ((size_t) size, 0.f); br.assign ((size_t) size, 0.f); w = 0; curT = sr * 0.3; lpL = lpR = 0; lfo = 0; mixS = 0; }
    float read (const std::vector<float>& b, double d) const
    {
        double p = (double) w - d; while (p < 0.0) p += size;
        int i0 = (int) p; const float fr = (float) (p - i0); if (i0 >= size) i0 -= size;
        int i1 = i0 + 1; if (i1 >= size) i1 = 0;
        return b[(size_t) i0] + (b[(size_t) i1] - b[(size_t) i0]) * fr;
    }
    void process (float* L, float* R, int n, int type, double targetSamples, float fb, float mix, double sr)
    {
        targetSamples = juce::jlimit (32.0, sr * 3.0, targetSamples);
        for (int i = 0; i < n; ++i)
        {
            curT += (targetSamples - curT) * 0.0006;
            mixS += (mix - mixS) * 0.002f;
            double dL = curT, dR = curT;
            if (type == 0) dR = curT * 0.75;
            if (type == 2)
            {
                lfo += kTwoPi * 0.7 / sr;
                const double m = sr * 0.0012 * std::sin (lfo) + sr * 0.0004 * std::sin (lfo * 3.1 + 0.7);
                dL += m; dR += m * 0.8;
            }
            const float yL = read (bl, dL), yR = read (br, dR);
            const float inL = L[i], inR = R[i];
            float wL, wR;
            if (type == 1) { const float mono = 0.5f * (inL + inR); wL = mono + fb * yR; wR = fb * yL; }
            else if (type == 2)
            {
                lpL += 0.3f * (fb * yL - lpL); lpR += 0.3f * (fb * yR - lpR);
                wL = inL + std::tanh (lpL); wR = inR + std::tanh (lpR);
            }
            else { wL = inL + fb * yL; wR = inR + fb * yR; }
            bl[(size_t) w] = wL; br[(size_t) w] = wR; if (++w >= size) w = 0;
            L[i] = inL + mixS * yL; R[i] = inR + mixS * yR;
        }
    }
};

//------------------------------------------------------------------ reverb
struct ReverbFx
{
    static constexpr int kChunk = 16;
    juce::Reverb rev; PitchShifter psL, psR;
    std::vector<float> preL, preR; int pw = 0;
    float prevL[kChunk] {}, prevR[kChunk] {}; float mixS = 0.f;

    void prepare (double sr)
    {
        rev.setSampleRate (sr); rev.reset(); psL.prepare (sr); psR.prepare (sr);
        preL.assign ((size_t) (sr * 0.25) + 8, 0.f); preR = preL; pw = 0; mixS = 0.f;
        std::fill (std::begin (prevL), std::end (prevL), 0.f); std::fill (std::begin (prevR), std::end (prevR), 0.f);
    }

    void process (float* L, float* R, int n, int type, float mix, float size, float damp, float preMs,
                  float width, float shimmer, int shimInt, double sr)
    {
        static const float baseSize[6] = { 0.78f, 0.50f, 0.25f, 0.94f, 0.88f, 1.00f };
        static const float baseDamp[6] = { 0.45f, 0.20f, 0.55f, 0.60f, 0.40f, 0.25f };
        type = juce::jlimit (0, 5, type);
        juce::Reverb::Parameters pr;
        pr.roomSize = type == 5 ? 1.f : juce::jlimit (0.05f, 0.99f, baseSize[type] + (size - 0.5f) * 0.4f);
        pr.damping = juce::jlimit (0.f, 1.f, baseDamp[type] + (damp - 0.5f) * 0.6f);
        pr.wetLevel = 0.33f; pr.dryLevel = 0.f; pr.width = width; pr.freezeMode = 0.f;
        rev.setParameters (pr);

        const float shimAmt = (type == 4 ? juce::jmax (shimmer, 0.35f) : shimmer) * 0.5f;
        const float ratio = shimInt == 1 ? 1.5f : (shimInt == 2 ? 0.5f : 2.f);
        const int preSamples = juce::jlimit (0, (int) preL.size() - 2, (int) (preMs * 0.001 * sr));
        const int psize = (int) preL.size();

        for (int start = 0; start < n; start += kChunk)
        {
            const int m = std::min (kChunk, n - start);
            float inL[kChunk], inR[kChunk];
            for (int i = 0; i < m; ++i)
            {
                preL[(size_t) pw] = L[start + i]; preR[(size_t) pw] = R[start + i];
                int rp = pw - preSamples; if (rp < 0) rp += psize;
                inL[i] = preL[(size_t) rp]; inR[i] = preR[(size_t) rp];
                if (++pw >= psize) pw = 0;
                if (shimAmt > 0.001f)
                {
                    inL[i] += shimAmt * psL.process (prevL[i], ratio);
                    inR[i] += shimAmt * psR.process (prevR[i], ratio);
                }
            }
            rev.processStereo (inL, inR, m);
            for (int i = 0; i < m; ++i)
            {
                prevL[i] = inL[i]; prevR[i] = inR[i];
                mixS += (mix - mixS) * 0.002f;
                L[start + i] = L[start + i] * (1.f - 0.4f * mixS) + inL[i] * mixS * 1.4f;
                R[start + i] = R[start + i] * (1.f - 0.4f * mixS) + inR[i] * mixS * 1.4f;
            }
        }
    }
};
} // namespace ptzr
