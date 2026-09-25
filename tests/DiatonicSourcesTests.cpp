#include "core/analysis/ImprovisationEngine.h"
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

const ImprovisationStrategy* scale(const ImprovisationResult& result)
{
    for (const auto& strategy : result.strategies)
        if (strategy.source.kind == MaterialKind::scale) return &strategy;
    return nullptr;
}
HarmonicSituation withSelectedCenter(const KeyContext& key, const ChordContext& chord)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.globalKey = key;
    snapshot.currentChord = chord;
    auto situation = buildHarmonicSituation(snapshot);
    situation.interpretationCount = 1;
    situation.primaryInterpretationIndex = 0;
    auto& selected = situation.interpretations[0];
    selected.valid = true;
    selected.center = situation.globalKey;
    selected.harmonic = situation.harmonic;
    selected.kind = HarmonicInterpretationKind::globalContext;
    selected.evidence = situation.evidence;
    return situation;
}
int main()
{
    const auto cMajor = makeKey(0, false);
    auto snapshot = makeSnapshot(cMajor, makeChord(0,{0,4,7,11}), makeChord(2,{0,3,7,10}), makeChord(1,{0,4,7,10}));
    auto result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(scale(result) && scale(result)->source.mode == DiatonicMode::dorian, "ii in C -> D Dorian");
    expect(scale(result)->source.notes[5].spelling == "B", "Dorian natural sixth");
    snapshot = makeSnapshot(cMajor, makeChord(2,{0,3,7,10}), makeChord(1,{0,4,7,10}), makeChord(0,{0,4,7,11}));
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(scale(result) && scale(result)->source.mode == DiatonicMode::mixolydian, "G7 to Cmaj -> Mixolydian");
    expect(scale(result)->source.notes[6].spelling == "F", "G Mixo b7");
    expect(scale(result)->source.notes[0].degree == 1 && scale(result)->source.notes[6].degree == 7,
           "relative degrees retained");
    expect(scale(result)->resolution.confirmed && scale(result)->guideNotes.size() == 2,
           "scale strategy retains guides and resolution");
    expect(! scale(result)->tensionClassified && !scale(result)->interpretationIndependent,
           "no premature T1 classification or independent harmony claim");
    snapshot.nextChord = makeChord(0,{0,3,7});
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(!scale(result) && result.strategies.front().resolution.confirmed, "minor target keeps anchors, not Mixo");
    snapshot.nextChordAvailable = false;
    expect(!scale(analyzeImprovisation(analyzeHarmonicSituation(snapshot))), "unresolved dominant no default Mixo");
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = makeChord(0,{0,4,7,10});
    expect(!scale(analyzeImprovisation(analyzeHarmonicSituation(snapshot))), "chain not major tonic");
    snapshot.currentChord = makeChord(-5,{0,4,7,10});
    snapshot.nextChord = makeChord(0,{0,4,7,11});
    expect(!scale(analyzeImprovisation(analyzeHarmonicSituation(snapshot))), "SubV waits for dedicated sources");
    snapshot.currentChord = makeChord(1,{0,1,4,7,10});
    expect(!scale(analyzeImprovisation(analyzeHarmonicSituation(snapshot))), "b9 conflicts with Mixolydian");
    snapshot.currentChord = makeChord(1,{0,5,7,10});
    expect(!scale(analyzeImprovisation(analyzeHarmonicSituation(snapshot))), "sus no added source third");
    // Selected-center unit cases for the seven rotations, without re-testing Stage 2 ranking.
    const int roots[] = {0,2,4,-1,1,3,5};
    const DiatonicMode expected[] = {DiatonicMode::ionian,DiatonicMode::dorian,DiatonicMode::phrygian,
        DiatonicMode::lydian,DiatonicMode::mixolydian,DiatonicMode::aeolian,DiatonicMode::locrian};
    for (int i = 0; i < 7; ++i)
    {
        if (i == 4) continue; // Dominants require a real target, tested above.
        const auto chord = i == 0 || i == 3 ? makeChord(roots[i],{0,4,7,11})
            : i == 6 ? makeChord(roots[i],{0,3,6,10}) : makeChord(roots[i],{0,3,7,10});
        const auto situation = withSelectedCenter(cMajor,chord);
        result = analyzeImprovisation(situation);
        expect(scale(result) && scale(result)->source.mode == expected[i], "mode follows selected center");
        expect(scale(result)->source.notes.size() == 7, "seven spelled degrees");
        expect(result.context.globalKey.key.rootFifths == situation.globalKey.key.rootFifths, "no key mutation");
    }
    auto situation = withSelectedCenter(cMajor, makeChord(2,{0,3,7,10}));
    situation.primaryInterpretationIndex = -1;
    result = analyzeImprovisation(situation);
    expect(!scale(result) && result.valid, "unresolved primary retains only chord strategy");
    situation = withSelectedCenter(cMajor, makeChord(2,{0,3,7,10}));
    situation.interpretations[0].center = makeGlobalKeyCenter(normalizeKey(makeKey(-2,false)));
    situation.interpretations[0].kind = HarmonicInterpretationKind::localCenter;
    result = analyzeImprovisation(situation);
    expect(scale(result) && scale(result)->source.mode == DiatonicMode::phrygian,
           "D minor in selected Bb major uses Phrygian, not global C Dorian");
    situation.interpretations[0].center.key.mode = KeyMode::custom;
    expect(!scale(analyzeImprovisation(situation)), "custom source unsupported, no invented major");
    situation = withSelectedCenter(makeKey(3,true),makeChord(3,{0,3,7,10}));
    expect(scale(analyzeImprovisation(situation))->source.mode == DiatonicMode::aeolian, "minor tonic Aeolian");
    situation = withSelectedCenter(cMajor, makeChord(0,{0,4,7,9,11}));
    situation.currentChord.degrees[9] = 7; // Explicit bb7 cannot silently become scale degree 6.
    expect(!scale(analyzeImprovisation(situation)), "explicit degree mismatch cannot be hidden by pitch-class match");
    for (int fifths = -5; fifths <= 6; ++fifths)
    {
        snapshot = makeCurrentNextSnapshot(makeKey(fifths,false),makeChord(fifths+1,{0,4,7,10}),makeChord(fifths,{0,4,7,11}));
        result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
        expect(scale(result) && scale(result)->source.mode == DiatonicMode::mixolydian, "dominant source in twelve keys");
        expect(scale(result)->source.notes[4].pitchClass == (circleOfFifthsToPitchClass(fifths+1)+7)%12,
               "transposed source fifth");
        const auto again = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
        expect(scale(result)->source.name == scale(again)->source.name
               && scale(result)->source.notes[6].spelling == scale(again)->source.notes[6].spelling, "deterministic source");
    }
    result = analyzeImprovisation(withSelectedCenter(makeKey(7,false),makeChord(7,{0,4,7,11})));
    expect(scale(result) && scale(result)->source.notes[2].spelling == "E#"
           && scale(result)->source.notes[6].spelling == "B#", "C# major degree spelling");
    result = analyzeImprovisation(withSelectedCenter(makeKey(-6,false),makeChord(-6,{0,4,7,11})));
    expect(scale(result) && scale(result)->source.notes[3].spelling == "Cb", "Gb major Cb spelling");
    result = analyzeImprovisation(withSelectedCenter(makeKey(8,false),makeChord(8,{0,4,7,11})));
    expect(scale(result) && scale(result)->source.notes[6].spelling == "F##", "double accidental spelling");
    expect(analyzeImprovisation({}).strategies.empty(), "invalid context no scale");
    std::cout << "Diatonic source tests passed\n";
}
