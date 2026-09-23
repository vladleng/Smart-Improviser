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
    const auto gMajor = makeChord(1, { 0, 4, 7 });
    const auto db7 = makeChord(-5, { 0, 4, 7, 10 });

    // Major ii-V-I full V position.
    const auto majorSituation = analyzeHarmonicSituation(
        makeSnapshot(cMajor, dMin7, g7, cMaj7));
    expect(majorSituation.valid, "major ii-V-I situation is valid");
    expect(majorSituation.pattern.type == HarmonicPatternType::majorIiVI,
           "major ii-V-I is recognized");
    expect(majorSituation.pattern.role == PatternMemberRole::dominant
           && majorSituation.pattern.positionIndex == 1
           && majorSituation.pattern.length == 3,
           "major V is position 2/3 dominant");
    expect(majorSituation.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "major ii-V-I is confirmed");
    expect(! majorSituation.localKey.valid,
           "global cadence does not create redundant local center");

    // Major boundary positions remain high-confidence candidates.
    const auto majorIiPosition = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, dMin7, g7));
    expect(majorIiPosition.pattern.type == HarmonicPatternType::majorIiVI
           && majorIiPosition.pattern.role == PatternMemberRole::predominant
           && majorIiPosition.pattern.positionIndex == 0,
           "major ii boundary is position 1/3 predominant");
    expect(majorIiPosition.pattern.evidence.confidence == ConfidenceLevel::high,
           "major ii boundary remains high");

    const auto majorIPosition = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(cMajor, g7, cMaj7));
    expect(majorIPosition.pattern.type == HarmonicPatternType::majorIiVI
           && majorIPosition.pattern.role == PatternMemberRole::resolution
           && majorIPosition.pattern.positionIndex == 2,
           "major I boundary is position 3/3 resolution");

    // Minor iiø-V-i on all three visible positions.
    const auto aMinor = makeKey(3, true);
    const auto bHalfDim7 = makeChord(5, { 0, 3, 6, 10 });
    const auto e7 = makeChord(4, { 0, 4, 7, 10 });
    const auto aMinorChord = makeChord(3, { 0, 3, 7 });
    const auto bb7 = makeChord(-2, { 0, 4, 7, 10 });

    const auto minorV = analyzeHarmonicSituation(
        makeSnapshot(aMinor, bHalfDim7, e7, aMinorChord));
    expect(minorV.pattern.type == HarmonicPatternType::minorIiHalfDimVi
           && minorV.pattern.role == PatternMemberRole::dominant,
           "minor ii-half-diminished-V-i recognized at V");
    expect(minorV.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "minor full cadence is confirmed");

    const auto minorIi = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(aMinor, bHalfDim7, e7));
    expect(minorIi.pattern.type == HarmonicPatternType::minorIiHalfDimVi
           && minorIi.pattern.role == PatternMemberRole::predominant
           && minorIi.pattern.positionIndex == 0,
           "minor ii boundary recognized");

    const auto minorI = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(aMinor, e7, aMinorChord));
    expect(minorI.pattern.type == HarmonicPatternType::minorIiHalfDimVi
           && minorI.pattern.role == PatternMemberRole::resolution
           && minorI.pattern.positionIndex == 2,
           "minor tonic boundary recognized");

    // I-VI-ii-V turnaround.
    const auto turnaroundVI = analyzeHarmonicSituation(
        makeSnapshot(cMajor, cMaj7, a7, dMin7));
    expect(turnaroundVI.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnaroundVI.pattern.role == PatternMemberRole::preparation
           && turnaroundVI.pattern.positionIndex == 1,
           "turnaround VI is position 2/4 preparation");

    const auto turnaroundIi = analyzeHarmonicSituation(
        makeSnapshot(cMajor, a7, dMin7, g7));
    expect(turnaroundIi.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnaroundIi.pattern.role == PatternMemberRole::predominant
           && turnaroundIi.pattern.positionIndex == 2,
           "turnaround ii is position 3/4 predominant");

    const auto turnaroundI = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, cMaj7, aMin7));
    expect(turnaroundI.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnaroundI.pattern.role == PatternMemberRole::tonic
           && turnaroundI.pattern.positionIndex == 0,
           "turnaround start is position 1/4 tonic");
    expect(turnaroundI.pattern.evidence.confidence == ConfidenceLevel::medium,
           "turnaround pair boundary is medium");

    const auto turnaroundV = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(cMajor, dMin7, g7));
    expect(turnaroundV.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && turnaroundV.pattern.role == PatternMemberRole::dominant
           && turnaroundV.pattern.positionIndex == 3,
           "turnaround end is position 4/4 dominant");

    // Dominant chain wins over a single secondary-dominant interpretation.
    const auto dominantChain = analyzeHarmonicSituation(
        makeSnapshot(cMajor, a7, d7, g7));
    expect(dominantChain.pattern.type == HarmonicPatternType::dominantChain,
           "A7-D7-G7 is a dominant chain");
    expect(dominantChain.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "dominant chain is confirmed");

    // Major ii-SubV-I.
    const auto majorTritone = analyzeHarmonicSituation(
        makeSnapshot(cMajor, dMin7, db7, cMaj7));
    expect(majorTritone.harmonic.substituteDominantConfirmed,
           "Db7-C confirms substitute dominant");
    expect(majorTritone.harmonic.effectiveFunction == HarmonicFunction::substituteDominant,
           "Db7 effective function is substitute dominant");
    expect(majorTritone.pattern.type == HarmonicPatternType::tritoneSubstitution
           && majorTritone.pattern.role == PatternMemberRole::substituteDominant
           && majorTritone.pattern.positionIndex == 1,
           "major ii-SubV-I recognized at SubV");
    expect(majorTritone.resolution.available && majorTritone.resolution.confirmed,
           "SubV-I exposes confirmed resolution");
    expect(majorTritone.resolution.moveCount == 2,
           "SubV resolution exposes guide-tone moves");
    expect(! majorTritone.localKey.valid,
           "global ii-SubV-I does not duplicate global key");

    // Tritone-substitution boundary positions.
    const auto majorSubIi = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, dMin7, db7));
    expect(majorSubIi.pattern.type == HarmonicPatternType::tritoneSubstitution
           && majorSubIi.pattern.role == PatternMemberRole::predominant,
           "ii-SubV boundary recognized");

    const auto majorSubI = analyzeHarmonicSituation(
        makePreviousCurrentSnapshot(cMajor, db7, cMaj7));
    expect(majorSubI.pattern.type == HarmonicPatternType::tritoneSubstitution
           && majorSubI.pattern.role == PatternMemberRole::resolution,
           "SubV-I resolution boundary recognized");

    // Minor iiø-SubV-i.
    const auto minorTritone = analyzeHarmonicSituation(
        makeSnapshot(aMinor, bHalfDim7, bb7, aMinorChord));
    expect(minorTritone.pattern.type == HarmonicPatternType::tritoneSubstitution
           && minorTritone.pattern.role == PatternMemberRole::substituteDominant,
           "minor ii-half-diminished-SubV-i recognized");

    // Applied SubV now also exposes a temporary local center in 0.2d.
    const auto ab7 = makeChord(-4, { 0, 4, 7, 10 });
    const auto appliedSubV = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, ab7, gMajor));
    expect(appliedSubV.pattern.type == HarmonicPatternType::tritoneSubstitution,
           "Ab7-G is tritone substitution, not secondary dominant");
    expect(appliedSubV.localKey.valid
           && appliedSubV.localKey.key.rootPitchClass == 7
           && appliedSubV.localKey.scope == KeyCenterScope::temporary,
           "Ab7-G exposes temporary G local center in 0.2d");

    // Ordinary applied dominant behavior remains intact and also carries its
    // temporary local center through the new analyzer.
    const auto appliedSituation = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, d7, gMajor));
    expect(appliedSituation.harmonic.appliedDominantConfirmed,
           "D7-G confirms applied dominant");
    expect(appliedSituation.pattern.type == HarmonicPatternType::secondaryDominant,
           "D7-G remains secondary dominant");
    expect(appliedSituation.localKey.valid
           && appliedSituation.localKey.key.rootPitchClass == 7
           && appliedSituation.localKey.scope == KeyCenterScope::temporary,
           "D7-G keeps temporary G center");

    // Generic V-I fallback remains intact.
    const auto dominantSituation = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor, g7, cMaj7));
    expect(dominantSituation.pattern.type == HarmonicPatternType::dominantToTonic,
           "V-I fallback remains available");
    expect(dominantSituation.harmonic.effectiveFunction == HarmonicFunction::dominant,
           "ordinary V-I keeps dominant function");
    expect(! dominantSituation.localKey.valid,
           "global V-I does not create local center");

    // False-positive guards.
    const auto falseMajorCandidate = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor,
                                dMin7,
                                makeChord(1, { 0, 3, 7, 10 }))); // Gm7
    expect(falseMajorCandidate.pattern.type != HarmonicPatternType::majorIiVI,
           "non-dominant V-root chord does not create major ii-V-I");

    const auto falseSubCandidate = analyzeHarmonicSituation(
        makeCurrentNextSnapshot(cMajor,
                                dMin7,
                                makeChord(-5, { 0, 3, 7, 10 }))); // Dbm7
    expect(falseSubCandidate.pattern.type != HarmonicPatternType::tritoneSubstitution,
           "non-dominant Db quality does not create ii-SubV");

    TimelineHarmonicSnapshot missingKey;
    missingKey.positionAvailable = true;
    missingKey.currentChord = g7;
    const auto invalid = analyzeHarmonicSituation(missingKey);
    expect(! invalid.valid, "missing global key remains safe non-analysis state");
    expect(! invalid.pattern.recognized(), "invalid input invents no global pattern");
    expect(! invalid.localPattern.recognized(), "invalid input invents no local pattern");
    expect(! invalid.localKey.valid, "invalid input invents no local center");

    std::cout << "SmartImproviser HarmonicEngineTests: OK\n";
    return 0;
}
