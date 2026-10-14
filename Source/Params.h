#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace ptzr
{
enum class Unit { Pct, Ms, Hz, HzFine, Sec, St, Ct, Db, PerSec };

struct FloatSpec { const char* id; const char* name; float lo, hi, def, skewMid; Unit unit; };

inline const juce::StringArray kLibraries { "Ethereal Choir", "Celestial Strings", "Warm Halo Pad", "Glass Piano",
                                            "Tape Cloud", "Morning Breeze", "Grand Cathedral", "Angel Bells",
                                            "Sunrise Keys", "Shimmer Air" };

inline const std::vector<FloatSpec>& floatSpecs()
{
    static const std::vector<FloatSpec> s = {
        { "size",      "Grain Size",      20.f,   800.f,   220.f,   180.f,  Unit::Ms },
        { "density",   "Grain Density",   1.f,    80.f,    24.f,    18.f,   Unit::PerSec },
        { "position",  "Position",        0.f,    1.f,     0.3f,    0.f,    Unit::Pct },
        { "scan",      "Scan",            0.f,    1.f,     0.5f,    0.f,    Unit::Pct },
        { "spread",    "Spread",          0.f,    1.f,     0.8f,    0.f,    Unit::Pct },
        { "pitch",     "Pitch",          -12.f,   12.f,    0.f,     0.f,    Unit::St },
        { "detune",    "Detune",          0.f,    100.f,   12.f,    0.f,    Unit::Ct },
        { "harmony",   "Harmony",         0.f,    1.f,     0.35f,   0.f,    Unit::Pct },
        { "blinkRate", "Blink Rate",      0.1f,   8.f,     1.2f,    1.5f,   Unit::PerSec },
        { "blinkAmt",  "Sparkle",         0.f,    1.f,     0.5f,    0.f,    Unit::Pct },
        { "octave",    "Octave",          0.f,    1.f,     0.25f,   0.f,    Unit::Pct },
        { "tape",      "Tape",            0.f,    1.f,     0.3f,    0.f,    Unit::Pct },
        { "glitch",    "Glitch",          0.f,    1.f,     0.05f,   0.f,    Unit::Pct },
        { "tone",      "Tone",            200.f,  20000.f, 12000.f, 2500.f, Unit::Hz },
        { "attack",    "Attack",          0.01f,  8.f,     1.5f,    1.f,    Unit::Sec },
        { "release",   "Release",         0.05f,  12.f,    3.f,     2.f,    Unit::Sec },
        { "revMix",    "Reverb Mix",      0.f,    1.f,     0.45f,   0.f,    Unit::Pct },
        { "revSize",   "Reverb Size",     0.f,    1.f,     0.7f,    0.f,    Unit::Pct },
        { "revDamp",   "Reverb Damping",  0.f,    1.f,     0.4f,    0.f,    Unit::Pct },
        { "revPre",    "Reverb Pre-Delay",0.f,    200.f,   20.f,    0.f,    Unit::Ms },
        { "revWidth",  "Reverb Width",    0.f,    1.f,     1.f,     0.f,    Unit::Pct },
        { "shimmer",   "Shimmer",         0.f,    1.f,     0.25f,   0.f,    Unit::Pct },
        { "dlyTime",   "Delay Time",      20.f,   1500.f,  380.f,   400.f,  Unit::Ms },
        { "dlyFb",     "Delay Feedback",  0.f,    0.95f,   0.4f,    0.f,    Unit::Pct },
        { "dlyMix",    "Delay Mix",       0.f,    1.f,     0.18f,   0.f,    Unit::Pct },
        { "tremRate",  "Tremolo Rate",    0.1f,   12.f,    4.f,     2.f,    Unit::HzFine },
        { "tremDepth", "Tremolo Depth",   0.f,    1.f,     0.f,     0.f,    Unit::Pct },
        { "tremPhase", "Tremolo Stereo",  0.f,    1.f,     0.5f,    0.f,    Unit::Pct },
        { "master",    "Master",         -24.f,   6.f,     0.f,     0.f,    Unit::Db },
    };
    return s;
}

struct ChoiceSpec { juce::String id, name; juce::StringArray items; int def; };

inline const std::vector<ChoiceSpec>& choiceSpecs()
{
    static const std::vector<ChoiceSpec> s = {
        { "library",   "Library",       kLibraries, 0 },
        { "color",     "Harmony Color", { "Sus", "Major", "Minor", "Lydian" }, 0 },
        { "revType",   "Reverb Type",   { "Hall", "Plate", "Room", "Cathedral", "Shimmer", "Infinite" }, 0 },
        { "dlyType",   "Delay Type",    { "Stereo", "Ping-Pong", "Tape" }, 0 },
        { "dlySync",   "Delay Sync",    { "Free", "1/2", "1/4", "1/8 Dotted", "1/8", "1/16" }, 2 },
        { "tremShape", "Tremolo Shape", { "Sine", "Triangle", "Square" }, 0 },
        { "shimInt",   "Shimmer Interval", { "Oct Up", "5th Up", "Oct Down" }, 0 },
    };
    return s;
}

inline const juce::StringArray& choiceItems (const juce::String& id)
{
    for (auto& c : choiceSpecs()) if (c.id == id) return c.items;
    static const juce::StringArray empty; return empty;
}

inline juce::String formatValue (Unit u, float v)
{
    switch (u)
    {
        case Unit::Pct:     return juce::String (juce::roundToInt (v * 100.f)) + "%";
        case Unit::Ms:      return juce::String (juce::roundToInt (v)) + " ms";
        case Unit::Hz:      return v >= 1000.f ? juce::String (v / 1000.f, 1) + "k" : juce::String (juce::roundToInt (v)) + " Hz";
        case Unit::HzFine:  return juce::String (v, 2) + " Hz";
        case Unit::Sec:     return juce::String (v, 2) + " s";
        case Unit::St:      return juce::String (v > 0.05f ? "+" : "") + juce::String (v, 1) + " st";
        case Unit::Ct:      return juce::String (juce::roundToInt (v)) + " ct";
        case Unit::Db:      return juce::String (v, 1) + " dB";
        case Unit::PerSec:  return juce::String (v, 1) + "/s";
    }
    return {};
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    for (auto& f : floatSpecs())
    {
        juce::NormalisableRange<float> r (f.lo, f.hi);
        if (f.skewMid > 0.f) r.setSkewForCentre (f.skewMid);
        const auto u = f.unit;
        p.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { f.id, 1 }, f.name, r, f.def,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction ([u] (float v, int) { return formatValue (u, v); })));
    }
    for (auto& c : choiceSpecs())
        p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { c.id, 1 }, c.name, c.items, c.def));
    return { p.begin(), p.end() };
}

