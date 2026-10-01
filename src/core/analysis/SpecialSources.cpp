#include "core/analysis/SpecialSources.h"
#include "core/analysis/MajorIiVPalette.h"
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
    enum class Form { melodicMinor, wholeHalf, halfWhole, wholeTone, harmonicMinorFragment } form = Form::melodicMinor;
};
const Rule minor {"boyko.melodic-minor.root", ImprovisationStrategyKind::melodicMinorApplication, 0,
    "Minor melodic color", "Levine, chapter 3, pp. 88-90; Boyko, section 2, pp. 88-89", "On m7 this is an optional overlay: keep the written b7 as an anchor and use the major 7 only as a passing color.",
    {1,0,9,3,0,11,0,5,0,13,0,7}, 10};
const Rule halfDim {"boyko.melodic-minor.bIII", ImprovisationStrategyKind::melodicMinorApplication, -3,
    "Locrian natural 2", "Levine, chapter 3, pp. 101-103; Boyko, section 2, pp. 92-93", "Natural 9 color; keep b3, b5 and b7 anchors.",
    {1,0,9,3,0,11,5,0,13,0,7,0}, -1};
const Rule lydian {"boyko.melodic-minor.V", ImprovisationStrategyKind::lydianDominantColor, 1,
    "Lydian dominant", "Levine, chapter 3, pp. 96-100; Boyko, section 2, pp. 96-97", "#11 color; resolve to the shown target.",
    {1,0,9,0,3,0,11,5,0,13,7,0}, -1};
const Rule flatSevenOverlay {"project.melodic-minor.bVII-overlay", ImprovisationStrategyKind::melodicMinorApplication, -2,
    "Melodic-minor overlay from bVII", "Project Fm6 / F melodic-minor application; this is not the altered scale",
    "Optional m6 overlay: keep the written dominant third as an anchor outside this source; the source's b3 is a color, not a replacement third.",
    {1,9,0,3,0,11,0,5,0,13,7,0}, 4};
const Rule minorTargetColor {"project.melodic-minor.minor-V-b13", ImprovisationStrategyKind::melodicMinorApplication, -1,
    "Rare fifth-mode melodic-minor color", "Levine, chapter 3, pp. 98-100: fifth mode is rare; conditional project application to written V7(b13)",
    "Rare conditional color on written b13: 11 and b13 can clash when sustained. It does not imply a minor next chord; follow the actual target.",
    {1,0,9,0,3,11,0,5,13,0,7,0}, -1};
const Rule harmonicMinorVFragment {"levine.harmonic-minor.minor-V-fragment", ImprovisationStrategyKind::harmonicMinorFragment, 0,
    "Harmonic-minor V fragment", "Levine, chapter 23, pp. 529-531: harmonic-minor fragments over V7(b9) resolving to minor",
    "Six-note V fragment for confirmed minor resolution and written b9 or b13; omit 11, and treat b13 as a passing color unless supported by the melody.",
    {1,9,0,0,3,0,0,5,13,0,7,0}, -1, Rule::Form::harmonicMinorFragment};
const Rule altered {"boyko.melodic-minor.bII", ImprovisationStrategyKind::alteredDominant, -5,
    "Altered dominant", "Levine, chapter 3, pp. 104-106; Boyko, section 2, pp. 101-102", "b9/#9/b5/b13; omit natural 5 in this line.",
    {1,9,0,9,3,0,5,0,13,0,7,0}, 7};
const Rule diminished {"boyko.diminished.whole-half", ImprovisationStrategyKind::diminishedApplication, 0,
    "Diminished whole-half", "Levine, chapter 3, pp. 112-124; Boyko, section 2, p. 112", "Whole-half on dim7; follow the actual next chord.",
    {1,0,9,3,0,11,5,0,13,7,0,7}, -1, Rule::Form::wholeHalf};
const Rule dominantDiminished {"levine.dominant.half-whole", ImprovisationStrategyKind::diminishedDominant, 0,
    "Dominant half-whole diminished", "Levine, chapter 3, pp. 112-124", "Optional b9/#9/#11 color on a compatible dominant; written natural 9 or b13 blocks this collection.",
    {1,9,0,9,3,0,11,5,0,13,7,0}, -1, Rule::Form::halfWhole};
