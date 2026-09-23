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
}

int main()
{
    const auto cMajor = makeKey(0, false);
    const auto majorIiVI = makeSnapshot(
        cMajor,
        makeChord(2, { 0, 3, 7, 10 }),  // Dm7
        makeChord(1, { 0, 4, 7, 10 }),  // G7
        makeChord(0, { 0, 4, 7, 11 })); // Cmaj7

    const auto majorSituation = analyzeHarmonicSituation(majorIiVI);
    expect(majorSituation.valid, "major ii-V-I situation is valid");
    expect(majorSituation.pattern.type == HarmonicPatternType::majorIiVI,
           "major ii-V-I is recognized");
    expect(majorSituation.pattern.role == PatternMemberRole::dominant,
           "current G7 is dominant member of major ii-V-I");
    expect(majorSituation.pattern.positionIndex == 1 && majorSituation.pattern.length == 3,
           "major ii-V-I pattern position is preserved");
    expect(majorSituation.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "major ii-V-I is confirmed by real next-chord resolution");
    expect(majorSituation.evidence.has(EvidenceFlag::patternMatch),
           "situation carries pattern evidence");
    expect(! majorSituation.localKey.valid,
           "primary dominant does not invent a temporary local key");

    const auto aMinor = makeKey(3, true);
    const auto minorIiVI = makeSnapshot(
        aMinor,
        makeChord(5, { 0, 3, 6, 10 }), // Bm7b5
        makeChord(4, { 0, 4, 7, 10 }), // E7
        makeChord(3, { 0, 3, 7 }));    // Am

    const auto minorSituation = analyzeHarmonicSituation(minorIiVI);
    expect(minorSituation.pattern.type == HarmonicPatternType::minorIiHalfDimVi,
           "minor ii-half-diminished-V-i is recognized");
    expect(minorSituation.pattern.evidence.confidence == ConfidenceLevel::confirmed,
           "minor ii-V-i is confirmed");

    TimelineHarmonicSnapshot applied;
    applied.positionAvailable = true;
    applied.ppq = 12.0;
    applied.globalKey = cMajor;
    applied.currentChord = makeChord(2, { 0, 4, 7, 10 }); // D7
    applied.nextChordAvailable = true;
    applied.nextChord = makeChord(1, { 0, 4, 7 }); // G

    const auto appliedSituation = analyzeHarmonicSituation(applied);
    expect(appliedSituation.harmonic.appliedDominantConfirmed,
           "D7 to G confirms applied dominant in C");
    expect(appliedSituation.pattern.type == HarmonicPatternType::secondaryDominant,
           "secondary dominant wins over generic dominant-to-target pattern");
    expect(appliedSituation.localKey.valid,
           "confirmed applied dominant creates temporary local center");
    expect(appliedSituation.localKey.scope == KeyCenterScope::temporary,
           "local center is explicitly temporary, not a modulation");
    expect(appliedSituation.localKey.key.rootPitchClass == 7,
           "D7 to G temporary center is G");
    expect(appliedSituation.localKey.key.mode == KeyMode::major,
           "major target produces temporary major center");
    expect(appliedSituation.evidence.has(EvidenceFlag::inferredLocalCenter),
           "temporary center is evidence-backed");

    TimelineHarmonicSnapshot dominantOnly;
    dominantOnly.positionAvailable = true;
    dominantOnly.ppq = 16.0;
    dominantOnly.globalKey = cMajor;
    dominantOnly.currentChord = makeChord(1, { 0, 4, 7, 10 }); // G7
    dominantOnly.nextChordAvailable = true;
    dominantOnly.nextChord = makeChord(0, { 0, 4, 7, 11 }); // Cmaj7

    const auto dominantSituation = analyzeHarmonicSituation(dominantOnly);
    expect(dominantSituation.pattern.type == HarmonicPatternType::dominantToTonic,
           "V-I is recognized when no larger ii-V-I context is available");

    TimelineHarmonicSnapshot missingKey;
    missingKey.positionAvailable = true;
    missingKey.currentChord = makeChord(1, { 0, 4, 7, 10 });
    const auto invalid = analyzeHarmonicSituation(missingKey);
    expect(! invalid.valid, "missing global key remains a safe non-analysis state");
    expect(! invalid.pattern.recognized(), "invalid input does not invent a pattern");
    expect(! invalid.localKey.valid, "invalid input does not invent a local center");

    std::cout << "SmartImproviser HarmonicEngineTests: OK\n";
    return 0;
}
