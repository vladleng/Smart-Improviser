#include "core/analysis/HarmonicEngine.h"

#include <array>
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
                       std::initializer_list<int> relativeTones,
                       double startPpq)
{
    ChordContext chord;
    chord.available = true;
    chord.defined = true;
    chord.startPpq = startPpq;
    chord.root = rootFifths;
    chord.bass = rootFifths;
    for (const auto semitone : relativeTones)
        chord.intervals.values[static_cast<std::size_t>(semitone)] = 0xFFu;
    return chord;
}

template <std::size_t N>
TimelineHarmonicSnapshot makeWindowSnapshot(const KeyContext& key,
                                            const std::array<ChordContext, N>& chords,
                                            int currentIndex)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = chords[static_cast<std::size_t>(currentIndex)].startPpq;
    snapshot.globalKey = key;
    snapshot.currentChord = chords[static_cast<std::size_t>(currentIndex)];

    if (currentIndex > 0)
    {
        snapshot.previousChordAvailable = true;
        snapshot.previousChord = chords[static_cast<std::size_t>(currentIndex - 1)];
    }
    if (currentIndex + 1 < static_cast<int>(N))
    {
        snapshot.nextChordAvailable = true;
        snapshot.nextChord = chords[static_cast<std::size_t>(currentIndex + 1)];
    }

    snapshot.patternWindow.chordCount = static_cast<std::uint8_t>(N);
    snapshot.patternWindow.currentIndex = currentIndex;
    for (std::size_t i = 0; i < N; ++i)
        snapshot.patternWindow.chords[i] = chords[i];
    return snapshot;
}

TimelineHarmonicSnapshot makeFallbackSnapshot(const KeyContext& key,
                                               const ChordContext& previous,
                                               const ChordContext& current)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = current.startPpq;
    snapshot.globalKey = key;
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = previous;
    snapshot.currentChord = current;
    return snapshot;
}
}