const Rule wholeTone {"levine.dominant.whole-tone", ImprovisationStrategyKind::wholeToneDominant, 0,
    "Dominant whole-tone", "Levine, chapter 3, pp. 124-127", "Use for an explicit augmented dominant; the natural fifth is absent.",
    {1,0,9,0,3,0,11,0,5,0,7,0}, -1, Rule::Form::wholeTone};

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
            const int allowed = interval == 7 ? 5 : interval == 4 ? 3 : 7;
            if (given && simpleGiven != allowed) return false;
            continue;
        }
        if (!expected) return false;
        if (!given || simpleGiven == (expected - 1) % 7 + 1) continue;
        // b5/#11 and #5/b13 are intentional dominant enharmonic realizations.
        if (rule.form != Rule::Form::harmonicMinorFragment
            && chord.quality == ChordQuality::dominant
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

void append(ImprovisationResult& result, const Rule& rule, int index,
            bool subV = false, bool missingTonic = false, bool chordLocal = false,
            bool pendingIiV = false)
{
    const auto& chord = result.context.currentChord;
    if (!compatible(rule, chord)) return;
    if (! missingTonic && ! chordLocal && ! pendingIiV
        && (index < 0 || index >= result.context.interpretationCount
        || static_cast<std::size_t>(index) >= result.context.interpretations.size()
        || ! result.context.interpretations[static_cast<std::size_t>(index)].valid)) return;

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
    strategy.ruleVersion = &rule == &minor ? 4 : &rule == &dominantDiminished ? 2 : 3;
    strategy.priority = &rule == &minorTargetColor ? 15 : 40; // Never harmonic confidence.
    strategy.interpretationIndependent = chordLocal;
    strategy.interpretationIndex = missingTonic || chordLocal || pendingIiV ? -1 : index;
    strategy.missingTonicApplication = missingTonic;
    if (missingTonic)
    {
        strategy.missingTonicRootFifths = result.context.incompleteCadence.missingTonicRootFifths;
        strategy.evidence = {};
        strategy.evidence.confidence = ConfidenceLevel::low;
        strategy.evidence.add(EvidenceFlag::patternMatch);
    }
    else if (chordLocal) strategy.evidence = {};
    else if (pendingIiV) strategy.evidence = result.context.localPattern.evidence;
    else strategy.evidence = result.context.interpretations[static_cast<std::size_t>(index)].evidence;
    strategy.source = {};
    strategy.source.kind = MaterialKind::scale;
    strategy.source.rootFifths = chordSpellingFifths + rule.sourceFifthsOffset;
    strategy.source.rootPitchClass = circleOfFifthsToPitchClass(strategy.source.rootFifths);
    const auto rootName = spell(strategy.source.rootFifths, 1, strategy.source.rootPitchClass);
    switch (rule.form)
    {
        case Rule::Form::melodicMinor: strategy.source.name = rootName + " melodic minor"; break;
        case Rule::Form::wholeHalf: strategy.source.name = rootName + " whole-half diminished"; break;
        case Rule::Form::halfWhole: strategy.source.name = rootName + " half-whole diminished"; break;
        case Rule::Form::wholeTone: strategy.source.name = rootName + " whole-tone"; break;
        case Rule::Form::harmonicMinorFragment: strategy.source.name = rootName + " harmonic-minor V fragment"; break;
    }
    strategy.characteristicNotes.clear();
    strategy.sourceReference = rule.reference;
    if (subV) strategy.sourceReference += "; SubV context application: project rule, not a quoted author rule";
    strategy.usageHint = rule.hint;
    strategy.idea = rule.application;
    strategy.explanation = strategy.sourceReference;
    strategy.conditions = pendingIiV
        ? "Candidate major ii-V: the expected I has not been observed; no next-chord target is claimed. "
        : chordLocal
        ? "Chord-local optional color; no tonic, key or functional resolution is inferred. "
        : missingTonic
        ? "Incomplete ii-V: the expected major I did not sound; source root is not an established key. "
        : "Use with harmonic interpretation " + std::to_string(index + 1)
            + "; source root is not a song key. ";
    strategy.conditions += rule.hint;
    if (rule.omittedInterval >= 0 && chord.hasTone(rule.omittedInterval))
        strategy.omittedChordTones.push_back(wrap(chord.rootPitchClass + rule.omittedInterval));
    if (rule.omittedInterval == 10 && ! chord.hasTone(10))
        strategy.usageHint = "Native minor melodic sound on the written m6 or m(maj7).";
    constexpr int melodic[] = {0,2,3,5,7,9,11};
    constexpr int wholeHalf[] = {0,2,3,5,6,8,9,11};
    constexpr int wholeHalfDegrees[] = {1,2,3,4,5,6,7,7};
    constexpr int halfWhole[] = {0,1,3,4,6,7,9,10};
    constexpr int wholeToneIntervals[] = {0,2,4,6,8,10};
    constexpr int harmonicFragment[] = {0,1,4,7,8,10};
    const int count = rule.form == Rule::Form::wholeHalf || rule.form == Rule::Form::halfWhole ? 8
        : rule.form == Rule::Form::wholeTone || rule.form == Rule::Form::harmonicMinorFragment ? 6 : 7;
    for (int i = 0; i < count; ++i)
    {
        MaterialNote note;
        switch (rule.form)
        {
            case Rule::Form::melodicMinor:
                note.semitonesFromRoot = melodic[i]; note.degree = i + 1; break;
            case Rule::Form::wholeHalf:
                note.semitonesFromRoot = wholeHalf[i]; note.degree = wholeHalfDegrees[i]; break;
            case Rule::Form::halfWhole:
                note.semitonesFromRoot = halfWhole[i];
                note.degree = rule.degrees[static_cast<std::size_t>(halfWhole[i])]; break;
            case Rule::Form::wholeTone:
                note.semitonesFromRoot = wholeToneIntervals[i];
                note.degree = rule.degrees[static_cast<std::size_t>(wholeToneIntervals[i])]; break;
            case Rule::Form::harmonicMinorFragment:
                note.semitonesFromRoot = harmonicFragment[i];
                note.degree = rule.degrees[static_cast<std::size_t>(harmonicFragment[i])]; break;
        }
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
    if (rule.form == Rule::Form::melodicMinor)
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
    const auto& chord = situation.currentChord;

    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        const int index = static_cast<int>(i);
        if (situation.localKey.valid
            && situation.localKey.evidence.confidence == ConfidenceLevel::confirmed
            && situation.primaryInterpretationIndex >= 0
            && index != situation.primaryInterpretationIndex)
            continue;
        const auto& interpretation = situation.interpretations[i];
        if (!interpretation.valid || !interpretation.center.valid || !interpretation.center.key.valid
            || (interpretation.center.key.mode != KeyMode::major && interpretation.center.key.mode != KeyMode::minor))
            continue;

        if (chord.quality == ChordQuality::dominant)
        {
            const bool expectedMajor = situation.expectedTonic.valid
                && interpretation.kind == HarmonicInterpretationKind::globalContext
                && interpretation.center.key.mode == KeyMode::major
                && interpretation.center.key.rootPitchClass
                    == circleOfFifthsToPitchClass(situation.expectedTonic.rootFifths);
            const bool confirmedTarget = situation.resolution.available
                && situation.resolution.confirmed
                && situation.nextChordAvailable && situation.nextChord.valid
                && (result.dominantContext == DominantContext::toMajor
                    || result.dominantContext == DominantContext::toMinor);
            if (! confirmedTarget && ! expectedMajor)
                continue;

            if (hasIncompleteMajorIiVPalette(situation)
                && result.dominantContext == DominantContext::toMinor)
            {
                // Keep an explicitly altered V -> minor fragment where
                // applicable. The common altered/diminished choices are added
                // once below for the ii–V shape, without losing the factual
                // minor resolution or producing two identical list rows.
                if (chord.hasTone(1) && chord.degrees[1] == 9
                    || chord.hasTone(8) && chord.degrees[8] == 13)
                    append(result, harmonicMinorVFragment, index);
                continue;
            }

            const bool subV = interpretation.harmonic.substituteDominantConfirmed;
            if (subV)
            {
                append(result, lydian, index, true);
            }
            else if (!interpretation.harmonic.substituteDominantCandidate)
            {
                if (result.dominantContext == DominantContext::toMajor || expectedMajor)
                {
                    append(result, lydian, index);
                    append(result, flatSevenOverlay, index);
                }
                if (confirmedTarget && result.dominantContext == DominantContext::toMinor
                    && (chord.hasTone(1) && chord.degrees[1] == 9
                        || chord.hasTone(8) && chord.degrees[8] == 13))
                    append(result, harmonicMinorVFragment, index);
                append(result, altered, index);
                append(result, dominantDiminished, index);
                if (chord.hasAlteration(ChordAlteration::sharpFifth) && ! chord.hasTone(7))
                    append(result, wholeTone, index);
            }
        }
        else if (chord.quality == ChordQuality::minor)
        {
            if (situation.incompleteCadence.valid
                && situation.incompleteCadence.positionIndex == 0
                && chord.rootPitchClass == situation.incompleteCadence.ii.rootPitchClass)
            {
                // A melodic-minor overlay on the written ii is optional chord
                // color, not evidence for the global project's parallel mode.
                if (index == 0) append(result, minor, -1, false, false, true);
                continue;
            }
            // Native m6 / m(maj7), or a visibly conditional overlay on m7.
            // Avoid automatic Am6 color for a plain relative vi in major.
            const auto& global = situation.globalKey;
            const bool relativeMinor = global.valid && global.key.mode == KeyMode::major
                && interpretation.kind == HarmonicInterpretationKind::globalContext
                && interpretation.center.key.rootPitchClass == global.key.rootPitchClass
                && chord.rootPitchClass
                    == circleOfFifthsToPitchClass(global.key.rootFifths + 3);
            if (! relativeMinor || chord.hasTone(9) || chord.hasTone(11))
                append(result, minor, index);
        }
        else if (chord.quality == ChordQuality::halfDiminished)
            append(result, halfDim, index);
        else if (chord.quality == ChordQuality::diminished && chord.hasTone(9))
            append(result, diminished, index);
    }

    if (hasIncompleteMajorIiVPalette(situation))
    {
        // Same established catalog as V -> major, but with explicitly
        // provisional provenance and the real next chord retained as target.
        append(result, lydian, -1, false, true);
        append(result, flatSevenOverlay, -1, false, true);
        append(result, altered, -1, false, true);
        append(result, dominantDiminished, -1, false, true);
        if (chord.hasAlteration(ChordAlteration::sharpFifth) && ! chord.hasTone(7))
            append(result, wholeTone, -1, false, true);
    }

    if (!situation.nextChordAvailable && !situation.resolution.confirmed
        && situation.localPattern.type == HarmonicPatternType::majorIiVI
        && situation.localPattern.positionIndex == 1
        && chord.quality == ChordQuality::dominant && !result.substituteDominant)
    {
        // A future that has not been supplied is not a missing tonic. Retain
        // the ii-V evidence and major-directed palette without a target.
        append(result, lydian, -1, false, false, false, true);
        append(result, flatSevenOverlay, -1, false, false, false, true);
        append(result, altered, -1, false, false, false, true);
        append(result, dominantDiminished, -1, false, false, false, true);
    }

    // No tonic interpretation is needed to present a compatible chord-local
    // palette. This also covers V/V -> implied V and unknown future; the
    // written slash bass and target remain untouched.
    if (chord.quality == ChordQuality::dominant)
    {
        const auto hasRule = [&](const char* id)
        {
            return std::any_of(result.strategies.begin(), result.strategies.end(),
                [id](const auto& strategy) { return strategy.ruleId == id; });
        };
        if (! hasRule(dominantDiminished.id))
            append(result, dominantDiminished, -1, false, false, true);
        // An explicit b13 is a dominant alteration, not a reason to suppress
        // the entire palette. These sources are chord-local when a minor I is
        // not confirmed (e.g. Em7-A7b13-D7 in a dominant chain).
        if (chord.hasTone(8) && chord.degrees[8] == 13)
        {
            if (! hasRule(minorTargetColor.id))
                append(result, minorTargetColor, -1, false, false, true);
            if (! hasRule(altered.id))
                append(result, altered, -1, false, false, true);
        }
        if (chord.hasAlteration(ChordAlteration::sharpFifth) && ! chord.hasTone(7)
            && ! hasRule(wholeTone.id))
            append(result, wholeTone, -1, false, false, true);
    }
}
}
