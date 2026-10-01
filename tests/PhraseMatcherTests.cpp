#include "core/analysis/PhraseMatcher.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace smartimproviser::harmony;
void check(bool value, const char* text) { if (!value) { std::cerr << text << '\n'; std::exit(1); } }
int wrap(int n) { return (n % 12 + 12) % 12; }
ChordContext chord(int root, std::initializer_list<int> tones)
{
    ChordContext c; c.available=c.defined=true; c.root=c.bass=root;
    for (int n:tones) c.intervals.values[n]=n==8 ? 13 : 0xFF;
    return c;
}
ImprovisationResult material(int root, std::initializer_list<int> tones, int next,
                            std::initializer_list<int> nextTones, int key=0,
                            bool hasNext=true, bool hasIi=false, bool half=false)
{
    TimelineHarmonicSnapshot s; s.positionAvailable=true; s.ppq=4;
    s.globalKey.available=s.globalKey.defined=true; s.globalKey.root=key;
    for(int n:{0,2,4,5,7,9,11}) s.globalKey.intervals.values[n]=0xFF;
    s.currentChord=chord(root,tones); s.nextChordAvailable=hasNext;
    s.nextChord=chord(next,nextTones);
    s.previousChordAvailable=hasIi;
    s.previousChord=half ? chord(root+1,{0,3,6,10}) : chord(root+1,{0,3,7,10});
    PatternTimelineWindow w; w.currentIndex=hasIi ? 1 : 0;
    w.chordCount=static_cast<std::uint8_t>((hasIi ? 2 : 1)+(hasNext ? 1 : 0));
    if(hasIi) w.chords[0]=s.previousChord;
    w.chords[w.currentIndex]=s.currentChord;
    if(hasNext) w.chords[w.currentIndex+1]=s.nextChord;
    return analyzeImprovisation(analyzeHarmonicSituation(s,w));
}
PhraseMatchRequest request(const ImprovisationResult& r)
{
    check(r.valid,"fixture must produce real analyzed material");
    return {{{r,0,4}},std::nullopt};
}
PhraseNote note(int slot, int root, int pc, double beat=0, PhraseNoteRole role=PhraseNoteRole::sourceTone, bool target=false)
{
    constexpr int base[]={0,2,4,5,7,9,11};
    const int interval=wrap(pc-root);
    for(int accidental:{0,-1,1,-2,2})
        for(int d=0;d<7;++d) if(wrap(base[d]+accidental)==interval)
            return {{slot,d+1,accidental},beat,0.5,target,role};
    std::abort();
}
const ImprovisationStrategy& strategy(const ImprovisationResult& r, const std::string& id)
{
    for(const auto& s:r.strategies) if(s.ruleId==id) return s;
    std::cerr << "missing fixture rule: " << id << '\n';std::exit(1);
}
void bind(Phrase& p,const ImprovisationStrategy& s,int slot=0)
{
    PhraseSlotRequirement b; b.chordIndex=slot; b.sourceRuleId=s.ruleId;
    b.sourceRuleVersion=s.ruleVersion; b.sourceRootOffset=wrap(s.source.rootPitchClass-s.actualChord.rootPitchClass);
    p.harmonicRequirements.push_back(b);
}
bool reason(const PhraseMatchResult& r,PhraseMatchReason why)
{
    for(const auto& d:r.diagnostics) if(d.reason==why) return true;
    return false;
}
int main()
{
    check(!requestedPhraseTension(0) && !requestedPhraseTension(99)
        && requestedPhraseTension(1)==TensionLevel::stable
        && requestedPhraseTension(2)==TensionLevel::color
        && requestedPhraseTension(3)==TensionLevel::outsideMaximum,"UI filter maps to independent phrase request");
    auto major=material(1,{0,4,7,10},0,{0,4,7,11}); // G7 -> Cmaj7
    auto query=request(major);
    Phrase anchors;
    anchors.notes={note(0,7,7,0,PhraseNoteRole::chordAnchor),note(0,7,11,1,PhraseNoteRole::guideTone)};
    auto r=assessPhrase(anchors,query);
    check(r.eligible() && r.harmonic==PhraseCompatibility::compatible && r.tension==PhraseTensionMatch::any,"unclassified phrase is available in All");
    query.requestedTension=TensionLevel::stable;
    r=assessPhrase(anchors,query);
    check(!r.eligible() && r.harmonic==PhraseCompatibility::compatible && r.tension==PhraseTensionMatch::unclassified,"default enum must not silently classify phrase T1");
    anchors.tensionClassified=true; anchors.tensionLevel=TensionLevel::stable;
    check(assessPhrase(anchors,query).eligible(),"explicit phrase label matches request");
    query.requestedTension=TensionLevel::color;
    r=assessPhrase(anchors,query);
    check(!r.eligible() && r.harmonic==PhraseCompatibility::compatible && r.tension==PhraseTensionMatch::differentLevel,"tension mismatch is separate from harmonic mismatch");
    check(r.notePitchClasses==std::vector<int>({7,11}) && anchors.notes[0].pitch.degree==1 && anchors.notes[1].beatOffset==1,"filter does not alter notes or rhythm");
    anchors.notes[0]=note(0,7,8,0,PhraseNoteRole::chordAnchor); anchors.tensionLevel=TensionLevel::color;
    r=assessPhrase(anchors,query);
    check(r.harmonic==PhraseCompatibility::incompatible && r.tension==PhraseTensionMatch::notEvaluated && !r.eligible(),"a matching tension cannot authorize a wrong chord anchor");

    const ImprovisationStrategy* mode=nullptr;
    for(const auto& s:major.strategies) if(s.source.kind==MaterialKind::scale && s.source.mode==DiatonicMode::mixolydian) { mode=&s;break; }
    check(mode!=nullptr,"major source exists");
    Phrase scale; bind(scale,*mode); scale.notes={note(0,7,9)}; // G's natural 9
    query.requestedTension.reset();
    check(assessPhrase(scale,query).eligible(),"compatible source color need not be an actual chord tone");
    scale.notes[0]=note(0,7,8);
    check(assessPhrase(scale,query).harmonic==PhraseCompatibility::incompatible,"non-source note rejected despite pitch role");
    scale.notes[0]=note(0,7,9); scale.harmonicRequirements[0].sourceRuleId="unavailable-rule";
    check(reason(assessPhrase(scale,query),PhraseMatchReason::sourceUnavailable),"catalog is authoritative");
    scale.harmonicRequirements.clear();
    check(assessPhrase(scale,query).harmonic==PhraseCompatibility::insufficientContext,"unbound source note does not guess a source");
    bind(scale,*mode); scale.harmonicRequirements[0].sourceRuleVersion=999;
    check(reason(assessPhrase(scale,query),PhraseMatchReason::sourceVersion),"unknown source version needs revalidation");
    scale.harmonicRequirements[0].sourceRuleVersion=mode->ruleVersion;
    scale.harmonicPattern=HarmonicPatternType::modalVamp;
    check(!assessPhrase(scale,query).eligible(),"required harmonic turn cannot be silently ignored");

    // Existing harmonic-minor family is shared by equivalent minor destinations.
    for(int key=0;key<12;++key) for(bool half:{false,true}) for(bool actualMinor:{false,true})
    {
        auto v=material(key+3,{0,4,7,8,10},key+2,actualMinor ? std::initializer_list<int>{0,3,7,10} : std::initializer_list<int>{0,4,7,10},key,true,true,half);
        auto q=request(v); const auto& hm=strategy(v,"project.harmonic-minor.contextual-V");
        Phrase p; bind(p,hm); p.harmonicRequirements[0].destinationQuality=ChordQuality::minor;
        p.notes={note(0,v.context.currentChord.rootPitchClass,hm.source.rootPitchClass)};
        auto result=assessPhrase(p,q);
        check(result.eligible() && result.contexts[0].destination.quality==ChordQuality::minor,"all transpositions and ii qualities retain minor source family");
        check(result.contexts[0].actualNextChord.quality==(actualMinor ? ChordQuality::minor : ChordQuality::dominant),"actual next remains distinct from assumed Dm");
        check(result.contexts[0].actualNextChord.quality==v.context.nextChord.quality,"matcher never rewrites factual harmony");
        p.harmonicRequirements[0].destinationQuality=ChordQuality::major;
        check(reason(assessPhrase(p,q),PhraseMatchReason::destinationMismatch),"major/minor requirements checked independently of note membership");
        p.harmonicRequirements[0].destinationQuality=ChordQuality::minor;
        p.harmonicRequirements[0].confirmedDestinationRequired=true;
        if(!actualMinor) check(reason(assessPhrase(p,q),PhraseMatchReason::destinationUnconfirmed),"hypothetical destination does not acquire confirmed status");
    }
    auto hypothetical=material(3,{0,4,7,8,10},2,{0,4,7,10},0,true,true); // A7b13 -> actual D7, expected Dm
    auto q=request(hypothetical);
    q.slots.push_back({material(2,{0,4,7,10},1,{0,4,7,10}),4,8});
    Phrase arrival; bind(arrival,strategy(hypothetical,"project.harmonic-minor.contextual-V"));
    arrival.harmonicRequirements[0].destinationQuality=ChordQuality::minor;
    arrival.notes={note(0,9,2),note(1,2,5,4,PhraseNoteRole::resolutionTarget,true)};
    check(assessPhrase(arrival,q).harmonic==PhraseCompatibility::incompatible,"imagined Dm third F cannot replace actual D7's F# landing");
    arrival.notes[1]=note(1,2,6,4,PhraseNoteRole::resolutionTarget,true);
    check(assessPhrase(arrival,q).eligible(),"actual D7 landing respected while source family keeps hypothetical Dm");

    auto unresolved=material(1,{0,4,7,10},0,{},0,false);
    Phrase needDestination; needDestination.notes={note(0,7,7,0,PhraseNoteRole::chordAnchor)};
    needDestination.harmonicRequirements.push_back({0,"",0,-1,ChordQuality::undefined,ChordQuality::major,false});
    check(reason(assessPhrase(needDestination,request(unresolved)),PhraseMatchReason::destinationMissing),"unresolved dominant cannot invent a target");
    auto secondary=material(3,{0,4,7,10},2,{0,3,7,10});
    auto sub=material(-5,{0,4,7,10},0,{0,4,7,11});
    check(secondary.secondaryDominant && sub.substituteDominant,"secondary/SubV fixtures use existing analysis");
    for(const auto& context:{secondary,sub})
    {
        Phrase p; const int root=context.context.currentChord.rootPitchClass;
        p.notes={note(0,root,wrap(root+4),0,PhraseNoteRole::guideTone)};
        check(assessPhrase(p,request(context)).eligible(),"secondary/SubV guides checked against actual written chord");
    }

    // Complete existing chromatic approach to Cmaj7's E: D# -> E.
    auto approachQuery=request(major);
    approachQuery.slots.push_back({material(0,{0,4,7,11},1,{0,4,7,10}),4,8});
    Phrase approach; approach.notes={note(0,7,3,3.5,PhraseNoteRole::passingApproach),note(1,0,4,4,PhraseNoteRole::resolutionTarget,true)};
    approach.approaches.push_back({0,"concept.chromatic-approach-below",{0,1}});
    check(assessPhrase(approach,approachQuery).eligible(),"approved approach accepts a non-source chromatic note with real landing");
    auto copy=approach; copy.approaches.clear();
    check(reason(assessPhrase(copy,approachQuery),PhraseMatchReason::missingApproach),"passing label alone cannot authorize chromatic note");
    copy=approach; copy.notes[1]=note(1,0,2,4,PhraseNoteRole::resolutionTarget,true);
    copy.tensionClassified=true; copy.tensionLevel=TensionLevel::outsideMaximum; approachQuery.requestedTension=TensionLevel::outsideMaximum;
    check(assessPhrase(copy,approachQuery).harmonic==PhraseCompatibility::incompatible,"T3 cannot authorize wrong approach landing");
    approachQuery.requestedTension.reset();
    copy=approach; copy.approaches[0].noteIndices={0};
    check(reason(assessPhrase(copy,approachQuery),PhraseMatchReason::invalidApproach),"incomplete approach rejected");
    copy=approach; copy.notes[0].harmonicRole=PhraseNoteRole::outsideTone; copy.approaches.clear();
    check(reason(assessPhrase(copy,approachQuery),PhraseMatchReason::unsupportedOutside),"outside policy remains explicitly unsupported");
    Phrase enclosure; enclosure.notes={note(0,7,5,3,PhraseNoteRole::passingApproach),note(0,7,3,3.5,PhraseNoteRole::passingApproach),note(1,0,4,4,PhraseNoteRole::resolutionTarget,true)};
    enclosure.approaches.push_back({0,"concept.chromatic-enclosure",{0,1,2}});
    check(assessPhrase(enclosure,approachQuery).eligible(),"approved complete enclosure accepted");
    std::swap(enclosure.notes[0].pitch,enclosure.notes[1].pitch);
    check(!assessPhrase(enclosure,approachQuery).eligible(),"reversed enclosure rejected");

    Phrase bad; bad.notes={note(0,7,7,0,PhraseNoteRole::chordAnchor)};
    auto missing=request(major); missing.slots[0].material.valid=false;
    check(assessPhrase(bad,missing).harmonic==PhraseCompatibility::insufficientContext,"unknown context is distinct from incompatible harmony");
    check(!assessPhrase(bad,PhraseMatchRequest{}).eligible(),"missing slot context rejected");
    bad.notes[0].pitch.degree=0; check(reason(assessPhrase(bad,query),PhraseMatchReason::invalidPitch),"invalid relative degree rejected");
    bad.notes[0]=note(0,7,7,3.8,PhraseNoteRole::chordAnchor);
    check(assessPhrase(bad,query).harmonic==PhraseCompatibility::insufficientContext,"sustain across unknown boundary needs explicit context");
    bad.notes[0].beatOffset=std::numeric_limits<double>::quiet_NaN();
    check(reason(assessPhrase(bad,query),PhraseMatchReason::invalidTiming),"NaN timing rejected");
    bad.notes[0]=note(9,7,7,0,PhraseNoteRole::chordAnchor);
    check(reason(assessPhrase(bad,query),PhraseMatchReason::missingContext),"missing referenced slot rejected");
    check(!assessPhrase(Phrase{},query).eligible(),"empty phrase cannot match");
    Phrase local; local.notes={note(0,7,7,0,PhraseNoteRole::chordAnchor)};
    PhraseSlotRequirement localRequirement; localRequirement.chordIndex=0;
    localRequirement.harmonicPattern=HarmonicPatternType::modalVamp;
    local.harmonicRequirements.push_back(localRequirement);
    check(reason(assessPhrase(local,request(major)),PhraseMatchReason::patternMismatch),"slot-local turn constraints are checked");
    Phrase currentApproach;
    currentApproach.notes={note(0,7,10,0,PhraseNoteRole::passingApproach),note(0,7,11,0.5,PhraseNoteRole::resolutionTarget,true)};
    currentApproach.approaches.push_back({0,"concept.chromatic-approach-below",{0,1}});
    check(assessPhrase(currentApproach,request(unresolved)).eligible(),"unresolved dominant can approach a real current-chord guide without inventing next");
    currentApproach.notes[1].beatOffset=0.25;
    check(reason(assessPhrase(currentApproach,request(unresolved)),PhraseMatchReason::invalidApproach),"overlapping approach notes rejected");
    currentApproach.notes[1].beatOffset=0.5;
    currentApproach.notes[1].harmonicRole=PhraseNoteRole::outsideTone;
    check(!assessPhrase(currentApproach,request(unresolved)).eligible(),"outside role cannot hide in an approved landing group");
    Phrase saved=approach; const auto pitches=assessPhrase(saved,approachQuery).notePitchClasses;
    approachQuery.requestedTension=TensionLevel::stable;
    const auto afterFilter=assessPhrase(saved,approachQuery);
    check(afterFilter.notePitchClasses==pitches && saved.notes[0].harmonicRole==PhraseNoteRole::passingApproach
        && saved.notes[0].beatOffset==3.5 && saved.notes[0].durationBeats==0.5
        && saved.notes[1].target && saved.approaches[0].noteIndices==std::vector<std::size_t>({0,1}),"tension changes preserve complete phrase data");
    Phrase invalidRole=anchors; invalidRole.notes[0].harmonicRole=static_cast<PhraseNoteRole>(99);
    check(reason(assessPhrase(invalidRole,request(major)),PhraseMatchReason::invalidPhrase),"invalid role metadata rejected");
    Phrase invalidTarget=approach; invalidTarget.approaches[0].noteIndices={0,99};
    check(reason(assessPhrase(invalidTarget,approachQuery),PhraseMatchReason::invalidApproach),"bad approach index rejected safely");
    std::cout << "PhraseMatcherTests: OK\n";
}
