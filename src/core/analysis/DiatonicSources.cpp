#include "core/analysis/DiatonicSources.h"
#include <array>
#include <algorithm>
#include <utility>

namespace smartimproviser::harmony
{
namespace
{
struct ModeDefinition
{
    DiatonicMode mode;
    const char* name;
    const char* rule;
    std::array<int, 7> intervals;
};
constexpr std::array<ModeDefinition, 7> modes {{
    {DiatonicMode::ionian, "Ionian", "diatonic.ionian", {0,2,4,5,7,9,11}},
    {DiatonicMode::dorian, "Dorian", "diatonic.dorian", {0,2,3,5,7,9,10}},
    {DiatonicMode::phrygian, "Phrygian", "diatonic.phrygian", {0,1,3,5,7,8,10}},
    {DiatonicMode::lydian, "Lydian", "diatonic.lydian", {0,2,4,6,7,9,11}},
    {DiatonicMode::mixolydian, "Mixolydian", "diatonic.mixolydian", {0,2,4,5,7,9,10}},
    {DiatonicMode::aeolian, "Aeolian", "diatonic.aeolian", {0,2,3,5,7,8,10}},
    {DiatonicMode::locrian, "Locrian", "diatonic.locrian", {0,1,3,5,6,8,10}}
}};
int wrap(int value, int modulus) { return (value % modulus + modulus) % modulus; }
std::string spell(int rootFifths, int degree, int pitchClass)
{
    static constexpr const char* letters[] = {"C","D","E","F","G","A","B"};
    static constexpr int natural[] = {0,2,4,5,7,9,11};
    const int letter = wrap(4 * wrap(rootFifths, 7) + degree - 1, 7);
    int accidental = wrap(pitchClass - natural[letter], 12);
    if (accidental > 6) accidental -= 12;
    return std::string(letters[letter]) + std::string(static_cast<std::size_t>(accidental < 0 ? -accidental : accidental),
                                                    accidental < 0 ? 'b' : '#');
}
bool containsChord(const ModeDefinition& mode, const NormalizedChord& chord)
{
    for (int i = 0; i < 12; ++i)
        if (chord.hasTone(i) && std::find(mode.intervals.begin(), mode.intervals.end(), i) == mode.intervals.end())
            return false;
    // Explicit degree spelling wins over enharmonic pitch-class coincidence.
    for (int degree = 1; degree <= 7; ++degree)
    {
        const auto interval = mode.intervals[static_cast<std::size_t>(degree - 1)];
        const int explicitDegree = chord.degrees[static_cast<std::size_t>(interval)];
        if (chord.hasTone(interval) && explicitDegree != 0 && (explicitDegree - 1) % 7 + 1 != degree)
            return false;
    }
    return true;
}
}

void addDiatonicSource(ImprovisationResult& result)
{
    if (!result.valid || result.strategies.empty()) return;
    const auto& situation = result.context;
    const auto index = situation.primaryInterpretationIndex;
    if (index < 0 || index >= situation.interpretationCount
        || static_cast<std::size_t>(index) >= situation.interpretations.size()
        || !situation.interpretations[static_cast<std::size_t>(index)].valid)
    {
        result.scaleUnavailableReason = "No primary interpretation: use the explicit chord tones.";
        return;
    }
    const auto& interpretation = situation.interpretations[static_cast<std::size_t>(index)];
    const auto& chord = situation.currentChord;
    if (chord.quality == ChordQuality::suspended2 || chord.quality == ChordQuality::suspended4
        || chord.quality == ChordQuality::power || chord.quality == ChordQuality::noThird
        || chord.quality == ChordQuality::unknown)
    {
        result.scaleUnavailableReason = "This chord needs a dedicated source rule; retain its explicit tones.";
        return;
    }
    const ModeDefinition* selected = nullptr;
    bool targetBased = false;
    // A leading-tone half-diminished chord can have dominant function without
    // being a dominant-seventh source case. Keep its diatonic Locrian material.
    if (chord.quality == ChordQuality::dominant || result.substituteDominant)
    {
        if (result.substituteDominant || interpretation.harmonic.substituteDominantCandidate
            || situation.harmonic.substituteDominantConfirmed)
            result.scaleUnavailableReason = "SubV source rules follow in 0.3d; retain the confirmed targets.";
        else if (result.dominantContext == DominantContext::toMinor)
            result.scaleUnavailableReason = "Minor-target dominant: dedicated minor/altered sources follow in 0.3d.";
        else if (result.dominantContext != DominantContext::toMajor)
            result.scaleUnavailableReason = "No confirmed major target for the basic dominant source.";
        else
        {
            selected = &modes[4]; // Ordinary V -> major: basic Mixolydian.
            targetBased = true;
        }
        if (!selected) return;
    }
    else
    {
        const auto& center = interpretation.center;
        if (!center.valid || !center.key.valid || (center.key.mode != KeyMode::major && center.key.mode != KeyMode::minor))
        {
            result.scaleUnavailableReason = "No supported major/minor center in the selected interpretation.";
            return;
        }
        for (const auto& mode : modes)
        {
            bool match = true;
            for (int interval = 0; interval < 12; ++interval)
            {
                const bool inMode = std::find(mode.intervals.begin(), mode.intervals.end(), interval) != mode.intervals.end();
                if (inMode != center.key.hasPitchClass((chord.rootPitchClass + interval) % 12)) { match = false; break; }
            }
            if (match) { selected = &mode; break; }
        }
    }
    if (!selected || !containsChord(*selected, chord))
    {
        result.scaleUnavailableReason = "No compatible diatonic source for all explicit chord tones/degrees.";
        return;
    }

    auto strategy = result.strategies.front(); // Preserve structural tones, targets and resolution.
    strategy.kind = ImprovisationStrategyKind::diatonicColor;
    strategy.ruleId = selected->rule;
    strategy.ruleVersion = 1;
    strategy.priority = 50;
    strategy.interpretationIndependent = false;
    strategy.interpretationIndex = index;
    strategy.evidence = interpretation.evidence;
    strategy.source = {};
    strategy.source.kind = MaterialKind::scale;
    strategy.source.mode = selected->mode;
    strategy.source.rootPitchClass = chord.rootPitchClass;
    strategy.source.rootFifths = chord.rootFifths;
    strategy.source.name = spell(chord.rootFifths, 1, chord.rootPitchClass) + " " + selected->name;
    for (int degree = 1; degree <= 7; ++degree)
    {
        const int interval = selected->intervals[static_cast<std::size_t>(degree - 1)];
        MaterialNote note;
        note.pitchClass = (chord.rootPitchClass + interval) % 12;
        note.semitonesFromRoot = interval;
        note.degree = degree;
        note.role = MaterialNoteRole::scaleTone;
        for (const auto& anchor : result.strategies.front().source.notes)
            if (anchor.pitchClass == note.pitchClass) { note.role = anchor.role; note.characteristic = anchor.characteristic; break; }
        note.spelling = spell(chord.rootFifths, degree, note.pitchClass);
        strategy.source.notes.push_back(std::move(note));
    }
    strategy.idea = "Connect the chord anchors using " + strategy.source.name + ".";
    strategy.explanation = targetBased ? "Basic dominant material for the confirmed major target."
        : "Diatonic material of the selected center "
            + spell(interpretation.center.key.rootFifths, 1, interpretation.center.key.rootPitchClass)
            + " " + keyModeName(interpretation.center.key.mode) + ", starting from the chord root.";
    strategy.conditions = "Scale notes are available material, not equally stable landing notes; use the chord anchors and targets.";
    if (chord.hasTone(4) && std::find(selected->intervals.begin(), selected->intervals.end(), 5) != selected->intervals.end())
        strategy.conditions += " Treat the natural 4th as a passing tone against the major 3rd.";
    result.strategies.push_back(std::move(strategy));
}
}
