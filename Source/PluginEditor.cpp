#include "PluginEditor.h"
#include <BinaryData.h>

using namespace ui;

//==============================================================================
static juce::Path pixelPanelPath (juce::Rectangle<float> r, float s = 3.f)
{
    const float x = r.getX(), y = r.getY(), rt = r.getRight(), b = r.getBottom();
    juce::Path p;
    p.startNewSubPath (x, y + 2 * s);
    p.lineTo (x + s, y + 2 * s);  p.lineTo (x + s, y + s);  p.lineTo (x + 2 * s, y + s);  p.lineTo (x + 2 * s, y);
    p.lineTo (rt - 2 * s, y);
    p.lineTo (rt - 2 * s, y + s); p.lineTo (rt - s, y + s); p.lineTo (rt - s, y + 2 * s); p.lineTo (rt, y + 2 * s);
    p.lineTo (rt, b - 2 * s);
    p.lineTo (rt - s, b - 2 * s); p.lineTo (rt - s, b - s); p.lineTo (rt - 2 * s, b - s); p.lineTo (rt - 2 * s, b);
    p.lineTo (x + 2 * s, b);
    p.lineTo (x + 2 * s, b - s);  p.lineTo (x + s, b - s);  p.lineTo (x + s, b - 2 * s);  p.lineTo (x, b - 2 * s);
    p.closeSubPath();
    return p;
}

static void drawPixelPanel (juce::Graphics& g, juce::Rectangle<int> r, juce::Colour fill, juce::Colour stroke)
{
    auto p = pixelPanelPath (r.toFloat().reduced (0.5f));
    g.setColour (fill);   g.fillPath (p);
    g.setColour (stroke); g.strokePath (p, juce::PathStrokeType (1.f));
}

//==============================================================================
PtzrLNF::PtzrLNF()
{
    setColour (juce::ComboBox::backgroundColourId, panel);
    setColour (juce::ComboBox::outlineColourId, line);
    setColour (juce::ComboBox::textColourId, ink);
    setColour (juce::ComboBox::arrowColourId, ink);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::textColourId, ink);
    setColour (juce::PopupMenu::headerTextColourId, grey);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ink);
    setColour (juce::PopupMenu::highlightedTextColourId, panel);
    setColour (juce::Label::textColourId, ink);
}
juce::Font PtzrLNF::getComboBoxFont (juce::ComboBox&) { return mono (12.f, true); }
juce::Font PtzrLNF::getPopupMenuFont() { return mono (12.f); }
juce::Font PtzrLNF::getTextButtonFont (juce::TextButton&, int) { return mono (14.f, true); }

void PtzrLNF::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
    g.setColour (panel);  g.fillRoundedRectangle (r, 3.f);
    g.setColour (box.hasKeyboardFocus (false) ? ink : line); g.drawRoundedRectangle (r, 3.f, 1.f);
    const float cx = (float) w - 14.f, cy = (float) h * 0.5f;
    juce::Path t; t.addTriangle (cx - 4.f, cy - 2.f, cx + 4.f, cy - 2.f, cx, cy + 3.f);
    g.setColour (ink); g.fillPath (t);
}
void PtzrLNF::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 1, box.getWidth() - 28, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void PtzrLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float d = juce::jmin (b.getWidth(), b.getHeight());
    auto sq = juce::Rectangle<float> (d, d).withCentre (b.getCentre());
    const auto c = sq.getCentre();
    const float R = d * 0.5f - 1.f;

    // pixel tick ring
    const int ticks = 21;
    for (int i = 0; i < ticks; ++i)
    {
        const float t = (float) i / (float) (ticks - 1);
        const float a = a0 + t * (a1 - a0);
        const float rr = R - 1.f;
        const float px = c.x + rr * std::sin (a), py = c.y - rr * std::cos (a);
        const bool lit = t <= pos + 0.001f;
        g.setColour (lit ? ink : line);
        g.fillRect (std::round (px - 1.f), std::round (py - 1.f), 2.f, 2.f);
    }
    // knob body with soft shadow
    const float kr = R * 0.66f;
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.035f));
        g.fillEllipse (c.x - kr - (float) i, c.y - kr + (float) i + 1.f, (kr + (float) i) * 2.f, (kr + (float) i) * 2.f);
    }
    g.setColour (panel);  g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);
    g.setColour (line);   g.drawEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f, 1.2f);
    // pointer
    const float ang = a0 + pos * (a1 - a0);
    const float s = std::sin (ang), co = std::cos (ang);
    g.setColour (ink);
    g.drawLine (c.x + kr * 0.30f * s, c.y - kr * 0.30f * co, c.x + kr * 0.84f * s, c.y - kr * 0.84f * co, 2.2f);
}

