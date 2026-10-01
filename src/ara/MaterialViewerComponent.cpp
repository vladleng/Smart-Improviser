#include "ara/MaterialViewerComponent.h"
#include "core/analysis/FretboardLayout.h"

#include <array>
#include <cmath>

namespace
{
using namespace smartimproviser::harmony;
const auto sourceColour = juce::Colour::fromRGB(189, 197, 210);
const auto chordColour = juce::Colour::fromRGB(91, 158, 223);
const auto guideColour = juce::Colour::fromRGB(87, 213, 188);
const auto characteristicColour = juce::Colour::fromRGB(245, 180, 91);
const auto targetColour = juce::Colour::fromRGB(185, 153, 242);

juce::Colour noteColour(const ViewerNote& note)
{
    if (note.roles & targetRole) return targetColour;
    if (note.roles & guideRole) return guideColour;
    if (note.roles & characteristicRole) return characteristicColour;
    if (note.roles & chordRole) return chordColour;
    return sourceColour;
}

int wrap(int n, int modulus) { return (n % modulus + modulus) % modulus; }

// Register is an illustrative ascending octave of source intervals, not Core
// rhythm, an inferred phrase, or a prescribed position on the instrument.
int illustrativeMidi(const ViewerNote& note, int rootPc)
{
    return 60 + rootPc + (note.sourceInterval >= 0 ? note.sourceInterval :
        wrap(note.pitchClass - rootPc, 12));
}

int staffStep(const std::string& spelling, int midi)
{
    static constexpr int naturalPc[] = {0, 2, 4, 5, 7, 9, 11};
    const std::string letters = "CDEFGAB";
    const auto index = letters.find(spelling.empty() ? 'C' : spelling[0]);
    if (index == std::string::npos) return 0;
    int accidental = 0;
    for (std::size_t i = 1; i < spelling.size(); ++i)
        accidental += spelling[i] == '#' ? 1 : spelling[i] == 'b' ? -1 : 0;
    const int octave = (midi - naturalPc[index] - accidental) / 12 - 1;
    return (octave - 4) * 7 + static_cast<int>(index) - 2; // E4 = 0
}

const std::string& visibleSpelling(const ViewerNote& note, ViewerLayer layer)
{
    if ((layer == ViewerLayer::chord || layer == ViewerLayer::guides
         || layer == ViewerLayer::characteristic) && ! note.chordSpelling.empty())
        return note.chordSpelling;
    return note.spelling;
}

void drawNote(juce::Graphics& g, const ViewerNote& note, const std::string& spelling,
              float x, float y, bool target)
{
    const auto colour = noteColour(note);
    g.setColour(colour);
    g.fillEllipse(x - 6.0f, y - 5.0f, 12.0f, 10.0f);
    if (target)
    {
        g.setColour(juce::Colour::fromRGB(31, 33, 37));
        g.fillEllipse(x - 3.0f, y - 2.0f, 6.0f, 4.0f);
    }
    g.setColour(juce::Colour::fromRGB(225, 230, 238));
    g.setFont(11.5f);
    g.drawText(juce::String::fromUTF8(spelling.c_str()),
               juce::Rectangle<float>(x - 19.0f, y + 7.0f, 38.0f, 14.0f),
               juce::Justification::centred);
}
}

void MaterialViewerComponent::showMaterial(MaterialView material, ViewerLayer visibleLayer,
                                           int firstFret, int lastFret)
{
    view = std::move(material);
    layer = visibleLayer;
    fretStart = firstFret;
    fretEnd = lastFret;
    repaint();
}

void MaterialViewerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour::fromRGB(42, 46, 53));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour::fromRGB(225, 230, 238));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    if (! view.valid)
    {
        g.drawText(juce::String::fromUTF8("Нет материала для текущего аккорда"),
                   bounds.reduced(18), juce::Justification::centred);
        return;
    }
    g.drawFittedText(juce::String::fromUTF8(view.sourceName.c_str()),
                     16, 6, getWidth() - 30, 22, juce::Justification::centredLeft, 1);

    juce::String aliases;
    for (const auto& note : view.current)
        if (! note.chordSpelling.empty() && note.spelling != note.chordSpelling)
        {
            if (aliases.isNotEmpty()) aliases += "  •  ";
            aliases += juce::String::fromUTF8(note.spelling.c_str()) + " = "
                + juce::String::fromUTF8(note.chordSpelling.c_str());
        }
    if (aliases.isNotEmpty())
    {
        g.setColour(juce::Colour::fromRGB(183, 191, 204));
        g.setFont(11.5f);
        const auto chordName = juce::String::fromUTF8(
            smartimproviser::harmony::normalizedChordSymbol(view.chord).c_str());
        g.drawFittedText(juce::String::fromUTF8("Источник / на ") + chordName + ": " + aliases,
                         18, 28, getWidth() - (view.targets.empty() ? 36 : 205), 17,
                         juce::Justification::centredLeft, 1);
    }

    constexpr float bottom = 102.0f;
    g.setColour(juce::Colour::fromRGB(121, 128, 139));
    for (int i = 0; i < 5; ++i)
        g.drawLine(54.0f, bottom - i * 10.0f, static_cast<float>(getWidth() - 18), bottom - i * 10.0f);
    g.setFont(13.0f);
    g.drawText("G", 20, 65, 28, 22, juce::Justification::centred);

    std::vector<const ViewerNote*> current, targets;
    for (const auto& n : view.current)
        if (visibleInLayer(n, layer) && layer != ViewerLayer::targets) current.push_back(&n);
    if (layer == ViewerLayer::all || layer == ViewerLayer::targets)
        for (const auto& n : view.targets) targets.push_back(&n);

    const float usable = static_cast<float>(getWidth() - 90);
    const float targetWidth = targets.empty() ? 0.0f : juce::jmin(150.0f, usable * 0.3f);
    const float sourceWidth = usable - targetWidth;
    const auto drawStaffGroup = [&](const std::vector<const ViewerNote*>& notes, float left,
                                    float width, int root, bool target)
    {
        for (std::size_t i = 0; i < notes.size(); ++i)
        {
            const auto& note = *notes[i];
            const float x = left + (static_cast<float>(i) + 0.5f) * width / static_cast<float>(notes.size());
            const int midi = illustrativeMidi(note, root);
            const auto& spelling = visibleSpelling(note, layer);
            const float y = bottom - 5.0f * static_cast<float>(staffStep(spelling, midi));
            if (y > bottom + 5.0f || y < bottom - 45.0f)
            {
                g.setColour(juce::Colour::fromRGB(121, 128, 139));
                if (y > bottom) for (float ledger = bottom + 10.0f; ledger <= y; ledger += 10.0f)
                    g.drawLine(x - 9.0f, ledger, x + 9.0f, ledger);
                if (y < bottom - 40.0f) for (float ledger = bottom - 50.0f; ledger >= y; ledger -= 10.0f)
                    g.drawLine(x - 9.0f, ledger, x + 9.0f, ledger);
            }
            drawNote(g, note, spelling, x, y, target);
        }
    };
    drawStaffGroup(current, 60.0f, sourceWidth,
                   view.sourceRootPitchClass >= 0 ? view.sourceRootPitchClass : view.chord.rootPitchClass,
                   false);
    if (! targets.empty())
    {
        g.setColour(targetColour);
        g.drawLine(60.0f + sourceWidth, 43.0f, 60.0f + sourceWidth, 120.0f);
        g.setFont(11.0f);
        g.drawText(juce::String::fromUTF8("след. аккорд"),
                   static_cast<int>(60.0f + sourceWidth), 28,
                   static_cast<int>(targetWidth), 15, juce::Justification::centred);
        drawStaffGroup(targets, 60.0f + sourceWidth, targetWidth,
                       view.nextChord.rootPitchClass, true);
    }

    // Standard guitar tuning, high E to low E. All available positions in the
    // selected range are displayed, without inventing a preferred fingering.
    const float left = 60.0f, right = static_cast<float>(getWidth() - 24);
    const float top = 157.0f, spacing = 18.0f;
    const float fretWidth = (right - left) / static_cast<float>(fretEnd - fretStart + 1);
    if (fretStart == 0 && fretEnd >= 1)
    {
        g.setColour(juce::Colour::fromRGB(49, 55, 63));
        g.fillRect(left, top - 7.0f, fretWidth, 5 * spacing + 14.0f);
    }
    for (int string = 0; string < 6; ++string)
    {
        const float y = top + string * spacing;
        g.setColour(juce::Colour::fromRGB(124, 131, 141));
        g.drawLine(left, y, right, y, string == 0 || string == 5 ? 1.2f : 0.8f);
        static constexpr const char* names[] = {"E", "B", "G", "D", "A", "E"};
        g.drawText(names[string], 27, static_cast<int>(y - 9), 27, 18, juce::Justification::centred);
    }
    for (int fret = fretStart; fret <= fretEnd + 1; ++fret)
    {
        const bool nut = fretStart == 0 && fret == 1;
        const float x = nut ? nutBoundary(fretStart, left, fretWidth)
                            : left + (fret - fretStart) * fretWidth;
        g.setColour(nut ? juce::Colour::fromRGB(230, 219, 193)
                        : juce::Colour::fromRGB(101, 108, 119));
        g.drawLine(x, top - 7.0f, x, top + 5 * spacing + 7.0f,
                   nut ? 4.0f : 0.8f);
    }
    g.setColour(sourceColour);
    g.setFont(11.0f);
    for (int fret = fretStart; fret <= fretEnd; ++fret)
    {
        const auto label = fret == 0 ? juce::String::fromUTF8("откр.")
                                     : juce::String(positionMarker(fret));
        if (label.isEmpty()) continue;
        const float x = fretCellCenter(fretStart, fret, left, fretWidth);
        g.drawText(label, static_cast<int>(x - fretWidth * 0.5f), 135,
                   static_cast<int>(fretWidth), 17, juce::Justification::centred);
    }
    static constexpr std::array<int, 6> openMidi {64, 59, 55, 50, 45, 40};
    for (int string = 0; string < 6; ++string)
        for (int fret = fretStart; fret <= fretEnd; ++fret)
        {
            const int pc = (openMidi[static_cast<std::size_t>(string)] + fret) % 12;
            const auto match = [pc](const ViewerNote* n) { return n->pitchClass == pc; };
            const auto currentIt = std::find_if(current.begin(), current.end(), match);
            const auto targetIt = std::find_if(targets.begin(), targets.end(), match);
            if (currentIt == current.end() && targetIt == targets.end()) continue;
            const float x = fretCellCenter(fretStart, fret, left, fretWidth);
            const float y = top + string * spacing;
            const auto& note = **(currentIt != current.end() ? currentIt : targetIt);
            g.setColour(noteColour(note));
            g.fillEllipse(x - 11.0f, y - 10.0f, 22.0f, 20.0f);
            if (targetIt != targets.end())
            {
                g.setColour(targetColour);
                g.drawEllipse(x - 13.0f, y - 12.0f, 26.0f, 24.0f, 1.5f);
            }
            g.setColour(juce::Colour::fromRGB(22, 27, 32));
            g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
            g.drawFittedText(juce::String::fromUTF8(visibleSpelling(note, layer).c_str()),
                             static_cast<int>(x - 11.0f), static_cast<int>(y - 8.0f),
                             22, 16, juce::Justification::centred, 1);
        }

    g.setFont(11.0f);
    const std::array<std::pair<const char*, juce::Colour>, 5> legend {{
        {"аккорд", chordColour}, {"направляющие", guideColour},
        {"характерные", characteristicColour}, {"источник", sourceColour},
        {"цели →", targetColour}
    }};
    float x = 26.0f;
    for (const auto& [name, colour] : legend)
    {
        g.setColour(colour);
        g.fillEllipse(x, 265.0f, 8.0f, 8.0f);
        g.setColour(sourceColour);
        g.drawText(juce::String::fromUTF8(name), static_cast<int>(x + 12), 260, 100, 18,
                   juce::Justification::centredLeft);
        x += 130.0f;
    }
}
