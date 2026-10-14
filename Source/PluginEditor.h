#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <vector>

namespace ui
{
inline const juce::Colour bg     { 0xfff4f3ef };
inline const juce::Colour panel  { 0xffffffff };
inline const juce::Colour ink    { 0xff111111 };
inline const juce::Colour grey   { 0xff8d8b85 };
inline const juce::Colour line   { 0xffdcd9d1 };

inline juce::Font mono (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}
}

class PtzrLNF : public juce::LookAndFeel_V4
{
public:
    PtzrLNF();
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
};

class CloudView;

class PtzrEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PtzrEditor (PtzrProcessor&);
    ~PtzrEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct CellDef { bool combo; const char* id; const char* label; ptzr::Unit unit; };
    struct SectionDef { juce::String title; std::vector<CellDef> cells; };
    struct Section { juce::String title; juce::Rectangle<int> area; std::vector<juce::Component*> cells; };

    void timerCallback() override;

    PtzrProcessor& proc;
    PtzrLNF lnf;
    juce::Image logo;
    std::unique_ptr<CloudView> cloud;
    juce::ComboBox presetBox, libraryBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> libAtt;
    std::vector<std::unique_ptr<juce::Component>> cells;
    std::vector<std::vector<Section>> rows;
    juce::Rectangle<int> headerLogo, displayArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PtzrEditor)
};