void PtzrLNF::drawButtonBackground (juce::Graphics& g, juce::Button& bt, const juce::Colour&, bool over, bool down)
{
    auto r = bt.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? ink : (over ? juce::Colour (0xfff0eee8) : panel));
    g.fillRoundedRectangle (r, 3.f);
    g.setColour (line); g.drawRoundedRectangle (r, 3.f, 1.f);
    bt.setColour (juce::TextButton::textColourOffId, down ? panel : ink);
    bt.setColour (juce::TextButton::textColourOnId, down ? panel : ink);
}

//==============================================================================
namespace
{
class KnobCell : public juce::Component
{
public:
    KnobCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, const juce::String& label, ptzr::Unit unit)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        addAndMakeVisible (slider);
        for (auto* l : { &name, &val })
        {
            l->setJustificationType (juce::Justification::centred);
            l->setInterceptsMouseClicks (false, false);
            addAndMakeVisible (*l);
        }
        name.setText (label.toUpperCase(), juce::dontSendNotification);
        name.setFont (mono (9.5f, true)); name.setColour (juce::Label::textColourId, grey);
        val.setFont (mono (10.f));        val.setColour (juce::Label::textColourId, ink);
        slider.onValueChange = [this, unit] { val.setText (ptzr::formatValue (unit, (float) slider.getValue()), juce::dontSendNotification); };
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, id, slider);
        slider.onValueChange();
    }
    void resized() override
    {
        auto r = getLocalBounds();
        name.setBounds (r.removeFromBottom (12));
        val.setBounds (r.removeFromBottom (13));
        slider.setBounds (r.reduced (2));
    }
private:
    juce::Slider slider; juce::Label name, val;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

class ComboCell : public juce::Component
{
public:
    ComboCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, const juce::String& label)
    {
        box.addItemList (ptzr::choiceItems (id), 1);
        addAndMakeVisible (box);
        name.setText (label.toUpperCase(), juce::dontSendNotification);
        name.setJustificationType (juce::Justification::centred);
        name.setFont (mono (9.5f, true)); name.setColour (juce::Label::textColourId, grey);
        name.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (name);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, id, box);
    }
    void resized() override
    {
        auto r = getLocalBounds();
        name.setBounds (r.removeFromBottom (12));
        r.removeFromBottom (13);
        box.setBounds (r.withSizeKeepingCentre (juce::jmax (40, r.getWidth() - 8), 24));
    }
private:
    juce::ComboBox box; juce::Label name;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
};
} // namespace

//==============================================================================
// Animated "cloud": pixel particles that react to the grains, pixel stars on every blink
class CloudView : public juce::Component, private juce::Timer
{
public:
    explicit CloudView (PtzrProcessor& p) : proc (p) { startTimerHz (30); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds();
        drawPixelPanel (g, r, panel, line);
        auto in = r.reduced (10, 8);
        g.saveState();
        g.reduceClipRegion (in);
        g.setColour (line.withAlpha (0.7f));
        for (int yy = in.getY(); yy < in.getBottom(); yy += 12)
            for (int xx = in.getX(); xx < in.getRight(); xx += 12) g.fillRect (xx, yy, 1, 1);

        const float W = (float) in.getWidth(), H = (float) in.getHeight();
        for (auto& p : ps)
        {
            const float px = std::floor ((in.getX() + p.x * W) / 3.f) * 3.f, py = std::floor ((in.getY() + p.y * H) / 3.f) * 3.f;
            g.setColour (ink.withAlpha (juce::jlimit (0.f, 1.f, p.life) * (0.25f + 0.55f * p.size)));
            const float sz = 3.f * (p.size > 0.8f ? 2.f : 1.f);
            g.fillRect (px, py, sz, sz);
        }
        for (auto& s : sparks)
        {
            const float cx = std::floor ((in.getX() + s.x * W) / 3.f) * 3.f, cy = std::floor ((in.getY() + s.y * H) / 3.f) * 3.f;
            const int arm = 1 + (int) ((1.f - s.life) * 4.f);
            g.setColour (ink.withAlpha (s.life));
            g.fillRect (cx, cy, 3.f, 3.f);
            for (int k = 1; k <= arm; ++k)
            {
                g.fillRect (cx + 3.f * k, cy, 3.f, 3.f); g.fillRect (cx - 3.f * k, cy, 3.f, 3.f);
                g.fillRect (cx, cy + 3.f * k, 3.f, 3.f); g.fillRect (cx, cy - 3.f * k, 3.f, 3.f);
            }
        }
        g.restoreState();

        const int lib = (int) proc.apvts.getRawParameterValue ("library")->load();
        g.setColour (ink);  g.setFont (mono (12.f, true));
        g.drawText (ptzr::kLibraries[juce::jlimit (0, 9, lib)].toUpperCase(), in.removeFromTop (16), juce::Justification::topLeft);
        g.setColour (grey); g.setFont (mono (10.f));
        g.drawText (proc.isLoading() ? "LOADING SOUND..." : "HOLD A NOTE - THE CLOUD DOES THE REST", in.removeFromBottom (14), juce::Justification::bottomLeft);
    }

private:
    struct P { float x, y, vx, vy, life, size; };
    struct Spark { float x, y, life; };

