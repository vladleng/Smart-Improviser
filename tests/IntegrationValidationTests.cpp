#include "core/analysis/HarmonicEngine.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <vector>

using namespace smartimproviser::harmony;

namespace
{
void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

KeyContext makeKey(std::int32_t rootFifths, bool minor)
{
    KeyContext key;
    key.available = true;
    key.defined = true;
    key.root = rootFifths;

    const int majorIntervals[] = { 0, 2, 4, 5, 7, 9, 11 };
    const int minorIntervals[] = { 0, 2, 3, 5, 7, 8, 10 };
    const auto* values = minor ? minorIntervals : majorIntervals;
    for (int i = 0; i < 7; ++i)
        key.intervals.values[static_cast<std::size_t>(values[i])] = 0xFFu;
    return key;
}

ChordContext makeChord(std::int32_t rootFifths,
                       std::initializer_list<int> relativeTones)
{
    ChordContext chord;
    chord.available = true;
    chord.defined = true;
    chord.root = rootFifths;
    chord.bass = rootFifths;
    for (const auto semitone : relativeTones)
        chord.intervals.values[static_cast<std::size_t>(semitone)] = 0xFFu;
    return chord;
}

TimelineHarmonicSnapshot windowAt(const KeyContext& key,
                                  const std::vector<ChordContext>& chords,
                                  std::size_t index)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = static_cast<double>(index * 4);
    snapshot.globalKey = key;
    snapshot.currentChord = chords[index];

    if (index > 0)
    {
        snapshot.previousChordAvailable = true;
        snapshot.previousChord = chords[index - 1];
    }

    if (index + 1 < chords.size())
    {
        snapshot.nextChordAvailable = true;
        snapshot.nextChord = chords[index + 1];
    }

    return snapshot;
}

bool primaryIs(const HarmonicSituation& situation,
               HarmonicInterpretationKind kind) noexcept
{
    return situation.primaryInterpretationIndex >= 0
        && situation.primaryInterpretationIndex < situation.interpretationCount
        && situation.interpretations[static_cast<std::size_t>(situation.primaryInterpretationIndex)].kind == kind;
}
}

