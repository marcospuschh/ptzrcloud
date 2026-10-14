#include "PluginProcessor.h"
#include "PluginEditor.h"

PtzrProcessor::PtzrProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PTZR", ptzr::createLayout())
{
    ptzr::applyPreset (apvts, ptzr::presets()[0]);
    startTimer (100);
}

PtzrProcessor::~PtzrProcessor() { stopTimer(); }

bool PtzrProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PtzrProcessor::startLoad (int lib)
{
    requestedLib = lib;
    loading = true;
    pool.addJob ([this, lib]
    {
        { juce::SpinLock::ScopedLockType l (srcLock); retired.reset(); }
        auto buf = ptzr::renderLibrary (lib);
        { juce::SpinLock::ScopedLockType l (srcLock); pending = std::move (buf); }
        if (requestedLib.load() == lib) loading = false;
    });
}

void PtzrProcessor::timerCallback()
{
    const int lib = (int) param ("library");
    if (lib != requestedLib.load()) startLoad (lib);
}

void PtzrProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    engine.prepare (sr);
    delay.prepare (sr);
    reverb.prepare (sr);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) samplesPerBlock, 2 };
    lowpass.prepare (spec);
    lowpass.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    masterGain.reset (sr, 0.05);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (param ("master")));
    startLoad ((int) param ("library"));
}

void PtzrProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2) return;

    // pick up a freshly rendered sound library (never blocks the audio thread)
    if (srcLock.tryEnter())
    {
        if (pending) { auto old = engine.swapSource (std::move (pending)); pending.reset(); retired = std::move (old); }
        srcLock.exit();
    }

    ptzr::EngineParams ep;
    ep.grainMs = param ("size");   ep.density = param ("density");  ep.position = param ("position");
    ep.scan = param ("scan");      ep.spread = param ("spread");    ep.pitch = param ("pitch");
    ep.detune = param ("detune");  ep.harmony = param ("harmony");  ep.blinkRate = param ("blinkRate");
    ep.blinkAmt = param ("blinkAmt"); ep.octave = param ("octave"); ep.tape = param ("tape");
    ep.glitch = param ("glitch");  ep.attack = param ("attack");    ep.release = param ("release");
    ep.color = (int) param ("color");

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    int pos = 0;
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        const int t = juce::jlimit (pos, n, meta.samplePosition);
        if (t > pos) { engine.render (L + pos, R + pos, t - pos, ep); pos = t; }
        if (m.isNoteOn())                         engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())                   engine.noteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) engine.allNotesOff();
        else if (m.isPitchWheel())                engine.setPitchBend ((float) (m.getPitchWheelValue() - 8192) / 8192.f * 2.f);
        else if (m.isControllerOfType (64))       engine.setSustain (m.getControllerValue() >= 64);
    }
    if (pos < n) engine.render (L + pos, R + pos, n - pos, ep);

    // ---- effects chain: tone -> tremolo -> delay -> reverb -> master
    lowpass.setCutoffFrequency (juce::jmin (param ("tone"), (float) (0.45 * sr)));
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    lowpass.process (ctx);

    tremolo.process (L, R, n, param ("tremRate"), param ("tremDepth"), param ("tremPhase"), (int) param ("tremShape"), sr);

    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto p = ph->getPosition())
            if (auto b = p->getBpm()) bpm = juce::jlimit (30.0, 300.0, *b);
    static const double beats[6] = { 0.0, 2.0, 1.0, 0.75, 0.5, 0.25 };
    const int sync = (int) param ("dlySync");
    const double dlySec = sync == 0 ? param ("dlyTime") * 0.001 : beats[juce::jlimit (0, 5, sync)] * 60.0 / bpm;
    delay.process (L, R, n, (int) param ("dlyType"), dlySec * sr, juce::jmin (0.95f, param ("dlyFb")), param ("dlyMix"), sr);

    reverb.process (L, R, n, (int) param ("revType"), param ("revMix"), param ("revSize"), param ("revDamp"),
                    param ("revPre"), param ("revWidth"), param ("shimmer"), (int) param ("shimInt"), sr);

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (param ("master")));
    float peak = 0.f;
    for (int i = 0; i < n; ++i)
    {
        const float g = masterGain.getNextValue();
        L[i] = std::tanh (L[i] * g); R[i] = std::tanh (R[i] * g);
        peak = juce::jmax (peak, std::abs (L[i]), std::abs (R[i]));
    }
    outLevel.store (peak);
}

void PtzrProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, getNumPrograms() - 1, index);
    currentPreset = index;
    ptzr::applyPreset (apvts, ptzr::presets()[(size_t) index]);
}

const juce::String PtzrProcessor::getProgramName (int index)
{
    return ptzr::presets()[(size_t) juce::jlimit (0, getNumPrograms() - 1, index)].name;
}

void PtzrProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", currentPreset, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void PtzrProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            currentPreset = juce::jlimit (0, getNumPrograms() - 1, (int) vt.getProperty ("preset", 0));
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* PtzrProcessor::createEditor() { return new PtzrEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PtzrProcessor(); }