// ---------------------------------------------------------------- presets
struct Preset { juce::String name; int lib; std::vector<std::pair<juce::String, float>> values; };

struct StyleDef
{
    const char* name;
    float size, density, position, scan, spread, detune, harmony, blinkRate, blinkAmt, octave, tape, glitch,
          tone, attack, release, revMix, revSize, revDamp, shimmer, dlyMix, dlyFb, tremDepth, tremRate;
    int color, revType, dlyType, dlySync;
};

inline const std::vector<Preset>& presets()
{
    static const std::vector<Preset> list = []
    {
        const StyleDef styles[] = {
            { "Halo",       380, 22, 0.30f, 0.35f, 0.80f,  8, 0.35f, 0.9f, 0.45f, 0.20f, 0.15f, 0.00f,  11000, 2.5f, 4.5f, 0.45f, 0.75f, 0.40f, 0.25f, 0.18f, 0.40f, 0.00f, 4.0f, 0, 0, 0, 2 },
            { "Drift",      250, 34, 0.50f, 0.80f, 1.00f, 18, 0.55f, 1.8f, 0.55f, 0.30f, 0.55f, 0.02f,   8500, 3.5f, 6.0f, 0.50f, 0.85f, 0.50f, 0.35f, 0.30f, 0.55f, 0.15f, 0.6f, 1, 4, 2, 4 },
            { "Cinema",     600, 14, 0.15f, 0.45f, 0.90f, 12, 0.45f, 0.5f, 0.60f, 0.35f, 0.30f, 0.00f,   6500, 4.0f, 8.0f, 0.60f, 0.95f, 0.55f, 0.20f, 0.22f, 0.50f, 0.08f, 0.3f, 2, 3, 1, 3 },
            { "Glitch Haze", 90, 55, 0.60f, 1.00f, 1.00f, 25, 0.50f, 3.5f, 0.50f, 0.25f, 0.70f, 0.35f,   9500, 0.6f, 3.0f, 0.40f, 0.70f, 0.35f, 0.30f, 0.35f, 0.60f, 0.45f, 6.0f, 3, 1, 1, 5 },
        };
        struct LibTweak { float sizeMul, toneMul; };
        const LibTweak tweaks[10] = { {1.f,1.f}, {1.f,1.f}, {1.1f,.9f}, {.55f,1.f}, {1.f,.7f},
                                      {1.3f,1.1f}, {1.f,.85f}, {.6f,1.1f}, {.6f,1.f}, {1.2f,1.2f} };
        std::vector<Preset> out;
        for (int lib = 0; lib < 10; ++lib)
            for (auto& s : styles)
            {
                Preset p;
                p.lib = lib;
                p.name = kLibraries[lib] + " - " + s.name;
                auto add = [&p] (const char* id, float v) { p.values.push_back ({ id, v }); };
                add ("size", juce::jlimit (20.f, 800.f, s.size * tweaks[lib].sizeMul));
                add ("density", s.density);   add ("position", s.position); add ("scan", s.scan);
                add ("spread", s.spread);     add ("detune", s.detune);     add ("harmony", s.harmony);
                add ("blinkRate", s.blinkRate); add ("blinkAmt", s.blinkAmt); add ("octave", s.octave);
                add ("tape", s.tape);         add ("glitch", s.glitch);
                add ("tone", juce::jlimit (200.f, 20000.f, s.tone * tweaks[lib].toneMul));
                add ("attack", s.attack);     add ("release", s.release);
                add ("revMix", s.revMix);     add ("revSize", s.revSize);   add ("revDamp", s.revDamp);
                add ("shimmer", s.shimmer);   add ("dlyMix", s.dlyMix);     add ("dlyFb", s.dlyFb);
                add ("tremDepth", s.tremDepth); add ("tremRate", s.tremRate);
                add ("pitch", 0.f); add ("revPre", 20.f); add ("revWidth", 1.f); add ("dlyTime", 380.f);
                add ("tremPhase", 0.5f); add ("master", 0.f);
                add ("color", (float) ((s.color + lib / 3) % 4));
                add ("revType", (float) s.revType); add ("dlyType", (float) s.dlyType); add ("dlySync", (float) s.dlySync);
                add ("shimInt", 0.f);
                out.push_back (std::move (p));
            }
        return out;
    }();
    return list;
}

inline void applyPreset (juce::AudioProcessorValueTreeState& apvts, const Preset& pr)
{
    auto set = [&apvts] (const juce::String& id, float v)
    {
        if (auto* prm = apvts.getParameter (id)) prm->setValueNotifyingHost (prm->convertTo0to1 (v));
    };
    set ("library", (float) pr.lib);
    for (auto& kv : pr.values) set (kv.first, kv.second);
}
} // namespace ptzr