int main()
{
    const auto fMajor = makeKey(-1, false);
    const auto cMajor = makeKey(0, false);
    const auto cMinor = makeKey(0, true);

    const auto gMin7 = makeChord(1, { 0, 3, 7, 10 }, 0.0);
    const auto c7 = makeChord(0, { 0, 4, 7, 10 }, 4.0);
    const auto fMaj7 = makeChord(-1, { 0, 4, 7, 11 }, 8.0);

    // Confirmed major ii-V-I is one pattern through the tonic, including a
    // direct seek/reopen analysis at I without any previous runtime calls.
    const std::array majorCadence { gMin7, c7, fMaj7 };
    for (int index = 0; index < 3; ++index)
    {
        const auto situation = analyzeHarmonicSituation(
            makeWindowSnapshot(fMajor, majorCadence, index));
        expect(situation.pattern.type == HarmonicPatternType::majorIiVI,
               "major ii-V-I keeps one top-level pattern");
        expect(situation.pattern.positionIndex == index
               && situation.pattern.length == 3,
               "major ii-V-I exposes 1/3, 2/3, 3/3 positions");
        expect(situation.patternContext.valid,
               "major cadence exposes PatternContext");
    }

    const auto majorTonic = analyzeHarmonicSituation(
        makeWindowSnapshot(fMajor, majorCadence, 2));
    expect(majorTonic.pattern.role == PatternMemberRole::resolution,
           "major tonic remains cadence resolution");
    expect(majorTonic.patternContext.status == PatternContextStatus::completed,
           "major cadence context completes on tonic");

    // Without bounded confirmed context the 0.3f safety rule remains intact.
    const auto fallback = analyzeHarmonicSituation(
        makeFallbackSnapshot(fMajor, c7, fMaj7));
    expect(fallback.pattern.type == HarmonicPatternType::dominantToTonic
           && fallback.pattern.positionIndex == 1
           && fallback.pattern.length == 2,
           "missing PatternContext falls back to generic V-I");

    // Minor iiø-V-i continuity.
    const auto dHalfDim7 = makeChord(2, { 0, 3, 6, 10 }, 0.0);
    const auto g7 = makeChord(1, { 0, 4, 7, 10 }, 4.0);
    const auto cMin7 = makeChord(0, { 0, 3, 7, 10 }, 8.0);
    const std::array minorTwoFiveOne { dHalfDim7, g7, cMin7 };
    const auto minorIiTonic = analyzeHarmonicSituation(
        makeWindowSnapshot(cMinor, minorTwoFiveOne, 2));
    expect(minorIiTonic.pattern.type == HarmonicPatternType::minorIiHalfDimVi
           && minorIiTonic.pattern.positionIndex == 2,
           "minor ii-half-diminished-V-i keeps 3/3 on tonic");

    // Minor iv-V-i continuity remains distinct from iiø-V-i.
    const auto fMin7 = makeChord(-1, { 0, 3, 7, 10 }, 0.0);
    const std::array minorIvCadence { fMin7, g7, cMin7 };
    const auto minorIvTonic = analyzeHarmonicSituation(
        makeWindowSnapshot(cMinor, minorIvCadence, 2));
    expect(minorIvTonic.pattern.type == HarmonicPatternType::minorIvVi
           && minorIvTonic.pattern.positionIndex == 2,
           "minor iv-V-i keeps its own 3/3 identity on tonic");

    // Real-world local cadence: global key may still be C major while the full
    // top-level cadential event resolves to F major.
    const auto aMin7 = makeChord(3, { 0, 3, 7, 10 }, 0.0);
    const auto d7 = makeChord(2, { 0, 4, 7, 10 }, 4.0);
    const auto chainGMin7 = makeChord(1, { 0, 3, 7, 10 }, 8.0);
    const auto chainC7 = makeChord(0, { 0, 4, 7, 10 }, 12.0);
    const auto chainFMaj7 = makeChord(-1, { 0, 4, 7, 11 }, 16.0);
    const std::array chain { aMin7, d7, chainGMin7, chainC7, chainFMaj7 };

    for (int index = 0; index < 5; ++index)
    {
        const auto situation = analyzeHarmonicSituation(
            makeWindowSnapshot(cMajor, chain, index));
        expect(situation.localPattern.type == HarmonicPatternType::majorCadentialChain,
               "iii-VI7-ii-V-I is one local top-level cadence");
        expect(situation.localPattern.positionIndex == index
               && situation.localPattern.length == 5,
               "extended cadence exposes stable 1/5 through 5/5 positions");
        expect(situation.localKey.valid
               && situation.localKey.key.rootPitchClass == 5
               && situation.localKey.key.mode == KeyMode::major,
               "extended cadence keeps F major as its local center");
        expect(situation.patternContext.valid
               && situation.patternContext.center.rootPitchClass == 5,
               "extended cadence PatternContext stores F center");
    }

    const auto chainD7 = analyzeHarmonicSituation(
        makeWindowSnapshot(cMajor, chain, 1));
    expect(chainD7.patternContext.nestedPatternCount >= 1
           && chainD7.patternContext.nestedPatterns[0].type
                == HarmonicPatternType::secondaryDominant,
           "D7 keeps nested V/ii evidence");

    const auto chainGm = analyzeHarmonicSituation(
        makeWindowSnapshot(cMajor, chain, 2));
    expect(chainGm.patternContext.nestedPatternCount == 2,
           "Gm7 carries V/ii resolution and ii-V-I nested evidence");
    expect(chainGm.patternContext.nestedPatterns[1].type
                == HarmonicPatternType::majorIiVI,
           "Gm7 nested cadence is major ii-V-I");

    const auto chainTonic = analyzeHarmonicSituation(
        makeWindowSnapshot(cMajor, chain, 4));
    expect(chainTonic.localPattern.role == PatternMemberRole::resolution
           && chainTonic.patternContext.status == PatternContextStatus::completed,
           "extended cadence completes on Fmaj7");

    // Chord edit invalidates the five-member chain immediately; there is no
    // stale runtime memory. The surviving Gm7-C7-Fmaj7 sub-cadence is still
    // recognized as a normal local ii-V-I.
    const auto dMin7 = makeChord(2, { 0, 3, 7, 10 }, 4.0);
    const std::array editedChain { aMin7, dMin7, chainGMin7, chainC7, chainFMaj7 };
    const auto afterEdit = analyzeHarmonicSituation(
        makeWindowSnapshot(cMajor, editedChain, 4));
    expect(afterEdit.localPattern.type == HarmonicPatternType::majorIiVI
           && afterEdit.localPattern.positionIndex == 2,
           "chord edit removes stale extended cadence but keeps valid nested cadence");

    std::cout << "SmartImproviser PatternContextTests: OK\n";
    return 0;
}