int main()
{
    const auto cMajor = makeKey(0, false);
    const auto fMajor = makeKey(-1, false);

    const auto cMaj7 = makeChord(0, { 0, 4, 7, 11 });
    const auto a7 = makeChord(3, { 0, 4, 7, 10 });
    const auto dMin7 = makeChord(2, { 0, 3, 7, 10 });
    const auto g7 = makeChord(1, { 0, 4, 7, 10 });
    const auto fMin7 = makeChord(-1, { 0, 3, 7, 10 });
    const auto abMaj7 = makeChord(-4, { 0, 4, 7, 11 });

    // ---------------------------------------------------------------------
    // Integrated turnaround -> temporary local center -> return to global.
    // Cmaj7 - A7 - Dm7 - G7 - Cmaj7
    const std::vector<ChordContext> turnaround = { cMaj7, a7, dMin7, g7, cMaj7 };

    const auto turnI = analyzeHarmonicSituation(windowAt(cMajor, turnaround, 0));
    expect(turnI.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnI.pattern.role == PatternMemberRole::tonic,
           "turnaround starts as I-VI-ii-V tonic candidate");
    expect(turnI.evidence.interpretation == InterpretationStatus::unique
           && primaryIs(turnI, HarmonicInterpretationKind::globalContext),
           "turnaround start is unique global context");

    const auto turnVI = analyzeHarmonicSituation(windowAt(cMajor, turnaround, 1));
    expect(turnVI.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnVI.pattern.role == PatternMemberRole::preparation,
           "A7 keeps global turnaround role");
    expect(turnVI.localKey.valid
           && turnVI.localKey.key.rootPitchClass == 2
           && turnVI.localKey.key.mode == KeyMode::minor
           && turnVI.localKey.status == KeyCenterStatus::tonicized,
           "A7-Dm creates temporary D minor tonicization");
    expect(turnVI.evidence.interpretation == InterpretationStatus::unique
           && primaryIs(turnVI, HarmonicInterpretationKind::localCenter),
           "confirmed A7-Dm tonicization becomes local primary");

    const auto turnIi = analyzeHarmonicSituation(windowAt(cMajor, turnaround, 2));
    expect(turnIi.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnIi.pattern.role == PatternMemberRole::predominant,
           "Dm returns to global turnaround predominant role");
    expect(! turnIi.localKey.valid
           && turnIi.evidence.interpretation == InterpretationStatus::unique
           && primaryIs(turnIi, HarmonicInterpretationKind::globalContext),
           "redundant C local center is rejected and global context resumes");

    const auto turnV = analyzeHarmonicSituation(windowAt(cMajor, turnaround, 3));
    expect(turnV.pattern.type == HarmonicPatternType::majorIiVI
           && turnV.pattern.role == PatternMemberRole::dominant
           && turnV.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "Dm-G7-C closes as confirmed major ii-V-I");
    expect(! turnV.localKey.valid && primaryIs(turnV, HarmonicInterpretationKind::globalContext),
           "global cadence does not create redundant local center");

    const auto turnResolution = analyzeHarmonicSituation(windowAt(cMajor, turnaround, 4));
    expect(turnResolution.pattern.type == HarmonicPatternType::majorIiVI
           && turnResolution.pattern.role == PatternMemberRole::resolution,
           "final Cmaj7 is recognized as ii-V-I resolution boundary");

    // ---------------------------------------------------------------------
    // Borrowed/modal ambiguity must resolve again when the harmony becomes
    // structurally decisive in the explicit global key.
    const std::vector<ChordContext> borrowed = { cMaj7, fMin7, g7, cMaj7 };
    const auto borrowedIv = analyzeHarmonicSituation(windowAt(cMajor, borrowed, 1));
    expect(borrowedIv.evidence.interpretation == InterpretationStatus::ambiguous
           && borrowedIv.primaryInterpretationIndex == -1
           && borrowedIv.interpretationCount == 2,
           "borrowed iv exposes unresolved global/modal ambiguity");

    const auto borrowedV = analyzeHarmonicSituation(windowAt(cMajor, borrowed, 2));
    expect(borrowedV.pattern.type == HarmonicPatternType::dominantToTonic
           && borrowedV.evidence.interpretation == InterpretationStatus::unique
           && primaryIs(borrowedV, HarmonicInterpretationKind::globalContext),
           "G7-C resolves borrowed ambiguity back to global context");

    // ---------------------------------------------------------------------
    // Local iiø-SubV-i inside a different global key, followed by a return to
    // the global ii-V-I pathway.
    const auto eHalfDim7 = makeChord(4, { 0, 3, 6, 10 });
    const auto eb7 = makeChord(-3, { 0, 4, 7, 10 });
    const auto dMinor = makeChord(2, { 0, 3, 7 });
    const std::vector<ChordContext> localSubV = { eHalfDim7, eb7, dMinor, g7, cMaj7 };

    const auto subVDominant = analyzeHarmonicSituation(windowAt(cMajor, localSubV, 1));
    expect(subVDominant.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "Eb7-Dm is recognized as applied tritone substitution globally");
    expect(subVDominant.localKey.valid
           && subVDominant.localKey.key.rootPitchClass == 2
           && subVDominant.localKey.key.mode == KeyMode::minor
           && subVDominant.localKey.status == KeyCenterStatus::established,
           "Em7b5-Eb7-Dm establishes local D minor");
    expect(subVDominant.localPattern.type == HarmonicPatternType::tritoneSubstitution
           && subVDominant.localPattern.role == PatternMemberRole::substituteDominant,
           "local pattern identifies ii-half-diminished-SubV-i dominant position");
    expect(primaryIs(subVDominant, HarmonicInterpretationKind::localCenter),
           "established local SubV cadence becomes primary interpretation");

    const auto localResolution = analyzeHarmonicSituation(windowAt(cMajor, localSubV, 2));
    expect(localResolution.localKey.valid
           && localResolution.localKey.key.rootPitchClass == 2
           && localResolution.localHarmonic.rootFunction == HarmonicFunction::tonic,
           "resolved Dm retains confirmed local tonic reading");
    expect(primaryIs(localResolution, HarmonicInterpretationKind::localCenter),
           "confirmed previous SubV resolution remains local primary on Dm");

    const auto returnedGlobal = analyzeHarmonicSituation(windowAt(cMajor, localSubV, 3));
    expect(returnedGlobal.pattern.type == HarmonicPatternType::majorIiVI
           && returnedGlobal.pattern.role == PatternMemberRole::dominant
           && ! returnedGlobal.localKey.valid
           && primaryIs(returnedGlobal, HarmonicInterpretationKind::globalContext),
           "Dm-G7-C cleanly returns from local D minor to global C major");

    // ---------------------------------------------------------------------
    // Remote V-I-X: confirmed tonicization first, then cautious modulation
    // candidate without mutating the explicit project key.
    const auto cs7 = makeChord(7, { 0, 4, 7, 10 });
    const auto fsMaj7 = makeChord(6, { 0, 4, 7, 11 });
    const auto bMaj7 = makeChord(5, { 0, 4, 7, 11 });
    const std::vector<ChordContext> remote = { cs7, fsMaj7, bMaj7 };

    const auto remoteDominant = analyzeHarmonicSituation(windowAt(cMajor, remote, 0));
    expect(remoteDominant.localKey.valid
           && remoteDominant.localKey.key.rootPitchClass == 6
           && remoteDominant.localKey.status == KeyCenterStatus::tonicized
           && primaryIs(remoteDominant, HarmonicInterpretationKind::localCenter),
           "C#7-F#maj7 is a confirmed temporary F# tonicization");

    const auto remoteTonic = analyzeHarmonicSituation(windowAt(cMajor, remote, 1));
    expect(remoteTonic.globalKey.key.rootPitchClass == 0,
           "modulation evidence never mutates explicit C global key");
    expect(remoteTonic.localKey.valid
           && remoteTonic.localKey.key.rootPitchClass == 6
           && remoteTonic.localKey.status == KeyCenterStatus::modulationCandidate,
           "F#maj7-Bmaj7 promotes F# to modulation candidate");
    expect(remoteTonic.evidence.interpretation == InterpretationStatus::ambiguous
           && remoteTonic.primaryInterpretationIndex == -1,
           "modulation candidate remains unresolved ambiguity");

    // ---------------------------------------------------------------------
    // Contradictory known next chord must suppress a boundary guess. Dm-G7
    // could be the end of I-VI-ii-V only when the next event is unavailable;
    // with Abmaj7 already known, the engine must not invent that turnaround.
    const std::vector<ChordContext> contradicted = { dMin7, g7, abMaj7 };
    const auto contradictedV = analyzeHarmonicSituation(windowAt(cMajor, contradicted, 1));
    expect(contradictedV.pattern.type != HarmonicPatternType::turnaroundIVIiiV,
           "known contradictory next chord suppresses turnaround-end false positive");
    expect(! contradictedV.harmonic.dominantResolutionConfirmed,
           "G7-Abmaj7 is not falsely marked as dominant resolution");

    // ---------------------------------------------------------------------
    // Stage 1 contract safety remains intact at integration level.
    TimelineHarmonicSnapshot noPosition;
    noPosition.globalKey = fMajor;
    noPosition.currentChord = cMaj7;
    expect(! analyzeHarmonicSituation(noPosition).valid,
           "missing timeline position remains a safe non-analysis state");

    TimelineHarmonicSnapshot noKey;
    noKey.positionAvailable = true;
    noKey.currentChord = cMaj7;
    expect(! analyzeHarmonicSituation(noKey).valid,
           "missing global key remains a safe non-analysis state");

    std::cout << "SmartImproviser IntegrationValidationTests: OK\n";
    return 0;
}
