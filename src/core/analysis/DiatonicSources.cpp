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
    if (chord.slashBass
        && std::find(mode.intervals.begin(), mode.intervals.end(),
                     wrap(chord.bassPitchClass - chord.rootPitchClass, 12)) == mode.intervals.end())
        return false;
    return true;
}

bool appendForInterpretation(ImprovisationResult& result, int index)
{
    const auto& situation = result.context;
    if (index < 0 || index >= situation.interpretationCount
        || static_cast<std::size_t>(index) >= situation.interpretations.size())
        return false;

    const auto& interpretation = situation.interpretations[static_cast<std::size_t>(index)];
    if (!interpretation.valid)
        return false;

    const auto& chord = situation.currentChord;
    if (chord.quality == ChordQuality::suspended2 || chord.quality == ChordQuality::suspended4
        || chord.quality == ChordQuality::power || chord.quality == ChordQuality::noThird
        || chord.quality == ChordQuality::unknown)
        return false;

    const ModeDefinition* selected = nullptr;
    bool targetBased = false;
    const bool expectedMajor = situation.expectedTonic.valid
        && interpretation.kind == HarmonicInterpretationKind::globalContext
        && interpretation.center.key.rootPitchClass
            == circleOfFifthsToPitchClass(situation.expectedTonic.rootFifths)
        && interpretation.center.key.mode == KeyMode::major;

    // Dominant source selection is interpretation-specific in 0.3f. A SubV
    // reading must not suppress an ordinary dominant alternative (or vice versa).
    if (chord.quality == ChordQuality::dominant)
    {
        if (interpretation.harmonic.substituteDominantConfirmed
            || interpretation.harmonic.substituteDominantCandidate)
            return false;
        if (result.dominantContext == DominantContext::toMinor)
            return false;
        if (result.dominantContext != DominantContext::toMajor && ! expectedMajor)
            return false;
        selected = &modes[4]; // Ordinary V -> major: basic Mixolydian.
        targetBased = result.dominantContext == DominantContext::toMajor;
    }
    else
    {
        const auto& center = interpretation.center;
        if (!center.valid || !center.key.valid
            || (center.key.mode != KeyMode::major && center.key.mode != KeyMode::minor))
            return false;

        for (const auto& mode : modes)
        {
            bool match = true;
            for (int interval = 0; interval < 12; ++interval)
            {
                const bool inMode = std::find(mode.intervals.begin(), mode.intervals.end(), interval) != mode.intervals.end();
                if (inMode != center.key.hasPitchClass((chord.rootPitchClass + interval) % 12))
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                selected = &mode;
                break;
            }
        }
    }

    if (!selected || !containsChord(*selected, chord))
        return false;

    auto strategy = result.strategies.front(); // Preserve structural tones, targets and resolution.
    strategy.kind = ImprovisationStrategyKind::diatonicColor;
    strategy.ruleId = selected->rule;
    strategy.ruleVersion = 2;
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
        {
            if (anchor.pitchClass == note.pitchClass)
            {
                note.role = anchor.role;
                note.characteristic = anchor.characteristic;
                break;
            }
        }
        note.spelling = spell(chord.rootFifths, degree, note.pitchClass);
        strategy.source.notes.push_back(std::move(note));
    }

    strategy.idea = "Connect the chord anchors using " + strategy.source.name + ".";
    strategy.explanation = targetBased
        ? "Basic dominant material for the confirmed major target in this interpretation."
        : (expectedMajor
            ? "Global V material with an absent expected tonic; follow the actual continuation, not a confirmed resolution."
            : "Diatonic material of interpretation " + std::to_string(index + 1) + ": "
            + spell(interpretation.center.key.rootFifths, 1, interpretation.center.key.rootPitchClass)
            + " " + keyModeName(interpretation.center.key.mode) + ", starting from the chord root.");
    strategy.conditions = "Scale notes are available material, not equally stable landing notes; use the chord anchors and targets.";
    if (chord.hasTone(4) && std::find(selected->intervals.begin(), selected->intervals.end(), 5) != selected->intervals.end())
        strategy.conditions += " Treat the natural 4th as a passing tone against the major 3rd.";
    strategy.source.chordRelativeNotes = strategy.source.notes;
    strategy.usageHint = "Use chord anchors and targets.";
    if (chord.hasTone(4) && std::find(selected->intervals.begin(), selected->intervals.end(), 5) != selected->intervals.end())
        strategy.usageHint = "Natural 4th: passing against major 3rd.";

    result.strategies.push_back(std::move(strategy));
    return true;
}

