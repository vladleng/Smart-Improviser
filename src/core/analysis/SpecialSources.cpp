#include "core/analysis/SpecialSources.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <utility>

namespace smartimproviser::harmony
{
namespace
{
int wrap(int n, int modulus = 12) { return (n % modulus + modulus) % modulus; }
std::string spell(int fifths, int degree, int pitch)
{
    constexpr const char* letters[] = {"C","D","E","F","G","A","B"};
    constexpr int natural[] = {0,2,4,5,7,9,11};
    const int letter = wrap(4 * wrap(fifths, 7) + degree - 1, 7);
    int accidental = wrap(pitch - natural[letter]);
    if (accidental > 6) accidental -= 12;
    return std::string(letters[letter]) + std::string(static_cast<std::size_t>(std::abs(accidental)), accidental < 0 ? 'b' : '#');
}
struct Rule
{
    const char* id;
    ImprovisationStrategyKind kind;
    int sourceFifthsOffset;
    const char* application;
    const char* reference;
    const char* hint;
    // Chord-relative pitch intervals; zero means a non-member, otherwise a degree.
    std::array<int, 12> degrees;
    int omittedInterval;
    bool diminished = false;
};
const Rule minor {"boyko.melodic-minor.root", ImprovisationStrategyKind::melodicMinorApplication, 0,
    "Minor melodic color", "Boyko, section 2, pp. 88-89, 121", "Major 7th on m7: passing to root, not an anchor.",
    {1,0,9,3,0,11,0,5,0,13,0,7}, 10};
const Rule halfDim {"boyko.melodic-minor.bIII", ImprovisationStrategyKind::melodicMinorApplication, -3,
    "Locrian natural 2", "Boyko, section 2, pp. 92-93, 121", "Natural 9 color; keep b3, b5 and b7 anchors.",
    {1,0,9,3,0,11,5,0,13,0,7,0}, -1};
const Rule lydian {"boyko.melodic-minor.V", ImprovisationStrategyKind::lydianDominantColor, 1,
    "Lydian dominant", "Boyko, section 2, pp. 96-97, 121", "#11 color; resolve to the shown target.",
    {1,0,9,0,3,0,11,5,0,13,7,0}, -1};
const Rule altered {"boyko.melodic-minor.bII", ImprovisationStrategyKind::alteredDominant, -5,
    "Altered dominant", "Boyko, section 2, pp. 101-102, 121", "b9/#9/b5/b13; omit natural 5 in this line.",
    {1,9,0,9,3,0,5,0,13,0,7,0}, 7};
const Rule diminished {"boyko.diminished.whole-half", ImprovisationStrategyKind::diminishedApplication, 0,
    "Diminished whole-half", "Boyko, section 2, p. 112, example 176", "Whole-half on dim7; follow the actual next chord.",
    {1,0,9,3,0,11,5,0,13,7,0,7}, -1, true};

bool compatible(const Rule& rule, const NormalizedChord& chord)
{
    for (int interval = 0; interval < 12; ++interval)
    {
        if (!chord.hasTone(interval)) continue;
        const int given = chord.degrees[static_cast<std::size_t>(interval)];
        const int expected = rule.degrees[static_cast<std::size_t>(interval)];
        const int simpleGiven = given ? (given - 1) % 7 + 1 : 0;
        if (interval == rule.omittedInterval)
        {
            const int allowed = interval == 7 ? 5 : 7;
            if (given && simpleGiven != allowed) return false;
            continue;
        }
        if (!expected) return false;
        if (!given || simpleGiven == (expected - 1) % 7 + 1) continue;
        // b5/#11 and #5/b13 are intentional dominant enharmonic realizations.
        if (chord.quality == ChordQuality::dominant
            && ((interval == 6 && (simpleGiven == 4 || simpleGiven == 5))
                || (interval == 8 && (simpleGiven == 5 || simpleGiven == 6)))) continue;
        return false;
    }
    // A slash bass is sounding context too. Do not omit its pitch silently.
    if (chord.slashBass && !rule.degrees[static_cast<std::size_t>(wrap(chord.bassPitchClass - chord.rootPitchClass))])
        return false;
    return true;
}

int functionalSubVRootFifths(const ImprovisationResult& result, const NormalizedChord& chord)
{
    if (! result.context.resolution.available || ! result.context.resolution.confirmed
        || ! result.context.resolution.targetChord.valid)
        return chord.rootFifths;

    const auto candidate = result.context.resolution.targetChord.rootFifths - 5;
    return circleOfFifthsToPitchClass(candidate) == chord.rootPitchClass
        ? candidate : chord.rootFifths;
}

void append(ImprovisationResult& result, const Rule& rule, bool subV = false)
{
    const auto& chord = result.context.currentChord;
    if (!compatible(rule, chord)) return;
    const int index = result.context.primaryInterpretationIndex;
    const auto& interpretation = result.context.interpretations[static_cast<std::size_t>(index)];
    auto strategy = result.strategies.front();
    const auto chordSpellingFifths = subV ? functionalSubVRootFifths(result, chord) : chord.rootFifths;
    if (subV)
    {
        const bool bassIsRoot = chord.bassPitchClass == chord.rootPitchClass;
        strategy.actualChord.rootFifths = chordSpellingFifths;
        if (bassIsRoot) strategy.actualChord.bassFifths = chordSpellingFifths;
        strategy.actualChord.slashBass = strategy.actualChord.bassPitchClass != strategy.actualChord.rootPitchClass;
    }
    strategy.kind = rule.kind;
    strategy.ruleId = subV ? "project.subv.melodic-minor.V" : rule.id;
    strategy.ruleVersion = subV ? 2 : 1;
    strategy.priority = 40; // Stable catalog order, not a tension or confidence score.
    strategy.interpretationIndependent = false;
    strategy.interpretationIndex = index;
    strategy.evidence = interpretation.evidence;
    strategy.source = {};
    strategy.source.kind = MaterialKind::scale;
    strategy.source.rootFifths = chordSpellingFifths + rule.sourceFifthsOffset;
    strategy.source.rootPitchClass = circleOfFifthsToPitchClass(strategy.source.rootFifths);
    strategy.source.name = spell(strategy.source.rootFifths, 1, strategy.source.rootPitchClass)
        + (rule.diminished ? " whole-half diminished" : " melodic minor");
    strategy.characteristicNotes.clear();
    strategy.sourceReference = rule.reference;
    if (subV) strategy.sourceReference += "; SubV context application: project rule, not a quoted author rule";
    strategy.usageHint = rule.hint;
    strategy.idea = rule.application;
    strategy.explanation = strategy.sourceReference;
    strategy.conditions = "Use with the selected interpretation; source root is not a song key. ";
    strategy.conditions += rule.hint;
    if (rule.omittedInterval >= 0 && chord.hasTone(rule.omittedInterval))
        strategy.omittedChordTones.push_back(wrap(chord.rootPitchClass + rule.omittedInterval));
    if (rule.omittedInterval == 10 && !chord.hasTone(10))
        strategy.usageHint = "Melodic minor color; keep the actual chord anchors.";
    constexpr int melodic[] = {0,2,3,5,7,9,11};
    constexpr int wholeHalf[] = {0,2,3,5,6,8,9,11};
    constexpr int wholeHalfDegrees[] = {1,2,3,4,5,6,7,7};
    const int count = rule.diminished ? 8 : 7;
    for (int i = 0; i < count; ++i)
    {
        MaterialNote note;
        note.semitonesFromRoot = rule.diminished ? wholeHalf[i] : melodic[i];
        note.degree = rule.diminished ? wholeHalfDegrees[i] : i + 1;
        note.pitchClass = wrap(strategy.source.rootPitchClass + note.semitonesFromRoot);
        note.spelling = spell(strategy.source.rootFifths, note.degree, note.pitchClass);
        const int relative = wrap(note.pitchClass - chord.rootPitchClass);
        note.role = chord.hasTone(relative) ? MaterialNoteRole::chordTone : MaterialNoteRole::colorTone;
        note.characteristic = !chord.hasTone(relative);
        for (const auto& guide : strategy.guideNotes)
            if (guide.pitchClass == note.pitchClass) note.role = MaterialNoteRole::guideTone;
        if (rule.omittedInterval == 10 && chord.hasTone(10) && relative == 11)
            note.role = MaterialNoteRole::passingTone;
        strategy.source.notes.push_back(note);
        auto chordNote = note;
        chordNote.semitonesFromRoot = relative;
        chordNote.degree = rule.degrees[static_cast<std::size_t>(relative)];
        if (chord.hasTone(relative) && chord.degrees[static_cast<std::size_t>(relative)])
            chordNote.degree = chord.degrees[static_cast<std::size_t>(relative)];
        chordNote.spelling = spell(chordSpellingFifths, chordNote.degree, note.pitchClass);
        strategy.source.chordRelativeNotes.push_back(chordNote);
        if (note.characteristic) strategy.characteristicNotes.push_back(chordNote);
    }
    if (!rule.diminished)
    {
        ChordContext thinking;
        thinking.available = thinking.defined = true;
        thinking.root = thinking.bass = strategy.source.rootFifths;
        thinking.intervals.values[0] = 1;
        thinking.intervals.values[3] = 3;
        thinking.intervals.values[7] = 5;
        thinking.intervals.values[9] = 6;
        strategy.thinkingStructure = normalizeChord(thinking);
    }
    // Color movements are suggestions, separate from confirmed Stage 2 tendency tones.
    for (const auto& color : strategy.characteristicNotes)
    {
        const MaterialNote* best = nullptr;
        int bestDelta = 13;
        for (const auto& target : strategy.targetNotes)
        {
            if (target.semitonesFromRoot != 0 && target.role != MaterialNoteRole::guideTone) continue;
            int delta = wrap(target.pitchClass - color.pitchClass);
            if (delta > 6) delta -= 12;
            if (std::abs(delta) < std::abs(bestDelta)) { best = &target; bestDelta = delta; }
        }
        if (best && std::abs(bestDelta) <= 2)
            strategy.sourceTransitions.push_back({color.pitchClass, best->pitchClass, bestDelta, ResolutionImportance::optional});
    }
    if (result.dominantContext == DominantContext::toMajor)
        strategy.conditions += " Resolve to the actual major target (including its major third).";
    else if (result.dominantContext == DominantContext::toMinor)
        strategy.conditions += " Resolve to the actual minor target (including its minor third).";
    result.strategies.push_back(std::move(strategy));
    result.scaleUnavailableReason.clear();
}
}
void addSpecialSources(ImprovisationResult& result)
{
    if (!result.valid || result.strategies.empty()) return;
    const auto& situation = result.context;
    const int index = situation.primaryInterpretationIndex;
    if (index < 0 || index >= situation.interpretationCount
        || static_cast<std::size_t>(index) >= situation.interpretations.size()) return;
    const auto& interpretation = situation.interpretations[static_cast<std::size_t>(index)];
    if (!interpretation.valid || !interpretation.center.valid || !interpretation.center.key.valid
        || (interpretation.center.key.mode != KeyMode::major && interpretation.center.key.mode != KeyMode::minor)) return;
    const auto& chord = situation.currentChord;
    if (chord.quality == ChordQuality::dominant)
    {
        if (!situation.resolution.available || !situation.resolution.confirmed
            || !situation.nextChordAvailable || !situation.nextChord.valid
            || (result.dominantContext != DominantContext::toMajor && result.dominantContext != DominantContext::toMinor)) return;
        if (result.substituteDominant)
            append(result, lydian, true);
        else if (!interpretation.harmonic.substituteDominantCandidate && !situation.harmonic.substituteDominantConfirmed)
        {
            if (result.dominantContext == DominantContext::toMajor) append(result, lydian);
            append(result, altered);
        }
    }
    else if (chord.quality == ChordQuality::minor) append(result, minor);
    else if (chord.quality == ChordQuality::halfDiminished) append(result, halfDim);
    else if (chord.quality == ChordQuality::diminished && chord.hasTone(9)) append(result, diminished);
}
}
