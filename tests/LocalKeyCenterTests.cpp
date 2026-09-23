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
    snapshot.ppq = 16.0;
    snapshot.globalKey = key;
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = previous;
    snapshot.currentChord = current;
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}

TimelineHarmonicSnapshot makeCurrentNext(const KeyContext& key,
                                         const ChordContext& current,
                                         const ChordContext& next)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 16.0;
    snapshot.globalKey = key;
    snapshot.currentChord = current;
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}
}

int main()
{
    const auto cMajor = makeKey(0, false);
    const auto fMajor = makeKey(-1, false);

    const auto cMaj7 = makeChord(0, { 0, 4, 7, 11 });
    const auto dMin7 = makeChord(2, { 0, 3, 7, 10 });
    const auto g7 = makeChord(1, { 0, 4, 7, 10 });
    const auto gMajor = makeChord(1, { 0, 4, 7 });
    const auto d7 = makeChord(2, { 0, 4, 7, 10 });
    const auto ab7 = makeChord(-4, { 0, 4, 7, 10 });

    // Global ii-V-I must not create a redundant local center.
    const auto globalCadence = analyzeHarmonicSituation(
        makeSnapshot(cMajor, dMin7, g7, cMaj7));
    expect(! globalCadence.localKey.valid,
           "global ii-V-I does not duplicate global key as local center");

    // Existing secondary-dominant tonicization remains available.
    const auto tonicizedG = analyzeHarmonicSituation(
        makeCurrentNext(cMajor, d7, gMajor));
    expect(tonicizedG.localKey.valid,
           "D7-G creates temporary G center in C major");
    expect(tonicizedG.localKey.key.rootPitchClass == 7
           && tonicizedG.localKey.key.mode == KeyMode::major,
           "D7-G local center is G major");
    expect(tonicizedG.localKey.scope == KeyCenterScope::temporary
           && tonicizedG.localKey.status == KeyCenterStatus::tonicized,
           "single applied dominant is a confirmed temporary tonicization");
    expect(tonicizedG.localHarmonic.valid
           && tonicizedG.localHarmonic.rootScaleDegree == 5
           && tonicizedG.localHarmonic.effectiveFunction == HarmonicFunction::dominant,
           "D7 is V in the inferred G-major local context");

    // Applied tritone substitute also tonicizes the target without changing
    // the global project key.
    const auto subVTonicizedG = analyzeHarmonicSituation(
        makeCurrentNext(cMajor, ab7, gMajor));
    expect(subVTonicizedG.localKey.valid
           && subVTonicizedG.localKey.key.rootPitchClass == 7,
           "Ab7-G infers G local center");
    expect(subVTonicizedG.localKey.status == KeyCenterStatus::tonicized,
           "SubV target is represented as temporary tonicization");
    expect(subVTonicizedG.localHarmonic.effectiveFunction
               == HarmonicFunction::substituteDominant,
           "Ab7 is substitute dominant in local G context");

    // F major with a complete iiø-V-i to D minor: this is the primary 0.2d
    // jazz use case. Project key stays F while local center becomes D minor.
    const auto eHalfDim7 = makeChord(4, { 0, 3, 6, 10 });
    const auto a7 = makeChord(3, { 0, 4, 7, 10 });
    const auto dMinor = makeChord(2, { 0, 3, 7 });

    const auto localMinor = analyzeHarmonicSituation(
        makeSnapshot(fMajor, eHalfDim7, a7, dMinor));
    expect(localMinor.localKey.valid,
           "Em7b5-A7-Dm in F major creates a local center");
    expect(localMinor.localKey.key.rootPitchClass == 2
           && localMinor.localKey.key.mode == KeyMode::minor,
           "local center is D minor");
    expect(localMinor.localKey.scope == KeyCenterScope::local
           && localMinor.localKey.status == KeyCenterStatus::established,
           "complete local ii-V-i establishes local center");
    expect(localMinor.localKey.evidence.confidence == ConfidenceLevel::confirmed,
           "complete local cadence confirms the local center");
    expect(localMinor.localHarmonic.rootScaleDegree == 5
           && localMinor.localHarmonic.effectiveFunction == HarmonicFunction::dominant,
           "A7 is locally V of D minor");
    expect(localMinor.localPattern.type == HarmonicPatternType::minorIiHalfDimVi
           && localMinor.localPattern.role == PatternMemberRole::dominant,
           "local pattern is minor ii-half-diminished-V-i");

    // The same local center appears earlier as a candidate from iiø-V.
    const auto candidateDMinor = analyzeHarmonicSituation(
        makeCurrentNext(fMajor, eHalfDim7, a7));
    expect(candidateDMinor.localKey.valid
           && candidateDMinor.localKey.key.rootPitchClass == 2
           && candidateDMinor.localKey.key.mode == KeyMode::minor,
           "Em7b5-A7 suggests D minor before the tonic arrives");
    expect(candidateDMinor.localKey.status == KeyCenterStatus::candidate
           && candidateDMinor.localKey.scope == KeyCenterScope::temporary,
           "unresolved ii-V is a candidate center, not an established local key");
    expect(candidateDMinor.localHarmonic.rootScaleDegree == 2
           && candidateDMinor.localHarmonic.effectiveFunction == HarmonicFunction::predominant,
           "Em7b5 is locally ii in candidate D minor");

    // Remote local major center, independent of the global key's scale degrees.
    const auto gsMin7 = makeChord(8, { 0, 3, 7, 10 });
    const auto cs7 = makeChord(7, { 0, 4, 7, 10 });
    const auto fsMaj7 = makeChord(6, { 0, 4, 7, 11 });
    const auto localFs = analyzeHarmonicSituation(
        makeSnapshot(cMajor, gsMin7, cs7, fsMaj7));
    expect(localFs.localKey.valid
           && localFs.localKey.key.rootPitchClass == 6
           && localFs.localKey.key.mode == KeyMode::major,
           "G#m7-C#7-F#maj7 establishes remote F# major local center");
    expect(localFs.localKey.scope == KeyCenterScope::local
           && localFs.localKey.status == KeyCenterStatus::established,
           "remote complete ii-V-I is an established local center");
    expect(localFs.localPattern.type == HarmonicPatternType::majorIiVI,
           "remote local cadence is recognized as major ii-V-I");

    // V-I-X where X supports the new center but conflicts with the global key
    // is deliberately only a modulation candidate, never an automatic project
    // key change.
    const auto bMaj7 = makeChord(5, { 0, 4, 7, 11 });
    const auto modulationCandidate = analyzeHarmonicSituation(
        makeSnapshot(cMajor, cs7, fsMaj7, bMaj7));
    expect(modulationCandidate.localKey.valid
           && modulationCandidate.localKey.key.rootPitchClass == 6,
           "C#7-F#maj7-Bmaj7 keeps F# local center");
    expect(modulationCandidate.localKey.scope == KeyCenterScope::local
           && modulationCandidate.localKey.status == KeyCenterStatus::modulationCandidate,
           "post-tonic local evidence creates modulation candidate only");
    expect(modulationCandidate.localKey.evidence.has(EvidenceFlag::modulationEvidence),
           "modulation candidate records explicit evidence");

    // Relative-minor resolution remains a temporary local interpretation when
    // the third visible chord fits both local and global collections.
    const auto gMin7 = makeChord(1, { 0, 3, 7, 10 });
    const auto relativeMinorResolution = analyzeHarmonicSituation(
        makeSnapshot(fMajor, a7, dMinor, gMin7));
    expect(relativeMinorResolution.localKey.valid
           && relativeMinorResolution.localKey.key.rootPitchClass == 2,
           "A7-Dm keeps D minor visible at the resolution position");
    expect(relativeMinorResolution.localKey.status == KeyCenterStatus::tonicized,
           "shared-key-signature continuation does not overclaim modulation");

    std::cout << "SmartImproviser LocalKeyCenterTests: OK\n";
    return 0;
}
