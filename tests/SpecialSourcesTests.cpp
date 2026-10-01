#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/MaterialViewer.h"

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
    const auto overlay = rule(result,"project.melodic-minor.bVII-overlay");
    expect(lyd && alt && overlay && rule(result,"levine.dominant.half-whole")
           && result.strategies.size() == 6,"major V offers bVII overlay alongside basic and diminished colors");
    expect(overlay->source.name == "F melodic minor"
           && normalizedChordSymbol(overlay->thinkingStructure) == "Fm6"
           && overlay->omittedChordTones.size() == 1
           && overlay->omittedChordTones[0] == 11
           && overlay->guideNotes.size() >= 2,
           "F melodic is a conditional overlay retaining G7's written B guide outside the source");
    expectPitches(overlay->source,{5,7,8,10,0,2,4});
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
    expect(!rule(result,"project.melodic-minor.minor-V-b13")
           && !rule(result,"levine.harmonic-minor.minor-V-fragment"),
           "plain V-to-minor does not promote rare fifth mode or harmonic-minor fragment");
    // An explicit b9 is retained; a natural 9 or 13 cannot disappear into altered.
    snapshot.currentChord = makeChord(1,{0,1,4,7,10});
    snapshot.currentChord.intervals.values[1] = 9;
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(rule(result,"boyko.melodic-minor.bII"),"b9 compatible with altered");
    const auto fragment = rule(result,"levine.harmonic-minor.minor-V-fragment");
    expect(fragment && fragment->kind == ImprovisationStrategyKind::harmonicMinorFragment
           && fragment->source.name == "G harmonic-minor V fragment"
           && result.dominantContext == DominantContext::toMinor,
           "written G7b9 resolving to Cm offers a six-note Levine V fragment");
    expectPitches(fragment->source,{7,8,11,2,3,5});
    expect(fragment->source.notes[1].spelling == "Ab"
           && fragment->source.notes[4].spelling == "Eb"
           && fragment->source.chordRelativeNotes[4].spelling == "Eb",
           "minor V fragment preserves b9 and b13 spelling and omits C11");
    snapshot.nextChord = makeChord(0,{0,4,7,11});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),
                 "levine.harmonic-minor.minor-V-fragment"),
           "actual major I excludes minor V fragment despite written b9");
    snapshot.nextChord = makeChord(0,{0,3,7});
    snapshot.currentChord = makeChord(1,{0,2,4,7,10});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"boyko.melodic-minor.bII"),"explicit natural 9 blocks altered");
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"levine.dominant.half-whole"),
           "explicit natural 9 blocks half-whole despite optional alteration on plain G7");
    snapshot.currentChord = makeChord(1,{0,4,7,9,10});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"boyko.melodic-minor.bII"),"explicit natural 13 blocks altered");
    expect(rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"levine.dominant.half-whole"),
           "natural 13 E belongs to G half-whole diminished");
    snapshot.currentChord = makeChord(1,{0,4,7,8,10});
    snapshot.currentChord.intervals.values[8] = 13;
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"levine.dominant.half-whole"),
           "explicit b13 blocks half-whole even in a dominant context");
    auto b13 = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(rule(b13,"project.melodic-minor.minor-V-b13")
           && rule(b13,"boyko.melodic-minor.bII"),
           "written b13 still offers compatible fifth-mode and altered choices");
    expect(rule(b13,"project.melodic-minor.minor-V-b13")->priority <
               rule(b13,"boyko.melodic-minor.bII")->priority
           && rule(b13,"levine.harmonic-minor.minor-V-fragment"),
           "rare fifth mode remains conditional below altered, while minor resolution permits V fragment");
    snapshot.currentChord.intervals.values[8] = 5;
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),
                 "levine.harmonic-minor.minor-V-fragment"),
           "written sharp fifth cannot silently become fragment flat thirteenth");
    snapshot.currentChord.intervals.values[8] = 13;
    snapshot = makeSnapshot(key,makeChord(4,{0,3,7,10}),makeChord(3,{0,4,7,8,10}),makeChord(2,{0,4,7,10}));
    snapshot.currentChord.intervals.values[8] = 13;
    b13 = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    const auto b13Mode = rule(b13,"project.melodic-minor.minor-V-b13");
    expect(b13Mode && b13Mode->source.name == "D melodic minor"
           && b13Mode->interpretationIndependent
           && b13.dominantContext == DominantContext::toDominant
           && b13Mode->nextChord.quality == ChordQuality::dominant
           && rule(b13,"boyko.melodic-minor.bII"),
           "Em7-A7b13-D7 receives chord-local options without inventing D minor");
    expect(!rule(b13,"levine.harmonic-minor.minor-V-fragment"),
           "b13 into dominant D7 does not imply D minor harmonic source");
    snapshot = makeSnapshot(key,makeChord(2,{0,3,7,10}),makeChord(1,{0,4,7,10}),makeChord(0,{0,3,7}));
    snapshot.currentChord = makeChord(1,{0,2,4,7,10});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"project.melodic-minor.bVII-overlay"),
           "explicit natural 9 blocks bVII melodic overlay's b9");
    snapshot = makeSnapshot(key,makeChord(2,{0,3,7,10}),makeChord(1,{0,4,7,10}),makeChord(0,{0,4,7,11}));
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
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(result.strategies.size()==3 && rule(result,"levine.dominant.half-whole")
           && !rule(result,"boyko.melodic-minor.bII"),"unresolved dominant has chord-local baseline and optional diminished color");
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = makeChord(0,{0,4,7,10});
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(result.strategies.size()==3 && result.dominantContext == DominantContext::toDominant
           && !rule(result,"boyko.melodic-minor.bII"),"dominant chain gets chord-local options without false tonic");
    snapshot.nextChord = makeChord(0,{0,4,7,11});
    snapshot.currentChord = makeChord(1,{0,5,7,10});
    expect(analyzeImprovisation(analyzeHarmonicSituation(snapshot)).strategies.size()==1,"sus must not receive added major third");
    auto situation = withSelectedCenter(makeKey(-1,false),makeChord(1,{0,3,7,10}));
    result = analyzeImprovisation(situation);
    auto min = rule(result,"boyko.melodic-minor.root");
    expect(min && min->source.name == "G melodic minor"
           && min->source.notes.back().role == MaterialNoteRole::passingTone
           && min->omittedChordTones.size() == 1,
           "written Gm7 retains b7 while optional melodic color marks major seventh passing");
    expect(result.strategies.front().source.notes.back().pitchClass==5,"foundation not rewritten");
    situation = withSelectedCenter(makeKey(-1,false),makeChord(1,{0,3,7,11}));
    result = analyzeImprovisation(situation);
    min = rule(result,"boyko.melodic-minor.root");
    expect(min && min->source.name == "G melodic minor","explicit Gm(maj7) gets minor melodic source");
    expect(normalizedChordSymbol(result.context.currentChord)=="Gm(maj7)","minor-major seventh symbol preserves written quality");
    expect(min->source.notes.back().spelling == "F#" && min->omittedChordTones.empty(),"major seventh is explicit chord tone");
    const auto relativeVi = withSelectedCenter(key, makeChord(3,{0,3,7,10}));
    expect(!rule(analyzeImprovisation(relativeVi),"boyko.melodic-minor.root"),
           "diatonic Am7 in C major does not suggest Am6 automatically");
    const auto explicitAm6 = withSelectedCenter(key, makeChord(3,{0,3,7,9}));
    expect(rule(analyzeImprovisation(explicitAm6),"boyko.melodic-minor.root"),
           "an explicitly written Am6 retains compatible melodic minor color");
    situation = withSelectedCenter(makeKey(-1,false),makeChord(1,{0,3,7,9}));
    situation.currentChord.degrees[9]=7;
    expect(!rule(analyzeImprovisation(situation),"boyko.melodic-minor.root"),"omission does not hide wrong explicit degree");
    // Levine distinguishes dominant half-whole from whole-half on dim7,
    // and whole-tone from the melodic-minor Lydian dominant application.
    snapshot = makeSnapshot(key,makeChord(2,{0,3,7,10}),makeChord(1,{0,1,4,7,10}),makeChord(0,{0,4,7,11}));
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    const auto* domDim = rule(result,"levine.dominant.half-whole");
    expect(domDim && domDim->source.name=="G half-whole diminished" && !domDim->tensionClassified,
           "explicit G7b9 receives unclassified half-whole source");
    expectPitches(domDim->source,{7,8,10,11,1,2,4,5});
    expect(domDim->source.notes[2].spelling=="A#" && domDim->source.notes[4].spelling=="C#",
           "dominant #9/#11 spelling is chord-relative");
    MaterialView dimView;
    bool projectedHalfWhole = false;
    const auto dimExplanation = buildExplanation(result);
    for (std::size_t i=0;i<dimExplanation.items.size();++i)
        if (dimExplanation.items[i].source.name=="G half-whole diminished")
        {
            dimView=buildMaterialView(result,dimExplanation,i);
            projectedHalfWhole=true;
        }
    expect(projectedHalfWhole && dimView.valid && dimView.targets.size()==4
           && dimView.current.size()==8 && dimView.current[2].spelling=="A#",
           "viewer receives eight named tones and the actual next-chord targets");
    snapshot.currentChord=makeChord(1,{0,4,7,10});
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    domDim = rule(result,"levine.dominant.half-whole");
    expect(domDim && !domDim->tensionClassified,
           "unqualified G7 offers half-whole as optional color without reclassifying the chord");
    snapshot.currentChord=makeChord(1,{0,4,8,10});
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    const auto* wt = rule(result,"levine.dominant.whole-tone");
    expect(wt && wt->source.name=="G whole-tone" && !wt->tensionClassified,
           "explicit G7#5 receives unclassified whole-tone source");
    expectPitches(wt->source,{7,9,11,1,3,5});
    expect(wt->source.notes[4].spelling=="D#" && wt->source.notes[3].spelling=="C#",
           "whole-tone spells #5 and #11 over G7");
    snapshot.currentChord=makeChord(1,{0,4,7,8,10});
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"levine.dominant.whole-tone"),
           "explicit natural fifth blocks whole-tone source");
    snapshot.currentChord=makeChord(1,{0,4,8,10});
    snapshot.nextChord=makeChord(0,{0,4,7,10});
    result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(rule(result,"levine.dominant.whole-tone")
           && result.dominantContext == DominantContext::toDominant,
           "augmented dominant chain offers whole-tone without inventing major tonic");
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
    result = analyzeImprovisation(situation);
    dim = rule(result,"boyko.diminished.whole-half");
    expect(dim && !dim->actualChord.hasTone(9),"dim triad offers whole-half without becoming dim7");
    situation.primaryInterpretationIndex=-1;
    result = analyzeImprovisation(situation);
    expect(result.context.primaryInterpretationIndex == -1
           && rule(result,"boyko.diminished.whole-half"),"diminished source does not select a hidden interpretation");
    situation = withSelectedCenter(key,makeChord(4,{0,3,6,10}));
    situation.interpretations[0].center.key.mode=KeyMode::custom;
    expect(analyzeImprovisation(situation).strategies.size()==1,"custom center unsupported");
    // Slash bass remains sounding; altered omits natural fifth, so cannot ignore it in bass.
    snapshot.currentChord=makeChord(1,{0,4,7,10});
    snapshot.currentChord.bass=2;
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"boyko.melodic-minor.bII"),"natural fifth slash bass prevents altered");
    snapshot.currentChord.bass=3; // A, the natural ninth over G.
    expect(!rule(analyzeImprovisation(analyzeHarmonicSituation(snapshot)),"levine.dominant.half-whole"),
           "slash bass outside the half-whole collection is not silently omitted");
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
