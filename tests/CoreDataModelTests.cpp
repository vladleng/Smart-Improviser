#include "core/model/HarmonicSituation.h"
#include "core/model/ImprovisationContracts.h"
#include "core/model/Phrase.h"

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
    // Reference major ii-V-I context: Dm7 -> G7 -> Cmaj7 in C major.
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = makeKey(0, false); // C major
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = makeChord(2, { 0, 3, 7, 10 }); // Dm7
    snapshot.currentChord = makeChord(1, { 0, 4, 7, 10 }); // G7
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = makeChord(0, { 0, 4, 7, 11 }); // Cmaj7

    const auto situation = buildHarmonicSituation(snapshot);

    expect(situation.valid, "HarmonicSituation is valid");
    expect(situation.ppq == 8.0, "HarmonicSituation preserves PPQ");
    expect(situation.previousChordAvailable, "previous chord is present");
    expect(situation.nextChordAvailable, "next chord is present");
    expect(situation.currentChord.quality == ChordQuality::dominant, "current chord is G7 dominant");
    expect(situation.globalKey.valid, "global KeyCenter is valid");
    expect(situation.globalKey.scope == KeyCenterScope::global, "global KeyCenter scope");
    expect(! situation.localKey.valid, "local KeyCenter is not guessed by data-model layer");
    expect(situation.harmonic.rootScaleDegree == 5, "G7 is degree V in C major");
    expect(situation.harmonic.effectiveFunction == HarmonicFunction::dominant, "G7 effective function is dominant");
    expect(situation.pattern.type == HarmonicPatternType::none, "pattern recognition is deferred to analyzer");
    expect(! situation.pattern.recognized(), "none pattern is not recognized");

    HarmonicPattern explicitPattern;
    explicitPattern.type = HarmonicPatternType::majorIiVI;
    explicitPattern.role = PatternMemberRole::dominant;
    explicitPattern.positionIndex = 1;
    explicitPattern.length = 3;
    explicitPattern.evidence.confidence = ConfidenceLevel::high;
    explicitPattern.evidence.add(EvidenceFlag::patternMatch);
    expect(explicitPattern.recognized(), "major ii-V-I pattern contract reports recognized");
    expect(explicitPattern.positionIndex == 1 && explicitPattern.length == 3,
           "HarmonicPattern preserves position inside pattern");

    expect(situation.resolution.available, "dominant resolution target is available");
    expect(situation.resolution.confirmed, "dominant resolution target is confirmed");
    expect(situation.resolution.targetPitchClass == 0, "G7 resolves to C pitch class");
    expect(situation.resolution.targetQuality == ChordQuality::major, "resolution target quality is major");
    expect(situation.resolution.moveCount == 2, "two structural dominant resolution moves are generated");
    expect(situation.resolution.moves[0].fromPitchClass == 11
           && situation.resolution.moves[0].toPitchClass == 0
           && situation.resolution.moves[0].semitoneDelta == 1,
           "dominant third resolves B to C");
    expect(situation.resolution.moves[1].fromPitchClass == 5
           && situation.resolution.moves[1].toPitchClass == 4
           && situation.resolution.moves[1].semitoneDelta == -1,
           "dominant seventh resolves F to E");

    expect(situation.evidence.has(EvidenceFlag::explicitChord), "explicit chord evidence is preserved");
    expect(situation.evidence.has(EvidenceFlag::explicitKey), "explicit key evidence is preserved");
    expect(situation.evidence.has(EvidenceFlag::previousChord), "previous chord evidence is preserved");
    expect(situation.evidence.has(EvidenceFlag::nextChord), "next chord evidence is preserved");
    expect(situation.evidence.has(EvidenceFlag::confirmedResolution), "resolution evidence is preserved");
    expect(situation.evidence.confidence == ConfidenceLevel::confirmed, "confirmed resolution raises confidence");

    // Reference minor iiø-V-i context: Bm7b5 -> E7 -> Am in A minor.
    TimelineHarmonicSnapshot minorSnapshot;
    minorSnapshot.positionAvailable = true;
    minorSnapshot.ppq = 16.0;
    minorSnapshot.globalKey = makeKey(3, true); // A minor
    minorSnapshot.previousChordAvailable = true;
    minorSnapshot.previousChord = makeChord(5, { 0, 3, 6, 10 }); // Bm7b5
    minorSnapshot.currentChord = makeChord(4, { 0, 4, 7, 10 }); // E7
    minorSnapshot.nextChordAvailable = true;
    minorSnapshot.nextChord = makeChord(3, { 0, 3, 7 }); // Am

    const auto minorSituation = buildHarmonicSituation(minorSnapshot);
    expect(minorSituation.valid, "minor HarmonicSituation is valid");
    expect(minorSituation.harmonic.rootScaleDegree == 5, "E7 is V in A minor");
    expect(minorSituation.resolution.confirmed, "E7 to Am resolution confirmed");
    expect(minorSituation.resolution.targetPitchClass == 9, "E7 resolves to A pitch class");
    expect(minorSituation.resolution.targetQuality == ChordQuality::minor, "minor target quality is preserved");
    expect(minorSituation.resolution.moveCount == 2, "minor dominant has two structural resolution moves");
    expect(minorSituation.resolution.moves[0].fromPitchClass == 8
           && minorSituation.resolution.moves[0].toPitchClass == 9
           && minorSituation.resolution.moves[0].semitoneDelta == 1,
           "E7 third resolves G# to A");
    expect(minorSituation.resolution.moves[1].fromPitchClass == 2
           && minorSituation.resolution.moves[1].toPitchClass == 0
           && minorSituation.resolution.moves[1].semitoneDelta == -2,
           "E7 seventh resolves D to C");

    TimelineHarmonicSnapshot noKey;
    noKey.currentChord = snapshot.currentChord;
    expect(! buildHarmonicSituation(noKey).valid, "missing key produces safe invalid situation");

    expect(static_cast<int>(TensionLevel::stable) == 1, "Tension 1 contract");
    expect(static_cast<int>(TensionLevel::color) == 2, "Tension 2 contract");
    expect(static_cast<int>(TensionLevel::outsideMaximum) == 3, "Tension 3 contract");

    Phrase phrase;
    phrase.id = "major-ii-v-i-example";
    phrase.harmonicPattern = HarmonicPatternType::majorIiVI;
    phrase.tensionLevel = TensionLevel::color;
    phrase.role = PhraseRole::development;
    phrase.startDegree = 5;
    phrase.targetDegree = 3;
    phrase.notes.push_back({ { 0, 5, 0 }, 0.0, 0.5, false });
    phrase.notes.push_back({ { 2, 3, 0 }, 1.0, 0.5, true });

    expect(phrase.notes.size() == 2, "Phrase stores semantic relative notes");
    expect(phrase.notes[1].pitch.chordIndex == 2, "Phrase note keeps harmonic slot");
    expect(phrase.notes[1].target, "Phrase note can be marked as target");

    std::cout << "SmartImproviser CoreDataModelTests: OK\n";
    return 0;
}
