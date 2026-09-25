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
        if (strategy.source.kind == MaterialKind::scale && strategy.source.mode != DiatonicMode::none) return &strategy;
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
const ImprovisationStrategy* rule(const ImprovisationResult& result, const std::string& id)
{
    for (const auto& candidate : result.strategies)
        if (candidate.ruleId == id) return &candidate;
    return nullptr;
}
void expectPitches(const SourceMaterial& source, std::initializer_list<int> expected)
{
    expect(source.notes.size() == expected.size(), "source note count");
    std::size_t i = 0;
    for (const int pitch : expected) expect(source.notes[i++].pitchClass == pitch, "source pitch sequence");
}
int main()
{
    const auto key = makeKey(0,false);
    auto snapshot = makeSnapshot(key,makeChord(2,{0,3,7,10}),makeChord(1,{0,4,7,10}),makeChord(0,{0,4,7,11}));
    auto result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    auto lyd = rule(result,"boyko.melodic-minor.V");
    auto alt = rule(result,"boyko.melodic-minor.bII");
    expect(lyd && alt && result.strategies.size() == 4,"major V exposes diatonic and two special applications");
    expect(lyd->source.name == "D melodic minor" && normalizedChordSymbol(lyd->thinkingStructure) == "Dm6","fifth source and separate m6 thinking");
    expectPitches(lyd->source,{2,4,5,7,9,11,1});
    expect(lyd->source.chordRelativeNotes.back().degree == 11 && lyd->source.chordRelativeNotes.back().spelling == "C#","lydian #11 relative to G, not source seventh");
    expectPitches(alt->source,{8,10,11,1,3,5,7});
    expect(alt->source.notes[2].spelling == "Cb" && alt->source.chordRelativeNotes[2].spelling == "B","source b3 versus actual dominant major third");
    expect(alt->source.rootFifths == -4 && alt->actualChord.rootFifths == 1,"source root does not rewrite harmony");
    expect(alt->omittedChordTones.size() == 1 && alt->omittedChordTones[0] == 2,"altered application declares omitted D");
    expect(!alt->tensionClassified && alt->resolution.confirmed && alt->targetNotes[1].pitchClass == 4,"major target preserved, tension undecided");
    expect(!alt->sourceTransitions.empty(),"color targets offered separately from structural resolution");
    snapshot.nextChord = makeChord(0,{0,3,7});
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    alt = rule(result,"boyko.melodic-minor.bII");
    expect(alt && !rule(result,"boyko.melodic-minor.V") && !scale(result),"minor target gets altered, not major-only source");
    expect(alt->targetNotes[1].pitchClass == 3 && alt->resolution.targetQuality == ChordQuality::minor,"actual Eb minor target");
    // An explicit b9 is retained; a natural 9 or 13 cannot disappear into altered.
    snapshot.currentChord = makeChord(1,{0,1,4,7,10});
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(rule(result,"boyko.melodic-minor.bII"),"b9 compatible with altered");
    snapshot.currentChord = makeChord(1,{0,2,4,7,10});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"boyko.melodic-minor.bII"),"explicit natural 9 blocks altered");
    snapshot.currentChord = makeChord(1,{0,4,7,9,10});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"boyko.melodic-minor.bII"),"explicit natural 13 blocks altered");
    snapshot.currentChord = makeChord(1,{0,3,4,8,10});
    snapshot.currentChord.intervals.values[3] = 9;
    snapshot.currentChord.intervals.values[8] = 5;
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    alt = rule(result,"boyko.melodic-minor.bII");
    expect(alt && alt->source.chordRelativeNotes[4].degree == 5 && alt->source.chordRelativeNotes[4].spelling == "D#","explicit #5 spelling survives parent source Eb");

    // Explicit Db spelling and host-canonicalized C# spelling must converge on
    // the same functional SubV notation when the real target is C.
    for (const int subVRootFifths : {-5, 7})
    {
        snapshot.currentChord = makeChord(subVRootFifths,{0,4,7,10});
        for (bool minorTarget : {false,true})
        {
            snapshot.nextChord = minorTarget ? makeChord(0,{0,3,7}) : makeChord(0,{0,4,7,11});
            result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
            auto sub = rule(result,"project.subv.melodic-minor.V");
            expect(sub && sub->source.name == "Ab melodic minor" && !rule(result,"boyko.melodic-minor.bII"),"SubV separate lydian dominant rule for both target qualities");
            expect(normalizedChordSymbol(sub->actualChord) == "Db7","confirmed SubV is functionally spelled Db7 over target C");
            expect(sub->source.chordRelativeNotes.back().spelling == "G","Db SubV #11 G");
            expect(result.contextDescription.rfind("Db7 -> C",0) == 0,"Stage 3 context uses functional Db spelling");
        }
    }

    snapshot.currentChord = makeChord(1,{0,4,7,10});
    snapshot.nextChordAvailable = false;
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies.size()==1,"unresolved dominant no special source");
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = makeChord(0,{0,4,7,10});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies.size()==1,"dominant chain guarded");
    snapshot.nextChord = makeChord(0,{0,4,7,11});
    snapshot.currentChord = makeChord(1,{0,5,7,10});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies.size()==1,"sus must not receive added major third");
    auto situation = withSelectedCenter(makeKey(-2,false),makeChord(1,{0,3,7,10}));
    result = analyzeImprovisation(situation);
    auto min = rule(result,"boyko.melodic-minor.root");
    expect(min && min->source.name == "G melodic minor","minor root source");
    expect(min->source.notes.back().role == MaterialNoteRole::passingTone && min->source.notes.back().spelling == "F#","major seventh on m7 is passing");
    expect(min->omittedChordTones.size()==1 && min->omittedChordTones[0]==5,"m7 foundation F explicitly separate from melodic source");
    expect(result.strategies.front().source.notes.back().pitchClass==5,"foundation not rewritten");
    situation.currentChord.degrees[10]=9;
    expect(!rule(analyzeImprovisation(situation),"boyko.melodic-minor.root"),"omission does not hide wrong explicit degree");
    situation = withSelectedCenter(key,makeChord(4,{0,3,6,10}));
    result = analyzeImprovisation(situation);
    auto half = rule(result,"boyko.melodic-minor.bIII");
    expect(half && half->source.name=="G melodic minor","Em7b5 from bIII");
    expectPitches(half->source,{7,9,10,0,2,4,6});
    situation.currentChord.tones[1]=true;
    expect(!rule(analyzeImprovisation(situation),"boyko.melodic-minor.bIII"),"explicit flat 9 conflicts with locrian natural 2");
    situation = withSelectedCenter(key,makeChord(1,{0,3,6,9}));
    situation.currentChord.degrees[9]=7;
    result = analyzeImprovisation(situation);
    auto dim = rule(result,"boyko.diminished.whole-half");
    expect(dim,"dim7 whole half source");
    expectPitches(dim->source,{7,9,10,0,1,3,4,6});
    expect(dim->source.notes[6].spelling=="Fb" && dim->source.notes[7].spelling=="F#","octatonic seventh spellings");
    situation.currentChord.tones[9]=false;
    expect(!rule(analyzeImprovisation(situation),"boyko.diminished.whole-half"),"dim triad not automatically dim7 application");
    situation.primaryInterpretationIndex=-1;
    expect(analyzeImprovisation(situation).strategies.size()==1,"no hidden selected interpretation");
    situation = withSelectedCenter(key,makeChord(4,{0,3,6,10}));
    situation.interpretations[0].center.key.mode=KeyMode::custom;
    expect(analyzeImprovisation(situation).strategies.size()==1,"custom center unsupported");
    // Slash bass remains sounding; altered omits natural fifth, so cannot ignore it in bass.
    snapshot.currentChord=makeChord(1,{0,4,7,10});
    snapshot.currentChord.bass=2;
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"boyko.melodic-minor.bII"),"natural fifth slash bass prevents altered");
    for(int fifths=-5;fifths<=6;++fifths)
    {
        snapshot=makeCurrentNextSnapshot(makeKey(fifths,false),makeChord(fifths+1,{0,4,7,10}),makeChord(fifths,{0,3,7}));
        const auto harmonic=analyzeHarmonicSituation(snapshot);
        result=analyzeImprovisation(harmonic);
        alt=rule(result,"boyko.melodic-minor.bII");
        expect(alt && alt->source.rootPitchClass==(circleOfFifthsToPitchClass(fifths+1)+1)%12,"altered transposition twelve keys");
        const auto again=analyzeImprovisation(harmonic);
        expect(result.strategies.size()==again.strategies.size(),"deterministic catalog size");
        for(std::size_t i=0;i<result.strategies.size();++i)
        {
            expect(result.strategies[i].ruleId==again.strategies[i].ruleId && result.strategies[i].source.name==again.strategies[i].source.name,"deterministic order and names");
            expect(!result.strategies[i].tensionClassified,"no source assigned automatic tension");
        }
        expect(result.context.globalKey.key.rootFifths==harmonic.globalKey.key.rootFifths,"global center immutable");
    }
    expect(analyzeImprovisation({}).strategies.empty(),"invalid context clears all sources");
    std::cout << "Special source tests passed\n";
}
