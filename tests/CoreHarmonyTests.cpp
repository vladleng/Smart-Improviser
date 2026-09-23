#include "core/analysis/HarmonicFunction.h"
#include "core/model/ChordModel.h"
#include "core/model/KeyModel.h"

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
}

int main()
{
    const auto cMajor = normalizeKey(makeKey(0, false));
    expect(cMajor.valid, "C major normalizes");
    expect(cMajor.mode == KeyMode::major, "C major mode");
    expect(scaleDegreeForPitchClass(cMajor, 0) == 1, "C is I");
    expect(scaleDegreeForPitchClass(cMajor, 2) == 2, "D is II");

    const auto dMin7 = normalizeChord(makeChord(2, { 0, 3, 7, 10 }));
    const auto g7 = normalizeChord(makeChord(1, { 0, 4, 7, 10 }));
    const auto cMaj7 = normalizeChord(makeChord(0, { 0, 4, 7, 11 }));

    expect(dMin7.quality == ChordQuality::minor, "Dm7 quality");
    expect(g7.quality == ChordQuality::dominant, "G7 quality");
    expect(cMaj7.quality == ChordQuality::major, "Cmaj7 quality");

    const auto dAnalysis = analyzeHarmonicFunction(dMin7, cMajor);
    const auto gAnalysis = analyzeHarmonicFunction(g7, cMajor, cMaj7);
    const auto cAnalysis = analyzeHarmonicFunction(cMaj7, cMajor);

    expect(dAnalysis.rootScaleDegree == 2, "Dm7 is ii in C");
    expect(dAnalysis.rootFunction == HarmonicFunction::predominant, "ii is predominant");
    expect(gAnalysis.rootScaleDegree == 5, "G7 is V in C");
    expect(gAnalysis.effectiveFunction == HarmonicFunction::dominant, "V7 is dominant");
    expect(gAnalysis.dominantResolutionConfirmed, "G7->C resolution confirmed");
    expect(! gAnalysis.substituteDominantConfirmed, "ordinary V7 is not SubV7");
    expect(cAnalysis.rootFunction == HarmonicFunction::tonic, "I is tonic");

    const auto d7 = normalizeChord(makeChord(2, { 0, 4, 7, 10 }));
    const auto gMajor = normalizeChord(makeChord(1, { 0, 4, 7 }));
    const auto d7ToG = analyzeHarmonicFunction(d7, cMajor, gMajor);
    expect(d7ToG.appliedDominantCandidate, "D7 in C is applied dominant candidate");
    expect(d7ToG.appliedTargetScaleDegree == 5, "D7 targets V");
    expect(d7ToG.appliedDominantConfirmed, "D7->G confirms V/V");
    expect(! d7ToG.substituteDominantConfirmed, "D7->G is not a tritone substitute");

    // Db7 is the tritone substitute of G7 and resolves to C by root semitone.
    const auto db7 = normalizeChord(makeChord(-5, { 0, 4, 7, 10 }));
    const auto db7Static = analyzeHarmonicFunction(db7, cMajor);
    expect(db7Static.substituteDominantCandidate, "Db7 in C is SubV candidate");
    expect(db7Static.substituteTargetScaleDegree == 1, "Db7 substitute target is tonic");
    expect(db7Static.effectiveFunction == HarmonicFunction::substituteDominant,
           "unambiguous Db7 exposes substitute-dominant function");

    const auto db7ToC = analyzeHarmonicFunction(db7, cMajor, cMaj7);
    expect(db7ToC.substituteDominantConfirmed, "Db7->C confirms SubV/I");
    expect(! db7ToC.dominantResolutionConfirmed,
           "Db7->C is not ordinary dominant-resolution motion");
    expect(! db7ToC.appliedDominantConfirmed,
           "Db7->C is not an applied dominant");
    expect(db7ToC.effectiveFunction == HarmonicFunction::substituteDominant,
           "confirmed Db7->C keeps substitute-dominant function");

    const auto fMin7 = normalizeChord(makeChord(-1, { 0, 3, 7, 10 }));
    const auto fMinInC = analyzeHarmonicFunction(fMin7, cMajor);
    expect(fMinInC.relation == HarmonicRelation::chromatic, "Fm7 is chromatic in C major");
    expect(fMinInC.modalInterchangeCandidate, "Fm7 is parallel-minor borrowing candidate");
    expect(fMinInC.modalInterchangeSource == KeyMode::minor, "Fm7 borrowing source is minor");

    std::cout << "SmartImproviser CoreHarmonyTests: OK\n";
    return 0;
}
