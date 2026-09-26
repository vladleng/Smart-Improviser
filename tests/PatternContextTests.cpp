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
TimelineHarmonicSnapshot makeSnapshot(const KeyContext& key,
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
    return snapshot;
}

template <std::size_t N>
PatternTimelineWindow makePatternWindow(const std::array<ChordContext, N>& chords,
                                        int currentIndex)
{
    PatternTimelineWindow window;
    window.chordCount = static_cast<std::uint8_t>(N);
    window.currentIndex = currentIndex;
    for (std::size_t i = 0; i < N; ++i)
        window.chords[i] = chords[i];
    return window;
}

template <std::size_t N>
HarmonicSituation analyzeWindow(const KeyContext& key,
                                const std::array<ChordContext, N>& chords,
                                int currentIndex)
{
    return analyzeHarmonicSituation(makeSnapshot(key, chords, currentIndex),
                                    makePatternWindow(chords, currentIndex));
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

    const std::array majorCadence { gMin7, c7, fMaj7 };
    for (int index = 0; index < 3; ++index)
    {
        const auto situation = analyzeWindow(fMajor, majorCadence, index);
        expect(situation.pattern.type == HarmonicPatternType::majorIiVI,
               "major ii-V-I keeps one top-level pattern");
        expect(situation.pattern.positionIndex == index
               && situation.pattern.length == 3,
               "major ii-V-I exposes 1/3, 2/3, 3/3 positions");
        expect(situation.patternContext.valid,
               "major cadence exposes PatternContext");
    }

    const auto majorTonic = analyzeWindow(fMajor, majorCadence, 2);
    expect(majorTonic.pattern.role == PatternMemberRole::resolution,
           "major tonic remains cadence resolution");
    expect(majorTonic.patternContext.status == PatternContextStatus::completed,
           "major cadence context completes on tonic");

    const auto fallback = analyzeHarmonicSituation(
        makeFallbackSnapshot(fMajor, c7, fMaj7));
    expect(fallback.pattern.type == HarmonicPatternType::dominantToTonic
           && fallback.pattern.positionIndex == 1
           && fallback.pattern.length == 2,
           "missing PatternContext falls back to generic V-I");
    expect(! fallback.patternContext.valid,
           "Stage 2 snapshot does not invent carried history");

    const auto dHalfDim7 = makeChord(2, { 0, 3, 6, 10 }, 0.0);
    const auto g7 = makeChord(1, { 0, 4, 7, 10 }, 4.0);
    const auto cMin7 = makeChord(0, { 0, 3, 7, 10 }, 8.0);
    const std::array minorTwoFiveOne { dHalfDim7, g7, cMin7 };
    const auto minorIiTonic = analyzeWindow(cMinor, minorTwoFiveOne, 2);
    expect(minorIiTonic.pattern.type == HarmonicPatternType::minorIiHalfDimVi
           && minorIiTonic.pattern.positionIndex == 2,
           "minor ii-half-diminished-V-i keeps 3/3 on tonic");

    const auto fMin7 = makeChord(-1, { 0, 3, 7, 10 }, 0.0);
    const std::array minorIvCadence { fMin7, g7, cMin7 };
    const auto minorIvTonic = analyzeWindow(cMinor, minorIvCadence, 2);
    expect(minorIvTonic.pattern.type == HarmonicPatternType::minorIvVi
           && minorIvTonic.pattern.positionIndex == 2,
           "minor iv-V-i keeps its own 3/3 identity on tonic");

    const auto aMin7 = makeChord(3, { 0, 3, 7, 10 }, 0.0);
    const auto d7 = makeChord(2, { 0, 4, 7, 10 }, 4.0);
    const auto chainGMin7 = makeChord(1, { 0, 3, 7, 10 }, 8.0);
    const auto chainC7 = makeChord(0, { 0, 4, 7, 10 }, 12.0);
    const auto chainFMaj7 = makeChord(-1, { 0, 4, 7, 11 }, 16.0);
    const std::array chain { aMin7, d7, chainGMin7, chainC7, chainFMaj7 };

    for (int index = 0; index < 5; ++index)
    {
        const auto situation = analyzeWindow(cMajor, chain, index);
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

    const auto chainD7 = analyzeWindow(cMajor, chain, 1);
    expect(chainD7.patternContext.nestedPatternCount >= 1
           && chainD7.patternContext.nestedPatterns[0].type
                == HarmonicPatternType::secondaryDominant,
           "D7 keeps nested V/ii evidence");

    const auto chainGm = analyzeWindow(cMajor, chain, 2);
    expect(chainGm.patternContext.nestedPatternCount == 2,
           "Gm7 carries V/ii resolution and ii-V-I nested evidence");
    expect(chainGm.patternContext.nestedPatterns[1].type
                == HarmonicPatternType::majorIiVI,
           "Gm7 nested cadence is major ii-V-I");

    const auto chainTonic = analyzeWindow(cMajor, chain, 4);
    expect(chainTonic.localPattern.role == PatternMemberRole::resolution
           && chainTonic.patternContext.status == PatternContextStatus::completed,
           "extended cadence completes on Fmaj7");

    const auto dMin7At4 = makeChord(2, { 0, 3, 7, 10 }, 4.0);
    const std::array editedChain { aMin7, dMin7At4, chainGMin7, chainC7, chainFMaj7 };
    const auto afterEdit = analyzeWindow(cMajor, editedChain, 4);
    expect(afterEdit.localPattern.type == HarmonicPatternType::majorIiVI
           && afterEdit.localPattern.positionIndex == 2,
           "chord edit removes stale extended cadence but keeps valid nested cadence");

    // ---------------------------------------------------------------------
    // 0.3f fix2 real-harmony regression cases from Corcovado.

    const auto bFlat7 = makeChord(-2, { 0, 4, 7, 10 }, 4.0);
    const auto eMin7 = makeChord(4, { 0, 3, 7, 10 }, 8.0);
    const std::array falseEbCadence { fMin7, bFlat7, eMin7 };
    const auto falseEb = analyzeWindow(cMajor, falseEbCadence, 0);
    expect(! falseEb.localKey.valid,
           "Fm7-Bb7-Em7 known future vetoes false Eb-major candidate");
    expect(falseEb.pattern.type != HarmonicPatternType::majorIiVI
           && falseEb.localPattern.type != HarmonicPatternType::majorIiVI,
           "contradicted ii-V does not present major ii-V-I 1/3");
    const auto falseEbDominant = analyzeWindow(cMajor, falseEbCadence, 1);
    expect(! falseEbDominant.localKey.valid
           && falseEbDominant.localPattern.type != HarmonicPatternType::majorIiVI,
           "Bb7 with known Em7 future does not keep false Eb-major 2/3 candidate");

    const auto eMin7At0 = makeChord(4, { 0, 3, 7, 10 }, 0.0);
    const auto a7At4 = makeChord(3, { 0, 4, 7, 10 }, 4.0);
    const auto d7At8 = makeChord(2, { 0, 4, 7, 10 }, 8.0);
    const std::array falseDMajor { eMin7At0, a7At4, d7At8 };
    const auto falseDStart = analyzeWindow(cMajor, falseDMajor, 0);
    expect(! falseDStart.localKey.valid,
           "Em7-A7-D7 does not invent D-major ii-V-I from dominant target");
    const auto falseDDominant = analyzeWindow(cMajor, falseDMajor, 1);
    expect(! falseDDominant.localKey.valid
           && falseDDominant.localPattern.type != HarmonicPatternType::majorIiVI,
           "A7-D7 keeps applied-dominant logic without treating D7 as tonic major");

    auto d7OverA = makeChord(2, { 0, 4, 7, 10 }, 8.0);
    d7OverA.bass = 3;
    const auto dMin7At0 = makeChord(2, { 0, 3, 7, 10 }, 0.0);
    const auto g7At4 = makeChord(1, { 0, 4, 7, 10 }, 4.0);
    const std::array falseCResolution { dMin7At0, g7At4, d7OverA };
    const auto falseC = analyzeWindow(cMajor, falseCResolution, 0);
    expect(falseC.pattern.type != HarmonicPatternType::majorIiVI,
           "Dm7-G7-D7/A known future vetoes false C-major ii-V-I");
    const auto falseCDominant = analyzeWindow(cMajor, falseCResolution, 1);
    expect(falseCDominant.pattern.type != HarmonicPatternType::majorIiVI
           && falseCDominant.localPattern.type != HarmonicPatternType::majorIiVI,
           "G7 with known D7/A future does not keep false C-major 2/3 candidate");

    const auto aMin7At4 = makeChord(3, { 0, 3, 7, 10 }, 4.0);
    const auto dMin7At8 = makeChord(2, { 0, 3, 7, 10 }, 8.0);
    const auto g7At12 = makeChord(1, { 0, 4, 7, 10 }, 12.0);
    const std::array iiiViIiV { eMin7At0, aMin7At4, dMin7At8, g7At12 };
    for (int index = 0; index < 4; ++index)
    {
        const auto situation = analyzeWindow(cMajor, iiiViIiV, index);
        expect(situation.pattern.type == HarmonicPatternType::majorIiiViIiV,
               "Em7-Am7-Dm7-G7 is recognized as iii-vi-ii-V");
        expect(situation.pattern.positionIndex == index
               && situation.pattern.length == 4,
               "iii-vi-ii-V exposes stable 1/4 through 4/4 positions");
        expect(situation.patternContext.valid
               && situation.patternContext.center.rootPitchClass == 0,
               "iii-vi-ii-V remains centered on global C major");
    }
    const auto iiiViIiVEnd = analyzeWindow(cMajor, iiiViIiV, 3);
    expect(iiiViIiVEnd.patternContext.status == PatternContextStatus::completed
           && ! iiiViIiVEnd.pattern.evidence.has(EvidenceFlag::confirmedResolution),
           "iii-vi-ii-V completes on V without fabricating tonic resolution");

    const auto cMaj7At0 = makeChord(0, { 0, 4, 7, 11 }, 0.0);
    const std::array fullTurnaround { cMaj7At0, aMin7At4, dMin7At8, g7At12 };
    const auto fullTurnaroundIi = analyzeWindow(cMajor, fullTurnaround, 2);
    expect(fullTurnaroundIi.pattern.type == HarmonicPatternType::turnaroundIVIiiV
           && fullTurnaroundIi.pattern.positionIndex == 2,
           "actual Cmaj7-Am7-Dm7-G7 remains I-VI-ii-V 3/4");

    // ---------------------------------------------------------------------
    // 0.3f fix3 correction: Abdim in this Corcovado voicing is the rootless
    // global V7(b9) sonority (B-D-F-Ab over implied G), not a bridge to G minor.
    auto d7OverAAt0 = makeChord(2, { 0, 4, 7, 10 }, 0.0);
    d7OverAAt0.bass = 3;
    const auto abDim = makeChord(-4, { 0, 3, 6, 9 }, 4.0);
    const auto gMin7At8 = makeChord(1, { 0, 3, 7, 10 }, 8.0);
    const auto c7At12 = makeChord(0, { 0, 4, 7, 10 }, 12.0);
    const auto fMaj7At16 = makeChord(-1, { 0, 4, 7, 11 }, 16.0);
    const std::array corcovadoStart { d7OverAAt0, abDim, gMin7At8, c7At12, fMaj7At16 };

    const auto corcovadoD7 = analyzeWindow(cMajor, corcovadoStart, 0);
    expect(corcovadoD7.pattern.type == HarmonicPatternType::dominantChain
           && corcovadoD7.pattern.positionIndex == 0
           && corcovadoD7.pattern.length == 2,
           "D7/A starts V/V to rootless-V dominant chain");
    expect(! corcovadoD7.localKey.valid,
           "D7/A does not pre-assign a false G-minor local center");

    const auto corcovadoAbDim = analyzeWindow(cMajor, corcovadoStart, 1);
    expect(corcovadoAbDim.pattern.type == HarmonicPatternType::dominantChain
           && corcovadoAbDim.pattern.positionIndex == 1
           && corcovadoAbDim.pattern.length == 2,
           "Abdim completes D7 to implied-G7 dominant chain");
    expect(corcovadoAbDim.impliedDominant.valid
           && corcovadoAbDim.impliedDominant.rootPitchClass == 7
           && corcovadoAbDim.impliedDominant.flatNinth,
           "Abdim is exposed as rootless G7(b9) in explicit C major");
    expect(corcovadoAbDim.harmonic.effectiveFunction == HarmonicFunction::dominant,
           "rootless G7(b9) keeps dominant effective function");
    expect(! corcovadoAbDim.localKey.valid,
           "rootless global dominant does not invent G-minor tonicization");

    const auto corcovadoGm = analyzeWindow(cMajor, corcovadoStart, 2);
    expect(corcovadoGm.localPattern.type == HarmonicPatternType::majorIiVI
           && corcovadoGm.localPattern.positionIndex == 0
           && corcovadoGm.localKey.valid
           && corcovadoGm.localKey.key.rootPitchClass == 5
           && corcovadoGm.localKey.key.mode == KeyMode::major,
           "Gm7 cleanly starts the following ii-V-I in F major");

    const auto abDimWithE = makeChord(-4, { 0, 3, 6, 8, 9 }, 4.0);
    const std::array colouredRootlessV { d7OverAAt0, abDimWithE, gMin7At8 };
    const auto colouredDominant = analyzeWindow(cMajor, colouredRootlessV, 1);
    expect(colouredDominant.impliedDominant.valid
           && colouredDominant.impliedDominant.thirteenth,
           "explicit E color upgrades the alias to G13(b9) without changing rootless function");

    // Host chord-track triad and full voicing, each with/without explicit E.
    const std::array rootlessVoicings {
        makeChord(-4, { 0, 3, 6 }, 4.0),
        makeChord(-4, { 0, 3, 6, 8 }, 4.0),
        abDim,
        abDimWithE
    };
    for (std::size_t variant = 0; variant < rootlessVoicings.size(); ++variant)
    {
        const auto& written = rootlessVoicings[variant];
        const auto normalized = normalizeChord(written);
        const auto confidence = variant < 2 ? ConfidenceLevel::medium
                                             : ConfidenceLevel::high;
        const std::array progression {
            d7OverAAt0, written, gMin7At8, c7At12, fMaj7At16
        };
        for (int position = 0; position < 2; ++position)
        {
            const auto situation = analyzeWindow(cMajor, progression, position);
            expect(situation.pattern.type == HarmonicPatternType::dominantChain
                   && situation.pattern.positionIndex == position
                   && situation.pattern.length == 2,
                   "shell/full voicing keeps dominant chain 1/2 through 2/2");
            expect(situation.pattern.evidence.confidence == confidence,
                   "both chain positions inherit shell/full-voicing confidence");
            expect(! situation.localKey.valid
                   && ! situation.pattern.evidence.has(EvidenceFlag::confirmedResolution),
                   "implied dominant chain invents neither local G minor nor tonic resolution");
        }
        const auto situation = analyzeWindow(cMajor, progression, 1);
        expect(situation.impliedDominant.valid
               && situation.impliedDominant.rootPitchClass == 7
               && situation.impliedDominant.flatNinth
               && situation.impliedDominant.evidence.confidence == confidence,
               "b9-3-5 shell implies G7(b9), provisional until b7 is explicit");
        expect(situation.impliedDominant.thirteenth == (variant % 2 == 1),
               "thirteenth is true only for explicit E, independently of b7");
        expect(situation.harmonic.effectiveFunction == HarmonicFunction::dominant
               && situation.currentChord.quality == ChordQuality::diminished
               && situation.currentChord.rootFifths == -4
               && situation.currentChord.bassFifths == normalized.bassFifths
               && situation.currentChord.tones == normalized.tones
               && normalizedChordSymbol(situation.currentChord) == normalizedChordSymbol(normalized),
               "functional alias preserves written Abdim identity and all explicit tones");
        for (int position = 2; position < 5; ++position)
        {
            const auto local = analyzeWindow(cMajor, progression, position);
            expect(local.localKey.valid
                   && local.localKey.key.rootPitchClass == 5
                   && local.localKey.key.mode == KeyMode::major
                   && local.localPattern.type == HarmonicPatternType::majorIiVI
                   && local.localPattern.positionIndex == position - 2
                   && local.localPattern.length == 3,
                   "following Gm7-C7-Fmaj7 remains separate local F-major 1/3 through 3/3");
        }
    }

    const std::array shellOnly { rootlessVoicings[0] };
    expect(! analyzeWindow(KeyContext {}, shellOnly, 0).impliedDominant.valid,
           "diminished shell without explicit key does not invent a dominant root");
    expect(! analyzeWindow(makeKey(2, false), shellOnly, 0).impliedDominant.valid,
           "diminished shell must match the explicit key's dominant");
    const std::array incompleteShell { makeChord(-4, { 0, 3 }, 0.0) };
    expect(! analyzeWindow(cMajor, incompleteShell, 0).impliedDominant.valid,
           "missing fifth of implied dominant does not qualify as b9-3-5 shell");

    std::cout << "SmartImproviser PatternContextTests: OK\n";
    return 0;
}
