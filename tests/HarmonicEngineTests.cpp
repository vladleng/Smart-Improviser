#include "core/analysis/HarmonicEngine.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>

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

TimelineHarmonicSnapshot makeSnapshot(const KeyContext& key,
                                      const ChordContext& previous,
                                      const ChordContext& current,
                                      const ChordContext& next)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = key;
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = previous;
    snapshot.currentChord = current;
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}

TimelineHarmonicSnapshot makeCurrentNextSnapshot(const KeyContext& key,
                                                 const ChordContext& current,
                                                 const ChordContext& next)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = key;
    snapshot.currentChord = current;
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}

TimelineHarmonicSnapshot makePreviousCurrentSnapshot(const KeyContext& key,
                                                     const ChordContext& previous,
                                                     const ChordContext& current)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = key;
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = previous;
    snapshot.currentChord = current;
    return snapshot;
}
}

int main()
{
    const auto cMajor = makeKey(0, false);
    const auto cMaj7 = makeChord(0, { 0, 4, 7, 11 });
    const auto aMin7 = makeChord(3, { 0, 3, 7, 10 });
    const auto a7 = makeChord(3, { 0, 4, 7, 10 });
    const auto dMin7 = makeChord(2, { 0, 3, 7, 10 });
    const auto d7 = makeChord(2, { 0, 4, 7, 10 });
    const auto g7 = makeChord(1, { 0, 4, 7, 10 });
    const auto db7 = makeChord(-5, { 0, 4, 7, 10 });

    // Major ii-V-I: full V position remains confirmed.
    const auto majorIiVI = makeSnapshot(cMajor, dMin7, g7, cMaj7);
    const auto majorSituation = analyzeHarmonicSituation(majorIiVI);
    expect(majorSituation.valid, "major ii-V-I situation is valid");
    expect(majorSituation.pattern.type == HarmonicPatternType::majorIiVI,
           "major ii-V-I is recognized");
    expect(majorSituation.pattern.role == PatternMemberRole::dominant,
           "current G7 is dominant member of major ii-V-I");
    expect(majorSituation.pattern.positionIndex == 1 && majorSituation.pattern.length == 3,
           "major ii-V-I V position is preserved");
    expect(majorSituation.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "major ii-V-I V position is confirmed by real resolution");
    expect(majorSituation.pattern.evidence.has(EvidenceFlag::previousChord)
           && majorSituation.pattern.evidence.has(EvidenceFlag::nextChord)
           && majorSituation.pattern.evidence.has(EvidenceFlag::confirmedResolution),
           "full major ii-V-I records complete pattern evidence");

    // Major ii-V-I boundary positions.
    const auto majorIiPosition = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, dMin7, g7));
    expect(majorIiPosition.pattern.type == HarmonicPatternType::majorIiVI,
           "major ii-V-I is recognized from ii position");
    expect(majorIiPosition.pattern.role == PatternMemberRole::predominant,
           "ii position is predominant");
    expect(majorIiPosition.pattern.positionIndex == 0,
           "ii position index is 1/3");
    expect(majorIiPosition.pattern.evidence.confidence == ConfidenceLevel::high,
           "ii boundary candidate is high, not confirmed");
    expect(majorIiPosition.pattern.evidence.has(EvidenceFlag::nextChord),
           "ii boundary candidate records next-chord evidence");

    const auto majorIPosition = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(cMajor, g7, cMaj7));
    expect(majorIPosition.pattern.type == HarmonicPatternType::majorIiVI,
           "major ii-V-I is recognized from I resolution position");
    expect(majorIPosition.pattern.role == PatternMemberRole::resolution,
           "I position is resolution");
    expect(majorIPosition.pattern.positionIndex == 2,
           "I position index is 3/3");
    expect(majorIPosition.pattern.evidence.confidence == ConfidenceLevel::high,
           "I boundary candidate is high, not confirmed");
    expect(majorIPosition.pattern.evidence.has(EvidenceFlag::previousChord),
           "I boundary candidate records previous-chord evidence");

    // Minor iiø-V-i: all three positions.
    const auto aMinor = makeKey(3, true);
    const auto bHalfDim7 = makeChord(5, { 0, 3, 6, 10 });
    const auto e7 = makeChord(4, { 0, 4, 7, 10 });
    const auto aMinorChord = makeChord(3, { 0, 3, 7 });
    const auto bb7 = makeChord(-2, { 0, 4, 7, 10 });

    const auto minorVPosition = analyzeHarmonicSituation(
        makeSnapshot(aMinor, bHalfDim7, e7, aMinorChord));
    expect(minorVPosition.pattern.type == HarmonicPatternType::minorIiHalfDimVi,
           "minor ii-half-diminished-V-i is recognized at V");
    expect(minorVPosition.pattern.role == PatternMemberRole::dominant,
           "minor V position is dominant");
    expect(minorVPosition.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "minor V position is confirmed");

    const auto minorIiPosition = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(aMinor, bHalfDim7, e7));
    expect(minorIiPosition.pattern.type == HarmonicPatternType::minorIiHalfDimVi,
           "minor ii-V-i is recognized from ii-half-diminished position");
    expect(minorIiPosition.pattern.role == PatternMemberRole::predominant
           && minorIiPosition.pattern.positionIndex == 0,
           "minor ii position is 1/3 predominant");
    expect(minorIiPosition.pattern.evidence.confidence == ConfidenceLevel::high,
           "minor ii boundary candidate is high");

    const auto minorIPosition = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(aMinor, e7, aMinorChord));
    expect(minorIPosition.pattern.type == HarmonicPatternType::minorIiHalfDimVi,
           "minor ii-V-i is recognized from tonic resolution position");
    expect(minorIPosition.pattern.role == PatternMemberRole::resolution
           && minorIPosition.pattern.positionIndex == 2,
           "minor i position is 3/3 resolution");

    // I-VI-ii-V turnaround.
    const auto turnaroundVI = analyzeHarmonicSituation(
        makeSnapshot(cMajor, cMaj7, a7, dMin7));
    expect(turnaroundVI.pattern.type == HarmonicPatternType::turnaroundIVIiiV,
           "I-VI-ii-V is recognized at VI");
    expect(turnaroundVI.pattern.role == PatternMemberRole::preparation
           && turnaroundVI.pattern.positionIndex == 1
           && turnaroundVI.pattern.length == 4,
           "turnaround VI position is 2/4 preparation");
    expect(turnaroundVI.pattern.evidence.confidence == ConfidenceLevel::high,
           "three-member turnaround window is high confidence");

    const auto turnaroundIi = analyzeHarmonicSituation(
        makeSnapshot(cMajor, a7, dMin7, g7));
    expect(turnaroundIi.pattern.type == HarmonicPatternType::turnaroundIVIiiV,
           "I-VI-ii-V is recognized at ii");
    expect(turnaroundIi.pattern.role == PatternMemberRole::predominant
           && turnaroundIi.pattern.positionIndex == 2,
           "turnaround ii position is 3/4 predominant");

    const auto turnaroundI = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, cMaj7, aMin7));
    expect(turnaroundI.pattern.type == HarmonicPatternType::turnaroundIVIiiV,
           "turnaround start is recognized from I-VI pair");
    expect(turnaroundI.pattern.role == PatternMemberRole::tonic
           && turnaroundI.pattern.positionIndex == 0,
           "turnaround start position is 1/4 tonic");
    expect(turnaroundI.pattern.evidence.confidence == ConfidenceLevel::medium,
           "two-member turnaround boundary is medium confidence");

    const auto turnaroundV = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(cMajor, dMin7, g7));
    expect(turnaroundV.pattern.type == HarmonicPatternType::turnaroundIVIiiV,
           "turnaround end is recognized from ii-V pair without tonic target");
    expect(turnaroundV.pattern.role == PatternMemberRole::dominant
           && turnaroundV.pattern.positionIndex == 3,
           "turnaround end position is 4/4 dominant");
    expect(turnaroundV.pattern.evidence.confidence == ConfidenceLevel::medium,
           "turnaround end boundary is medium confidence");

    // A7-D7-G7 dominant chain.
    const auto dominantChain = analyzeHarmonicSituation(
        makeSnapshot(cMajor, a7, d7, g7));
    expect(dominantChain.pattern.type == HarmonicPatternType::dominantChain,
           "three linked dominants are recognized as a dominant chain");
    expect(dominantChain.pattern.role == PatternMemberRole::dominant
           && dominantChain.pattern.positionIndex == 1
           && dominantChain.pattern.length == 3,
           "dominant chain current member is 2/3 dominant");
    expect(dominantChain.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "dominant chain is confirmed by real adjacent dominant relations");

    // 0.2c: major ii-SubV-I full cadence.
    const auto majorTritone = analyzeHarmonicSituation(
        makeSnapshot(cMajor, dMin7, db7, cMaj7));
    expect(majorTritone.harmonic.substituteDominantConfirmed,
           "Db7->C confirms tritone-substitute dominant");
    expect(majorTritone.harmonic.effectiveFunction == HarmonicFunction::substituteDominant,
           "Db7 effective function is substitute dominant");
    expect(majorTritone.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "Dm7-Db7-Cmaj7 is recognized as tritone substitution pattern");
    expect(majorTritone.pattern.role == PatternMemberRole::substituteDominant
           && majorTritone.pattern.positionIndex == 1
           && majorTritone.pattern.length == 3,
           "Db7 is substitute-dominant position 2/3");
    expect(majorTritone.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "full ii-SubV-I is confirmed");
    expect(majorTritone.resolution.available && majorTritone.resolution.confirmed,
           "SubV-I creates confirmed resolution target");
    expect(majorTritone.resolution.targetChord.rootPitchClass == 0,
           "Db7 resolution target is C");
    expect(majorTritone.resolution.moveCount == 2,
           "SubV resolution exposes two structural guide-tone moves");
    expect(majorTritone.resolution.moves[0].semitoneDelta == -1
           && majorTritone.resolution.moves[1].semitoneDelta == 1,
           "Db7 guide tones resolve by semitone to Cmaj7");
    expect(! majorTritone.localKey.valid,
           "0.2c does not invent local-key inference for SubV");

    // Major ii-SubV-I boundary positions are high rather than confirmed.
    const auto majorSubIi = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, dMin7, db7));
    expect(majorSubIi.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "major ii-SubV is recognized at ii boundary");
    expect(majorSubIi.pattern.role == PatternMemberRole::predominant
           && majorSubIi.pattern.positionIndex == 0,
           "ii-SubV boundary current ii is 1/3 predominant");
    expect(majorSubIi.pattern.evidence.confidence == ConfidenceLevel::high,
           "ii-SubV boundary remains high confidence");

    const auto majorSubI = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(cMajor, db7, cMaj7));
    expect(majorSubI.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "SubV-I is recognized at tonic resolution boundary");
    expect(majorSubI.pattern.role == PatternMemberRole::resolution
           && majorSubI.pattern.positionIndex == 2,
           "SubV-I tonic is 3/3 resolution");
    expect(majorSubI.pattern.evidence.confidence == ConfidenceLevel::high,
           "SubV-I resolution boundary remains high confidence");

    // Minor iiø-SubV-i full cadence.
    const auto minorTritone = analyzeHarmonicSituation(
        makeSnapshot(aMinor, bHalfDim7, bb7, aMinorChord));
    expect(minorTritone.harmonic.substituteDominantConfirmed,
           "Bb7->Am confirms minor-key tritone substitute");
    expect(minorTritone.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "Bm7b5-Bb7-Am is recognized as tritone substitution pattern");
    expect(minorTritone.pattern.role == PatternMemberRole::substituteDominant
           && minorTritone.pattern.positionIndex == 1,
           "minor-key SubV is position 2/3");
    expect(minorTritone.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "minor ii-SubV-i is confirmed");

    // Applied SubV to a non-tonic scale degree: Ab7 -> G in C major.
    const auto ab7 = makeChord(-4, { 0, 4, 7, 10 });
    const auto gMajor = makeChord(1, { 0, 4, 7 });
    const auto appliedSubV = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, ab7, gMajor));
    expect(appliedSubV.harmonic.substituteDominantConfirmed,
           "Ab7->G confirms substitute dominant of V");
    expect(! appliedSubV.harmonic.appliedDominantConfirmed,
           "Ab7->G is not ordinary secondary dominant motion");
    expect(appliedSubV.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "applied SubV is not mislabeled secondary dominant");
    expect(appliedSubV.pattern.role == PatternMemberRole::substituteDominant,
           "applied SubV keeps substitute-dominant role");
    expect(! appliedSubV.localKey.valid,
           "applied SubV local-center inference is deferred to 0.2d");

    // Single applied dominant behavior from 0.2a remains intact.
    const auto appliedSituation = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, d7, gMajor));
    expect(appliedSituation.harmonic.appliedDominantConfirmed,
           "D7 to G confirms applied dominant in C");
    expect(! appliedSituation.harmonic.substituteDominantConfirmed,
           "D7 to G is not SubV");
    expect(appliedSituation.pattern.type == HarmonicPatternType::secondaryDominant,
           "single applied dominant remains secondary dominant");
    expect(appliedSituation.localKey.valid
           && appliedSituation.localKey.scope == KeyCenterScope::temporary,
           "confirmed applied dominant keeps temporary local center");
    expect(appliedSituation.localKey.key.rootPitchClass == 7,
           "D7 to G temporary center is G");

    // Generic V-I fallback is retained when no ii preparation is visible.
    const auto dominantSituation = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, g7, cMaj7));
    expect(dominantSituation.pattern.type == HarmonicPatternType::dominantToTonic,
           "V-I is recognized when no larger ii-V-I context is available");
    expect(dominantSituation.harmonic.effectiveFunction == HarmonicFunction::dominant,
           "ordinary V-I keeps dominant function");

    // Boundary false-positive guards.
    const auto falseMajorCandidate = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor,
                                dMin7,
                                makeChord(1, { 0, 3, 7, 10 }))); // Gm7
    expect(falseMajorCandidate.pattern.type != HarmonicPatternType::majorIiVI,
           "non-dominant V-quality chord does not create major ii-V-I");

    const auto falseSubCandidate = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor,
                                dMin7,
                                makeChord(-5, { 0, 3, 7, 10 }))); // Dbm7
    expect(falseSubCandidate.pattern.type != HarmonicPatternType::tritoneSubstitution,
           "non-dominant Db quality does not create ii-SubV candidate");

    TimelineHarmonicSnapshot missingKey;
    missingKey.positionAvailable = true;
    missingKey.currentChord = g7;
    const auto invalid = analyzeHarmonicSituation(missingKey);
    expect(! invalid.valid, "missing global key remains a safe non-analysis state");
    expect(! invalid.pattern.recognized(), "invalid input does not invent a pattern");
    expect(! invalid.localKey.valid, "invalid input does not invent a local center");

    std::cout << "SmartImproviser HarmonicEngineTests: OK\n";
    return 0;
}