bool appendIncompleteCadenceMode(ImprovisationResult& result)
{
    const auto& situation = result.context;
    const auto& incomplete = situation.incompleteCadence;
    const auto& chord = situation.currentChord;
    if (! incomplete.valid || incomplete.positionIndex < 0 || incomplete.positionIndex > 1
        || ! situation.nextChordAvailable || ! incomplete.actualContinuation.valid
        || situation.resolution.confirmed)
        return false;
    const bool onIi = incomplete.positionIndex == 0;
    const auto& mode = onIi ? modes[1] : modes[4]; // Dorian ii, Mixolydian V.
    if (onIi ? (chord.quality != ChordQuality::minor
                 || chord.rootPitchClass != incomplete.ii.rootPitchClass)
             : (chord.quality != ChordQuality::dominant
                 || chord.rootPitchClass != incomplete.v.rootPitchClass
                 || (chord.rootPitchClass + 5) % 12
                     != circleOfFifthsToPitchClass(incomplete.missingTonicRootFifths)))
        return false;
    if (! containsChord(mode, chord)) return false;

    // The two members share a provisional major center without declaring its
    // absent I played. The actual next chord remains the only factual target.
    auto strategy = result.strategies.front();
    strategy.kind = ImprovisationStrategyKind::diatonicColor;
    strategy.ruleId = mode.rule;
    strategy.ruleVersion = 3;
    strategy.priority = 45;
    strategy.interpretationIndex = -1;
    strategy.interpretationIndependent = false;
    strategy.missingTonicApplication = true;
    strategy.missingTonicRootFifths = incomplete.missingTonicRootFifths;
    strategy.evidence = {};
    strategy.evidence.confidence = ConfidenceLevel::low;
    strategy.evidence.add(EvidenceFlag::patternMatch);
    strategy.source = {};
    strategy.source.kind = MaterialKind::scale;
    strategy.source.mode = mode.mode;
    strategy.source.rootPitchClass = chord.rootPitchClass;
    strategy.source.rootFifths = chord.rootFifths;
    strategy.source.name = spell(chord.rootFifths, 1, chord.rootPitchClass) + " " + mode.name;
    for (int degree = 1; degree <= 7; ++degree)
    {
        const int interval = mode.intervals[static_cast<std::size_t>(degree - 1)];
        MaterialNote note;
        note.pitchClass = (chord.rootPitchClass + interval) % 12;
        note.semitonesFromRoot = interval;
        note.degree = degree;
        note.role = MaterialNoteRole::scaleTone;
        for (const auto& anchor : result.strategies.front().source.notes)
            if (anchor.pitchClass == note.pitchClass)
            {
                note.role = anchor.role;
                note.characteristic = anchor.characteristic;
                break;
            }
        note.spelling = spell(chord.rootFifths, degree, note.pitchClass);
        strategy.source.notes.push_back(std::move(note));
    }
    strategy.source.chordRelativeNotes = strategy.source.notes;
    strategy.idea = "Connect the chord anchors using " + strategy.source.name + ".";
    strategy.explanation = "Major ii-V material on an incomplete cadence; the expected I did not sound.";
    strategy.conditions = "The absent I is a source hypothesis only. Follow the actual next chord; no major resolution is confirmed.";
    strategy.usageHint = "Use chord anchors and targets; the expected I is missing.";
    result.strategies.push_back(std::move(strategy));
    return true;
}

