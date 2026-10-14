#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"
#include "Engine.h"
#include "Fx.h"

class PtzrProcessor : public juce::AudioProcessor, private juce::Timer
{
public:
    PtzrProcessor();
    ~PtzrProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PTZR CLOUD"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override { return (int) ptzr::presets().size(); }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> outLevel { 0.f };
    float getActivity() const { return engine.getActivity(); }
    int getBlinkCount() const { return engine.blinkCount.load(); }
    bool isLoading() const { return loading.load(); }

private:
    void timerCallback() override;
    void startLoad (int lib);
    float param (const char* id) const { return apvts.getRawParameterValue (id)->load(); }

    int currentPreset = 0;
    double sr = 44100.0;

    ptzr::GranularEngine engine;
    ptzr::Tremolo tremolo;
    ptzr::StereoDelay delay;
    ptzr::ReverbFx reverb;
    juce::dsp::StateVariableTPTFilter<float> lowpass;
    juce::SmoothedValue<float> masterGain;

    juce::SpinLock srcLock;
    std::shared_ptr<const ptzr::SourceBuffer> pending, retired;
    std::atomic<int> requestedLib { -1 };
    std::atomic<bool> loading { false };

    juce::ThreadPool pool { 1 };   // must stay last (destroyed first)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PtzrProcessor)
};
