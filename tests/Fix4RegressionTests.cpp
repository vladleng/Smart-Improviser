#include "core/analysis/DominantDestination.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/MaterialSelection.h"
#include "core/analysis/MaterialViewer.h"
#include "core/analysis/ManualTensionKey.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <tuple>
using namespace smartimproviser::harmony;
namespace {
void check(bool ok, const char* text) { if (!ok) { std::cerr << text << '\n'; std::exit(1); } }
ChordContext chord(int root, std::initializer_list<int> notes) {
    ChordContext c; c.available=c.defined=true; c.root=c.bass=root;
    for(int n:notes) c.intervals.values[n]=n==8 ? 13 : 0xFF;
    return c;
}
ImprovisationResult turn(int key, bool halfDim, bool minorNext, bool previous=true,
                         int added=-1, int bass=999, int nextOffset=2) {
    TimelineHarmonicSnapshot s; s.positionAvailable=true; s.ppq=4;
    s.globalKey.available=s.globalKey.defined=true; s.globalKey.root=key;
    for(int n:{0,2,4,5,7,9,11}) s.globalKey.intervals.values[n]=0xFF;
    s.previousChordAvailable=previous;
    s.previousChord=halfDim ? chord(key+4,{0,3,6,10}) : chord(key+4,{0,3,7,10});
    s.currentChord=chord(key+3,{0,4,7,8,10});
    if(added>=0) s.currentChord.intervals.values[added]=0xFF;
    if(bass!=999) s.currentChord.bass=bass;
    s.nextChordAvailable=true;
    s.nextChord=minorNext ? chord(key+nextOffset,{0,3,7,10}) : chord(key+nextOffset,{0,4,7,10});
    PatternTimelineWindow w; w.currentIndex=previous ? 1 : 0; w.chordCount=previous ? 3 : 2;
    if(previous) w.chords[0]=s.previousChord;
    w.chords[w.currentIndex]=s.currentChord; w.chords[w.currentIndex+1]=s.nextChord;
    for(int i=0;i<w.chordCount;++i) w.chords[i].startPpq=i*4;
    return analyzeImprovisation(analyzeHarmonicSituation(s,w));
}
const ImprovisationStrategy* rule(const ImprovisationResult& r,const char* id) {
    for(const auto& s:r.strategies) if(s.ruleId==id) return &s;
    return nullptr;
}
using Signature=std::vector<std::tuple<std::string,int,std::vector<int>>>;
Signature catalog(const ImprovisationResult& r) {
    Signature result;
    for(const auto& s:r.strategies) if(s.source.kind==MaterialKind::scale || s.source.kind==MaterialKind::arpeggio) {
        std::vector<int> notes; for(const auto& n:s.source.notes) notes.push_back(n.pitchClass);
        result.emplace_back(s.ruleId,s.source.rootPitchClass,notes);
    }
    std::sort(result.begin(),result.end()); return result;
}
}
int main() {
    for(int key=-5;key<=6;++key) {
        auto hypothetical=turn(key,false,false);
        auto actual=turn(key,true,true);
        check(catalog(hypothetical)==catalog(actual),"same dominant and minor destination must produce the same source catalog");
        check(catalog(actual).size()==4,"b13 minor palette contains four distinct sources");
        check(catalog(hypothetical)==catalog(turn(key,true,false)),"half-diminished ii cannot change hypothetical minor palette");
        check(catalog(actual)==catalog(turn(key,false,true)),"minor ii cannot change confirmed minor palette");
        check(catalog(actual)==catalog(turn(key,false,true,false)),"confirmed minor palette works without preceding ii");
        check(catalog(hypothetical)==catalog(turn(key,false,false,false)),"root motion and context can supply presumed minor destination without ii");
        check(!rule(actual,"levine.harmonic-minor.minor-V-fragment"),"obsolete fragment must never be generated");
        const auto* h=rule(actual,"project.harmonic-minor.contextual-V");
        const auto* a=rule(actual,"project.minor-V.bII-dim7-arpeggio");
        check(h && h->source.notes.size()==7 && h->source.rootFifths==key+2,"full harmonic minor comes from minor destination");
        check(a && a->source.kind==MaterialKind::arpeggio && a->source.notes.size()==4,"bII diminished arpeggio is four notes, not an octatonic scale");
        check(a->source.rootFifths==key-2 && a->thinkingStructure.quality==ChordQuality::diminished,"arpeggio root and thinking chord are bII dim7");
        const int relative[]={1,4,7,10};
        for(int i=0;i<4;++i) check(a->source.chordRelativeNotes[i].semitonesFromRoot==relative[i],"arpeggio gives b9, third, fifth and b7");
        check(!a->tensionClassified && !h->tensionClassified,"new sources receive no automatic T1 classification");
        const auto e=buildExplanation(actual), hypotheticalItems=buildExplanation(hypothetical); bool projected=false;
        for(const auto& item:e.items) for(const auto& other:hypotheticalItems.items)
            if(item.sourceRuleId==other.sourceRuleId)
                check(manualTensionKey(actual,item)==manualTensionKey(hypothetical,other),
                      "manual source labels follow the same minor destination in both cases");
        for(std::size_t i=0;i<e.items.size();++i) if(e.items[i].sourceRuleId==a->ruleId) {
            check(!compactMaterialHidden(e,i),"arpeggio is selectable in compact list");
            const auto v=buildMaterialView(actual,e,i); int sourceNotes=0; bool flat13Anchor=false;
            for(const auto& n:v.current) {
                if(n.roles&sourceRole) {++sourceNotes;check(n.pitchClass!=(actual.context.currentChord.rootPitchClass+9)%12,"arpeggio does not introduce natural 13");}
                if(n.pitchClass==(actual.context.currentChord.rootPitchClass+8)%12)
                    flat13Anchor=(n.roles&chordRole) && !(n.roles&sourceRole);
            }
            check(sourceNotes==4 && flat13Anchor,"viewer preserves written b13 outside four-note arpeggio"); projected=true;
        }
        check(projected,"arpeggio reaches explanation and viewer");
        const auto d=dominantDestination(hypothetical.context);
        check(d.minor() && !d.confirmed,"D7 continuation supports presumed Dm without claiming played minor tonic");
        check(hypothetical.context.nextChord.quality==ChordQuality::dominant
              && hypothetical.context.resolution.targetQuality!=ChordQuality::minor,"actual changed-quality chord remains factual, not a confirmed minor tonic");
        check(dominantDestination(actual.context).minor() && dominantDestination(actual.context).confirmed,"actual minor resolution stays confirmed");
        check(hypothetical.context.localKey.status!=KeyCenterStatus::established
              && hypothetical.context.localKey.status!=KeyCenterStatus::tonicized,"hypothetical destination cannot establish D7 as a local tonic");
        check(!rule(turn(key,true,true,true,2),"project.minor-V.bII-dim7-arpeggio"),"written natural ninth blocks b9 arpeggio");
        check(!rule(turn(key,true,true,true,-1,key+5),"project.minor-V.bII-dim7-arpeggio"),"incompatible slash bass is not silently omitted");
        check(!dominantDestination(turn(key,false,false,true,-1,999,1).context).minor(),"non-destination root motion cannot create minor hypothesis");
    }
    std::cout << "Fix4 regression tests passed\n";
}