bool appendChordLocalDominant(ImprovisationResult& result)
{
    const auto& chord = result.context.currentChord;
    const auto& mixolydian = modes[4];
    if (chord.quality != ChordQuality::dominant
        || result.dominantContext == DominantContext::toMinor
        || ! containsChord(mixolydian, chord))
        return false;
    for (std::uint8_t i = 0; i < result.context.interpretationCount; ++i)
        if (result.context.interpretations[i].valid
            && (result.context.interpretations[i].harmonic.substituteDominantConfirmed
                || result.context.interpretations[i].harmonic.substituteDominantCandidate))
            return false;

    // Membership of the *written* chord is enough to offer a baseline. This
    // neither chooses an interpretation nor claims that the next chord is I.
    auto strategy = result.strategies.front();
    strategy.kind = ImprovisationStrategyKind::diatonicColor;
    strategy.ruleId = "chord-local.mixolydian";
    strategy.ruleVersion = 1;
    strategy.priority = 45;
    strategy.interpretationIndependent = true;
    strategy.interpretationIndex = -1;
    strategy.evidence = {};
    strategy.source = {};
    strategy.source.kind = MaterialKind::scale;
    strategy.source.mode = mixolydian.mode;
    strategy.source.rootPitchClass = chord.rootPitchClass;
    strategy.source.rootFifths = chord.rootFifths;
    strategy.source.name = spell(chord.rootFifths, 1, chord.rootPitchClass) + " Mixolydian";
    for (int degree = 1; degree <= 7; ++degree)
    {
        const int interval = mixolydian.intervals[static_cast<std::size_t>(degree - 1)];
        MaterialNote note;
        note.pitchClass = wrap(chord.rootPitchClass + interval, 12);
        note.semitonesFromRoot = interval;
        note.degree = degree;
        note.role = MaterialNoteRole::scaleTone;
        for (const auto& anchor : result.strategies.front().source.notes)
            if (anchor.pitchClass == note.pitchClass)
            {
                note.role = anchor.role;
                note.characteristic = anchor.characteristic;
                break;
            }
        note.spelling = spell(chord.rootFifths, degree, note.pitchClass);
        strategy.source.notes.push_back(std::move(note));
    }
    strategy.source.chordRelativeNotes = strategy.source.notes;
    strategy.idea = "Chord-local Mixolydian on the written dominant.";
    strategy.explanation = "Compatible chord-scale material; no tonic or local key is inferred.";
    strategy.conditions = "Optional material from explicit chord tones. Follow the actual next chord; a dominant chain is not a confirmed tonic resolution.";
    strategy.usageHint = "Chord-local source; use written guides and actual next-chord targets.";
    result.strategies.push_back(std::move(strategy));
    return true;
}
}

void addDiatonicSource(ImprovisationResult& result)
{
    if (!result.valid || result.strategies.empty())
        return;

    const auto& situation = result.context;
    bool added = false;
    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        if (situation.localKey.valid
            && situation.localKey.evidence.confidence == ConfidenceLevel::confirmed
            && situation.primaryInterpretationIndex >= 0
            && i != situation.primaryInterpretationIndex)
            continue;
        added = appendForInterpretation(result, static_cast<int>(i)) || added;
    }
    added = appendIncompleteCadenceMode(result) || added;
    if (! added)
        added = appendChordLocalDominant(result);

    if (added)
    {
        result.scaleUnavailableReason.clear();
        return;
    }

    if (situation.interpretationCount == 0)
        result.scaleUnavailableReason = "No harmonic interpretation: use the explicit chord tones.";
    else if (situation.currentChord.quality == ChordQuality::suspended2
             || situation.currentChord.quality == ChordQuality::suspended4
             || situation.currentChord.quality == ChordQuality::power
             || situation.currentChord.quality == ChordQuality::noThird
             || situation.currentChord.quality == ChordQuality::unknown)
        result.scaleUnavailableReason = "This chord needs a dedicated source rule; retain its explicit tones.";
    else if (result.dominantContext == DominantContext::toMinor)
        result.scaleUnavailableReason = "No basic diatonic source for this minor-target dominant; retain anchors and contextual alternatives.";
    else
        result.scaleUnavailableReason = "No compatible diatonic source for the available harmonic interpretations.";
}
}
