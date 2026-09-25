#include "core/analysis/HarmonicConcepts.h"
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
const HarmonicConcept* conceptOf(const ImprovisationResult& result, HarmonicConceptKind kind, const std::string& source = {})
{
    for (const auto& idea : result.concepts)
        if (idea.kind == kind && (source.empty() || idea.sourceRuleId == source)) return &idea;
    return nullptr;
}
int main()
{
    const auto key = makeKey(0,false);
    auto snapshot = makeSnapshot(key,makeChord(2,{0,3,7,10}),makeChord(1,{0,4,7,10}),makeChord(0,{0,4,7,11}));
    auto result = analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    auto anchors = conceptOf(result,HarmonicConceptKind::chordAnchors);
    auto guide = conceptOf(result,HarmonicConceptKind::guideTargeting);
    expect(anchors && anchors->material.size()==4,"explicit anchors form a concrete idea");
    expect(guide && guide->movesAreConfirmed && guide->moves.size()==2,"confirmed guide-tone resolution");
    expect(guide->moves[0].fromPitchClass==11 && guide->moves[0].toPitchClass==0
        && guide->moves[1].fromPitchClass==5 && guide->moves[1].toPitchClass==4,"B to C and F to E");
    auto colors=conceptOf(result,HarmonicConceptKind::diatonicExtensions);
    expect(colors && colors->material.size()==3,"Mixo colors 9 11 13");
    expect(colors->material[0].pitchClass==9 && colors->material[0].degree==9,"G7 ninth A");
    expect(colors->material[1].pitchClass==0 && colors->material[1].role==MaterialNoteRole::passingTone,"natural fourth C marked passing against B");
    auto skeleton=conceptOf(result,HarmonicConceptKind::thinkingArpeggio,"boyko.melodic-minor.V");
    expect(skeleton && skeleton->material.size()==4 && skeleton->title=="Think Dm6 over G7","m6 skeleton from approved source");
    const int dm6[] = {2,5,9,11};
    for (int i=0;i<4;++i) expect(skeleton->material[static_cast<std::size_t>(i)].pitchClass==dm6[i],"Dm6 skeleton pitches");
    auto alt=conceptOf(result,HarmonicConceptKind::thinkingArpeggio,"boyko.melodic-minor.bII");
    expect(alt && alt->material.size()==4 && alt->material[1].spelling=="B","Abm6 over G7 shows real third B");
    auto approach=conceptOf(result,HarmonicConceptKind::chromaticApproach);
    auto enclosure=conceptOf(result,HarmonicConceptKind::enclosure);
    expect(approach && approach->target.pitchClass==4 && approach->targetScope==ConceptTargetScope::nextChord,"approach targets actual E of Cmaj7");
    expect(approach->approachShape.size()==2 && approach->approachShape[0].semitonesFromTarget==-1
        && !approach->approachShape[0].belongsToCurrentChord,"D# approach to E is non-chord over G7");
    expect(enclosure && enclosure->approachShape.size()==3 && enclosure->approachShape[0].semitonesFromTarget==1
        && enclosure->approachShape[0].belongsToCurrentChord,"upper F belongs to G7, lower D# does not");
    expect(enclosure->approachShape.back().isTarget,"last step identifies target, not preparation");
    const auto text=harmonicConceptsText(result);
    expect(text.find("Target: E (3) in Cmaj7 [next chord]")!=std::string::npos,"diagnostics identify exact target and scope");
    expect(text.find("C (11) [passing]")!=std::string::npos,"passing condition visible");
    snapshot.nextChord=makeChord(0,{0,3,7});
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    approach=conceptOf(result,HarmonicConceptKind::chromaticApproach);
    expect(approach && approach->target.pitchClass==3 && approach->target.spelling=="Eb","minor target changes approach landing");
    expect(approach->approachShape[0].belongsToCurrentChord,"D below Eb belongs to real G7");
    expect(!conceptOf(result,HarmonicConceptKind::diatonicExtensions),"no invented diatonic colors for unavailable minor dominant source");
    snapshot.currentChord=makeChord(7,{0,4,7,10}); // Studio Pro canonical C# means confirmed Db SubV.
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    skeleton=conceptOf(result,HarmonicConceptKind::thinkingArpeggio,"project.subv.melodic-minor.V");
    expect(skeleton && skeleton->title=="Think Abm6 over Db7","fix2 functional spelling inherited");
    expect(harmonicConceptsText(result).find("Cb -> C")!=std::string::npos,"SubV guide spelling follows real functional chord");
    expect(result.context.currentChord.rootFifths==7,"raw ARA spelling not overwritten");
    // No invented third for sus, no invented seventh for triads.
    snapshot.currentChord=makeChord(1,{0,5,7,10});
    snapshot.nextChord=makeChord(0,{0,4,7});
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    guide=conceptOf(result,HarmonicConceptKind::guideTargeting);
    expect(guide && guide->material.size()==1 && guide->material[0].pitchClass==5,"sus has b7 only");
    snapshot.currentChord=makeChord(0,{0,4,7});
    snapshot.nextChordAvailable=false;
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    guide=conceptOf(result,HarmonicConceptKind::guideTargeting);
    approach=conceptOf(result,HarmonicConceptKind::chromaticApproach);
    expect(guide && guide->material.size()==1 && guide->moves.empty(),"triad only has third guide and no invented target");
    expect(approach && approach->targetScope==ConceptTargetScope::currentChord && approach->target.pitchClass==4,"without next target actual current third");
    snapshot.currentChord=makeChord(0,{0,7});
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    expect(!conceptOf(result,HarmonicConceptKind::guideTargeting),"power chord has no third or seventh");
    expect(conceptOf(result,HarmonicConceptKind::chromaticApproach)->target.pitchClass==0,"root fallback for power chord");
    auto situation=withSelectedCenter(key,makeChord(2,{0,3,6,10}));
    situation.primaryInterpretationIndex=-1;
    result=analyzeImprovisation(situation);
    bool hasDependentConcept = false;
    for (const auto& idea:result.concepts)
    {
        if (idea.interpretationIndependent)
        {
            expect(idea.interpretationIndex==-1,"interpretation-independent concepts keep index -1");
        }
        else
        {
            hasDependentConcept = true;
            expect(idea.interpretationIndex==0,"unresolved primary retains explicit interpretation provenance");
        }
    }
    expect(hasDependentConcept,"unresolved primary retains attributed source concepts without hidden selection");
    situation=withSelectedCenter(makeKey(7,false),makeChord(7,{0,4,7,11}));
    result=analyzeImprovisation(situation);
    anchors=conceptOf(result,HarmonicConceptKind::chordAnchors);
    expect(anchors->material[1].spelling=="E#" && anchors->material[3].spelling=="B#","concept anchors use degree spelling");
    situation=withSelectedCenter(makeKey(-6,false),makeChord(-6,{0,4,7,11}));
    result=analyzeImprovisation(situation);
    colors=conceptOf(result,HarmonicConceptKind::diatonicExtensions);
    expect(colors && colors->material[1].spelling=="Cb","Gb major fourth spelled Cb");
    // Suggestions are not converted into harmonic evidence for ordinary ii -> V.
    snapshot=makeSnapshot(key,makeChord(0,{0,4,7,11}),makeChord(2,{0,3,7,10}),makeChord(1,{0,4,7,10}));
    result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    guide=conceptOf(result,HarmonicConceptKind::guideTargeting);
    expect(guide && !guide->movesAreConfirmed && !guide->moves.empty(),"optional ii V connections remain optional");
    for(int fifths=-5;fifths<=6;++fifths)
    {
        snapshot=makeCurrentNextSnapshot(makeKey(fifths,false),makeChord(fifths+1,{0,4,7,10}),makeChord(fifths,{0,4,7,11}));
        result=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
        approach=conceptOf(result,HarmonicConceptKind::chromaticApproach);
        expect(approach && approach->target.pitchClass==(circleOfFifthsToPitchClass(fifths)+4)%12,"actual third target in twelve keys");
        for(const auto& idea:result.concepts)
        {
            if(idea.target.pitchClass>=0) expect(idea.targetChord.hasTone(idea.target.semitonesFromRoot),"every approach target exists in real target chord");
            for(const auto& step:idea.approachShape)
                expect(step.belongsToCurrentChord==idea.actualChord.hasTone(step.semitonesFromCurrentRoot),"preparation evaluated against actual current chord");
        }
        const auto again=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
        expect(harmonicConceptsText(result)==harmonicConceptsText(again),"deterministic concepts and diagnostics");
        const auto count=result.concepts.size();
        addHarmonicConcepts(result);
        expect(result.concepts.size()==count,"rebuilding concepts does not duplicate");
        expect(result.context.globalKey.key.rootFifths==fifths,"source concepts never change global key");
    }
    result=analyzeImprovisation({});
    expect(result.concepts.empty() && harmonicConceptsText(result).empty(),"invalid input clears all concepts");
    std::cout << "Harmonic concept tests passed\n";
}