    void timerCallback() override
    {
        act += (proc.getActivity() - act) * 0.12f;
        const int bc = proc.getBlinkCount();
        if (bc != lastBlink)
        {
            for (int i = 0; i < juce::jmin (3, bc - lastBlink) && sparks.size() < 18; ++i)
                sparks.push_back ({ 0.1f + 0.8f * rng.nextFloat(), 0.2f + 0.6f * rng.nextFloat(), 1.f });
            lastBlink = bc;
        }
        const float level = juce::jlimit (0.f, 1.f, proc.outLevel.load() * 2.f);
        const int spawn = (int) (act * 40.f * (0.3f + level) + rng.nextFloat());
        for (int i = 0; i < spawn && ps.size() < 260; ++i)
        {
            const float gx = (rng.nextFloat() + rng.nextFloat() + rng.nextFloat()) / 3.f;
            const float gy = (rng.nextFloat() + rng.nextFloat() + rng.nextFloat()) / 3.f;
            ps.push_back ({ gx, 0.15f + gy * 0.7f, (rng.nextFloat() - 0.5f) * 0.003f, (rng.nextFloat() - 0.5f) * 0.002f - 0.0007f, 1.f, rng.nextFloat() });
        }
        for (auto& p : ps) { p.x += p.vx; p.y += p.vy; p.life -= 0.014f; }
        ps.erase (std::remove_if (ps.begin(), ps.end(), [] (const P& p) { return p.life <= 0.f; }), ps.end());
        for (auto& s : sparks) s.life -= 0.035f;
        sparks.erase (std::remove_if (sparks.begin(), sparks.end(), [] (const Spark& s) { return s.life <= 0.f; }), sparks.end());
        repaint();
    }

    PtzrProcessor& proc;
    std::vector<P> ps; std::vector<Spark> sparks;
    juce::Random rng;
    int lastBlink = 0; float act = 0.f;
};

//==============================================================================
PtzrEditor::PtzrEditor (PtzrProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);
    logo = juce::ImageCache::getFromMemory (BinaryData::ptzr_logo_png, BinaryData::ptzr_logo_pngSize);

    cloud = std::make_unique<CloudView> (proc);
    addAndMakeVisible (*cloud);

    // preset browser (grouped by library)
    int lastLib = -1;
    const auto& list = ptzr::presets();
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (list[i].lib != lastLib)
        {
            if (lastLib >= 0) presetBox.addSeparator();
            presetBox.addSectionHeading (ptzr::kLibraries[list[i].lib]);
            lastLib = list[i].lib;
        }
        presetBox.addItem (list[i].name, (int) i + 1);
    }
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { const int id = presetBox.getSelectedId(); if (id > 0) proc.setCurrentProgram (id - 1); };
    addAndMakeVisible (presetBox);

    libraryBox.addItemList (ptzr::kLibraries, 1);
    addAndMakeVisible (libraryBox);
    libAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "library", libraryBox);

    prevBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + n - 1) % n); };
    nextBtn.onClick = [this] { const int n = proc.getNumPrograms(); proc.setCurrentProgram ((proc.getCurrentProgram() + 1) % n); };
    addAndMakeVisible (prevBtn); addAndMakeVisible (nextBtn);

    using S = SectionDef;
    auto K = [] (const char* id, const char* label, ptzr::Unit u) { return CellDef { false, id, label, u }; };
    auto C = [] (const char* id, const char* label) { return CellDef { true, id, label, ptzr::Unit::Pct }; };
    using U = ptzr::Unit;

    const std::vector<std::vector<S>> layout = {
        { S { "GRAIN",   { K ("size", "Size", U::Ms), K ("density", "Density", U::PerSec), K ("position", "Position", U::Pct),
                           K ("scan", "Scan", U::Pct), K ("spread", "Spread", U::Pct), K ("pitch", "Pitch", U::St), K ("detune", "Detune", U::Ct) } },
          S { "HARMONY", { C ("color", "Color"), K ("harmony", "Harmony", U::Pct), K ("octave", "Octave", U::Pct),
                           K ("blinkRate", "Blink", U::PerSec), K ("blinkAmt", "Sparkle", U::Pct) } } },
        { S { "REVERB",  { C ("revType", "Type"), K ("revMix", "Mix", U::Pct), K ("revSize", "Size", U::Pct), K ("revDamp", "Damp", U::Pct),
                           K ("revPre", "Pre", U::Ms), K ("revWidth", "Width", U::Pct), K ("shimmer", "Shimmer", U::Pct), C ("shimInt", "Interval") } },
          S { "DELAY",   { C ("dlyType", "Type"), C ("dlySync", "Sync"), K ("dlyTime", "Time", U::Ms), K ("dlyFb", "Feedback", U::Pct), K ("dlyMix", "Mix", U::Pct) } } },
        { S { "TREMOLO", { C ("tremShape", "Shape"), K ("tremRate", "Rate", U::HzFine), K ("tremDepth", "Depth", U::Pct), K ("tremPhase", "Stereo", U::Pct) } },
          S { "TAPE",    { K ("tape", "Wobble", U::Pct), K ("glitch", "Glitch", U::Pct) } },
          S { "TONE / ENV", { K ("tone", "Tone", U::Hz), K ("attack", "Attack", U::Sec), K ("release", "Release", U::Sec) } },
          S { "OUT",     { K ("master", "Master", U::Db) } } },
    };

    for (auto& rowDef : layout)
    {
        std::vector<Section> row;
        for (auto& sd : rowDef)
        {
            Section sec; sec.title = sd.title;
            for (auto& cd : sd.cells)
            {
                std::unique_ptr<juce::Component> c;
                if (cd.combo) c = std::make_unique<ComboCell> (proc.apvts, cd.id, cd.label);
                else          c = std::make_unique<KnobCell> (proc.apvts, cd.id, cd.label, cd.unit);
                addAndMakeVisible (*c);
                sec.cells.push_back (c.get());
                cells.push_back (std::move (c));
            }
            row.push_back (std::move (sec));
        }
        rows.push_back (std::move (row));
    }

    setSize (940, 604);
    startTimerHz (10);
}

