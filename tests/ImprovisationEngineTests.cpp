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

int main()
{
    const auto key = makeKey(0, false);
    const auto g7 = makeChord(1, {0, 4, 7, 10});
    auto snapshot = makeCurrentNextSnapshot(key, g7, makeChord(0, {0, 4, 7, 11}));
    const auto major = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(major.valid && major.strategies.size() == 1, "one foundation recommendation");
    expect(major.dominantContext == DominantContext::toMajor, "G7 to major");
    const auto& notes = major.strategies[0].source.notes;
    expect(notes.size() == 4 && notes[0].pitchClass == 7 && notes[1].pitchClass == 11
           && notes[2].pitchClass == 2 && notes[3].pitchClass == 5, "G B D F, root-relative order");
    expect(! major.strategies[0].tensionClassified, "foundation does not impose T1 on explicit chords");
    snapshot.nextChord = makeChord(0, {0, 3, 7});
    const auto minor = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(minor.dominantContext == DominantContext::toMinor, "minor target overrides global major key");
    expect(minor.strategies[0].resolution.targetQuality == ChordQuality::minor, "resolution preserved");
    snapshot.nextChord = makeChord(0, {0, 4, 7, 10});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).dominantContext
           == DominantContext::toDominant, "dominant chain is not major tonic");
    snapshot.nextChord = makeChord(-4, {0, 4, 7, 11});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).dominantContext
           == DominantContext::unresolved, "deceptive next chord not invented V-I");
    snapshot.nextChordAvailable = false;
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).dominantContext
           == DominantContext::unresolved, "missing next chord");
    snapshot = makeCurrentNextSnapshot(key, makeChord(3, {0, 4, 7, 10}), makeChord(2, {0, 3, 7, 10}));
    const auto secondary = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(secondary.secondaryDominant && secondary.dominantContext == DominantContext::toMinor,
           "A7 to Dm7 secondary with minor target");
    snapshot = makeCurrentNextSnapshot(key, makeChord(-5, {0, 4, 7, 10}), makeChord(0, {0, 4, 7, 11}));
    const auto sub = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(sub.substituteDominant && sub.dominantContext == DominantContext::toMajor, "SubV context");
    auto ambiguous = analyzeHarmonicSituation(snapshot);
    ambiguous.primaryInterpretationIndex = -1;
    ambiguous.evidence.markAmbiguous(2);
    const auto result = analyzeImprovisation(ambiguous);
    expect(result.strategies[0].interpretationIndex == -1
           && result.strategies[0].interpretationIndependent, "no hidden primary selection");
    expect(result.strategies[0].evidence.interpretation == InterpretationStatus::ambiguous,
           "chord recommendation preserves ambiguity");
    snapshot = makeCurrentNextSnapshot(key, makeChord(1, {0, 5, 7, 10}), makeChord(0, {0, 4, 7}));
    auto sus = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(sus.strategies[0].source.notes.size() == 4, "sus explicit notes");
    for (const auto& n : sus.strategies[0].source.notes)
        expect(n.semitonesFromRoot != 4, "no invented third on sus");
    snapshot.currentChord = makeChord(0, {0, 4, 7});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies[0].source.notes.size() == 3,
           "no invented seventh on triad");
    snapshot.currentChord = makeChord(1, {0, 1, 4, 7, 10});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies[0].source.notes.size() == 5,
           "explicit b9 retained");
    for (int fifths = -5; fifths <= 6; ++fifths)
    {
        snapshot = makeCurrentNextSnapshot(makeKey(fifths, false),
            makeChord(fifths + 1, {0, 4, 7, 10}), makeChord(fifths, {0, 4, 7, 11}));
        const auto situation = analyzeHarmonicSituation(snapshot);
        const auto a = analyzeImprovisation(situation);
        const auto b = analyzeImprovisation(situation);
        expect(a.dominantContext == DominantContext::toMajor, "all 12 major targets");
        expect(a.contextDescription == b.contextDescription
               && a.strategies[0].source.notes[0].pitchClass == circleOfFifthsToPitchClass(fifths + 1),
               "determinism and transposition");
    }
    snapshot.globalKey.available = false;
    expect(! analyzeImprovisation(analyzeHarmonicSituation(snapshot)).valid, "missing key yields no material");
    snapshot.globalKey = key;
    snapshot.positionAvailable = false;
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies.empty(), "missing position clears suggestions");
    snapshot.positionAvailable = true;
    snapshot.currentChord.defined = false;
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies.empty(), "no chord clears suggestions");
    std::cout << "Improvisation foundation tests passed\n";
}
