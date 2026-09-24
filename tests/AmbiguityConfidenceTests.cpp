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

TimelineHarmonicSnapshot currentOnly(const KeyContext& key,
                                     const ChordContext& current)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = key;
    snapshot.currentChord = current;
    return snapshot;
}

TimelineHarmonicSnapshot currentNext(const KeyContext& key,
                                     const ChordContext& current,
                                     const ChordContext& next)
{
    auto snapshot = currentOnly(key, current);
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}

TimelineHarmonicSnapshot fullWindow(const KeyContext& key,
                                    const ChordContext& previous,
                                    const ChordContext& current,
                                    const ChordContext& next)
{
    auto snapshot = currentNext(key, current, next);
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = previous;
    return snapshot;
}

const HarmonicInterpretation* find(const HarmonicSituation& situation,
                                   HarmonicInterpretationKind kind)
{
    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        if (situation.interpretations[i].valid
            && situation.interpretations[i].kind == kind)
            return &situation.interpretations[i];
    }
    return nullptr;
}
}

int main()
{
    const auto cMajor = makeKey(0, false);
    const auto fMajor = makeKey(-1, false);

    const auto cMaj7 = makeChord(0, { 0, 4, 7, 11 });
    const auto fMin7 = makeChord(-1, { 0, 3, 7, 10 });

    // Plain diatonic harmony remains a single unique interpretation.
    const auto plain = analyzeHarmonicSituation(currentOnly(cMajor, cMaj7));
    expect(plain.evidence.interpretation == InterpretationStatus::unique,
           "plain diatonic harmony is unique");
    expect(plain.interpretationCount == 1,
           "plain harmony stores one interpretation");
    expect(plain.primaryInterpretationIndex == 0,
           "plain harmony exposes global interpretation as primary");

    // Borrowed iv in C major is both chromatic in C major and diatonic in the
    // parallel C minor collection. 0.2e must preserve both readings.
    const auto borrowed = analyzeHarmonicSituation(currentOnly(cMajor, fMin7));
    expect(borrowed.evidence.interpretation == InterpretationStatus::ambiguous,
           "borrowed iv is marked ambiguous");
    expect(borrowed.primaryInterpretationIndex == -1,
           "borrowed iv has no premature primary interpretation");
    expect(borrowed.interpretationCount == 2,
           "borrowed iv stores global and modal alternatives");
    expect(borrowed.evidence.has(EvidenceFlag::borrowedAmbiguity),
           "borrowed ambiguity is recorded as evidence");
    const auto* modal = find(borrowed, HarmonicInterpretationKind::modalInterchange);
    expect(modal != nullptr
           && modal->center.key.rootPitchClass == 0
           && modal->center.key.mode == KeyMode::minor,
           "borrowed iv exposes C minor modal alternative");

    const auto eHalfDim7 = makeChord(4, { 0, 3, 6, 10 });
    const auto a7 = makeChord(3, { 0, 4, 7, 10 });
    const auto dMinor = makeChord(2, { 0, 3, 7 });

    // Unresolved iiø-V in global F major suggests D minor, but does not yet
    // justify replacing the global reading with one unique interpretation.
    const auto candidate = analyzeHarmonicSituation(currentNext(fMajor, eHalfDim7, a7));
    expect(candidate.localKey.valid
           && candidate.localKey.status == KeyCenterStatus::candidate,
           "unresolved ii-V exposes candidate local center");
    expect(candidate.evidence.interpretation == InterpretationStatus::ambiguous,
           "candidate local center keeps global/local readings ambiguous");
    expect(candidate.primaryInterpretationIndex == -1,
           "candidate center has no primary interpretation");
    expect(candidate.evidence.has(EvidenceFlag::globalLocalConflict),
           "candidate center records global/local conflict");
    expect(find(candidate, HarmonicInterpretationKind::localCenter) != nullptr,
           "candidate center is stored as an explicit alternative");

    // Once iiø-V-i is visible, confirmed local cadence evidence is decisive.
    const auto established = analyzeHarmonicSituation(
        fullWindow(fMajor, eHalfDim7, a7, dMinor));
    expect(established.localKey.valid
           && established.localKey.status == KeyCenterStatus::established,
           "complete local cadence establishes D minor");
    expect(established.evidence.interpretation == InterpretationStatus::unique,
           "confirmed local cadence resolves ambiguity");
    expect(established.primaryInterpretationIndex >= 0,
           "confirmed local cadence exposes a primary interpretation");
    expect(established.interpretations[static_cast<std::size_t>(established.primaryInterpretationIndex)].kind
               == HarmonicInterpretationKind::localCenter,
           "local interpretation wins after confirmed local cadence");
    expect(established.interpretationCount >= 2,
           "global reading is retained for diagnostics after resolution");

    // A possible modulation remains deliberately ambiguous in 0.2e.
    const auto cs7 = makeChord(7, { 0, 4, 7, 10 });
    const auto fsMaj7 = makeChord(6, { 0, 4, 7, 11 });
    const auto bMaj7 = makeChord(5, { 0, 4, 7, 11 });
    const auto modulation = analyzeHarmonicSituation(
        fullWindow(cMajor, cs7, fsMaj7, bMaj7));
    expect(modulation.localKey.valid
           && modulation.localKey.status == KeyCenterStatus::modulationCandidate,
           "remote V-I-X exposes modulation candidate");
    expect(modulation.evidence.interpretation == InterpretationStatus::ambiguous,
           "modulation candidate remains ambiguous");
    expect(modulation.primaryInterpretationIndex == -1,
           "modulation candidate never changes global key implicitly");

    std::cout << "SmartImproviser AmbiguityConfidenceTests: OK\n";
    return 0;
}