PtzrEditor::~PtzrEditor() { setLookAndFeel (nullptr); }

void PtzrEditor::timerCallback()
{
    const int cur = proc.getCurrentProgram();
    if (presetBox.getSelectedId() != cur + 1) presetBox.setSelectedId (cur + 1, juce::dontSendNotification);
}

void PtzrEditor::resized()
{
    auto r = getLocalBounds().reduced (16);
    auto head = r.removeFromTop (46);
    headerLogo = head.removeFromLeft (132);
    head.removeFromLeft (14);

    libraryBox.setBounds (head.removeFromRight (190).withSizeKeepingCentre (190, 28));
    head.removeFromRight (8);
    nextBtn.setBounds (head.removeFromRight (30).withSizeKeepingCentre (30, 28));
    head.removeFromRight (4);
    presetBox.setBounds (head.removeFromRight (330).withSizeKeepingCentre (330, 28));
    head.removeFromRight (4);
    prevBtn.setBounds (head.removeFromRight (30).withSizeKeepingCentre (30, 28));

    r.removeFromTop (10);
    displayArea = r.removeFromTop (138);
    cloud->setBounds (displayArea);
    r.removeFromTop (10);

    const int rowH = (r.getHeight() - 2 * 8) / 3;
    for (auto& row : rows)
    {
        auto rr = r.removeFromTop (rowH);
        r.removeFromTop (8);
        int totalCells = 0;
        for (auto& s : row) totalCells += (int) s.cells.size();
        const int gaps = 8 * ((int) row.size() - 1);
        const float cellW = (float) (rr.getWidth() - gaps - (int) row.size() * 16) / (float) totalCells;
        for (auto& s : row)
        {
            const int w = (int) (cellW * (float) s.cells.size()) + 16;
            s.area = rr.removeFromLeft (w);
            rr.removeFromLeft (8);
            auto inner = s.area.reduced (8, 0).withTrimmedTop (20).withTrimmedBottom (4);
            for (auto* c : s.cells) c->setBounds (inner.removeFromLeft ((int) cellW));
        }
    }
}

void PtzrEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    // logo tile
    drawPixelPanel (g, headerLogo, juce::Colours::black, juce::Colours::black);
    if (logo.isValid())
    {
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (logo, headerLogo.reduced (16, 11).toFloat(), juce::RectanglePlacement::centred);
    }
    else
    {
        g.setColour (juce::Colours::white); g.setFont (mono (24.f, true));
        g.drawText ("ptzr", headerLogo, juce::Justification::centred);
    }
    g.setColour (ink); g.setFont (mono (24.f, true));
    g.drawText ("CLOUD", headerLogo.getRight() + 14, headerLogo.getY(), 120, headerLogo.getHeight(), juce::Justification::centredLeft);

    for (auto& row : rows)
        for (auto& s : row)
        {
            drawPixelPanel (g, s.area, panel, line);
            g.setColour (grey); g.setFont (mono (9.5f, true));
            g.drawText (s.title, s.area.getX() + 14, s.area.getY() + 6, s.area.getWidth() - 20, 12, juce::Justification::centredLeft);
        }
}
