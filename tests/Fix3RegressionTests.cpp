#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/MajorIiVPalette.h"
#include "core/analysis/MaterialSelection.h"
#include "core/analysis/MaterialViewer.h"
#include "core/analysis/TensionEngine.h"
#include <cstdlib>
#include <iostream>
using namespace smartimproviser::harmony;
namespace {
void check(bool ok, const char* text) { if (!ok) { std::cerr << text << '\n'; std::exit(1); } }
ChordContext chord(int root, std::initializer_list<int> notes) {
    ChordContext c; c.available=c.defined=true; c.root=c.bass=root;
    for (int n : notes) c.intervals.values[n]=n==8 ? 13 : 0xFF;
    return c;
}
ImprovisationResult turn(ChordContext previous, ChordContext current, ChordContext next, int key, ChordContext future = {}) {
    TimelineHarmonicSnapshot s; s.positionAvailable=true; s.ppq=4;
    s.previousChordAvailable=previous.available; s.previousChord=previous;
    s.currentChord=current; s.nextChordAvailable=true; s.nextChord=next;
    s.globalKey.available=s.globalKey.defined=true; s.globalKey.root=key;
    for (int n : {0,2,4,5,7,9,11}) s.globalKey.intervals.values[n]=0xFF;
    PatternTimelineWindow w; w.chordCount=3; w.currentIndex=1;
    w.chords[0]=previous; w.chords[1]=current; w.chords[2]=next;
    if (future.available) { w.chordCount=4; w.chords[3]=future; }
    for (int i=0;i<w.chordCount;++i) w.chords[i].startPpq=i*4;
    auto r=analyzeImprovisation(analyzeHarmonicSituation(s,w));
    const auto profile = analyzeStableTension(r);
    for (const auto& c : profile.bands[0].alternatives) r.strategies.push_back(c.strategy);
    return r;
}
ImprovisationResult resolvedF(int key) {
    TimelineHarmonicSnapshot s; s.positionAvailable=true; s.ppq=8;
    s.previousChordAvailable=true; s.previousChord=chord(key,{0,4,7,10});
    s.currentChord=chord(key-1,{0,4,7,11});
    s.nextChordAvailable=true; s.nextChord=chord(key-1,{0,3,7,10});
    s.globalKey.available=s.globalKey.defined=true; s.globalKey.root=key;
    for(int n:{0,2,4,5,7,9,11}) s.globalKey.intervals.values[n]=0xFF;
    PatternTimelineWindow w; w.chordCount=4; w.currentIndex=2;
    w.chords[0]=chord(key+1,{0,3,7,10}); w.chords[1]=s.previousChord;
    w.chords[2]=s.currentChord; w.chords[3]=s.nextChord;
    for(int i=0;i<4;++i) w.chords[i].startPpq=i*4;
    auto r=analyzeImprovisation(analyzeHarmonicSituation(s,w));
    const auto profile=analyzeStableTension(r);
    for(const auto& c:profile.bands[0].alternatives) r.strategies.push_back(c.strategy);
    return r;
}
const ImprovisationStrategy* rule(const ImprovisationResult& r, const char* id) {
    for (const auto& s : r.strategies) if (s.ruleId==id) return &s;
    return nullptr;
}
}
int main() {
    for (int key=-5;key<=6;++key) {
        auto dim=turn(chord(key+2,{0,4,7,10}),chord(key-4,{0,3,6}),chord(key+1,{0,3,7,10}),key);
        auto scale=rule(dim,"boyko.diminished.whole-half");
        check(scale && scale->source.notes.size()==8,"diminished triad offers whole-half in every transposition");
        check(!scale->actualChord.hasTone(9),"scale cannot turn the written triad into dim7");
        auto major=turn(chord(key+1,{0,4,7,10}),chord(key,{0,4,7,11}),chord(key+2,{0,3,7,10}),key);
        auto nine=rule(major,"project.t1.diatonic-nine");
        check(nine,"major chord offers natural-nine subset");
        bool characteristic=false;
        for (const auto& n:nine->characteristicNotes) if(n.semitonesFromRoot==2 && n.characteristic) characteristic=true;
        check(characteristic,"added natural nine is characteristic");
        auto e=buildExplanation(major); bool visible=false;
        for(std::size_t i=0;i<e.items.size();++i) if(e.items[i].sourceRuleId=="project.t1.diatonic-nine")
            for(const auto& n:buildMaterialView(major,e,i).current)
                if(n.pitchClass==(major.context.currentChord.rootPitchClass+2)%12 && (n.roles&characteristicRole)) visible=true;
        check(visible,"characteristic nine reaches the viewer layer");
        auto local=resolvedF(key); auto localItems=buildExplanation(local);
        const int localBase=playingBaseIndex(local,localItems);
        check(localBase>=0 && localItems.items[localBase].source.mode==DiatonicMode::ionian,
              "confirmed F turn supplies Ionian foundation instead of global Lydian");
        check(rule(local,"project.t1.diatonic-nine"),"confirmed local Fmaj7 offers its ninth");
        auto duplicate=turn(chord(key+2,{0,4,7,10}),chord(key+2,{0,3,7,10}),chord(key+1,{0,4,7,10}),key,chord(key+3,{0,3,7,10}));
        auto bases=buildExplanation(duplicate); int selected=playingBaseIndex(duplicate,bases);
        check(selected>=0 && bases.items[selected].missingTonicApplication,"incomplete-turn foundation wins over duplicate global mode");
        int count=0; for(const auto& item:bases.items) if(isDiatonicFoundation(item)) ++count;
        check(count>=2,"diagnostic alternatives survive compact selection");
        auto minor=turn(chord(key+4,{0,3,7,10}),chord(key+3,{0,4,7,8,10}),chord(key+2,{0,4,7,10}),key);
        check(hasContextualMinorIiVTarget(minor.context),"global minor destination survives changed-quality D7 continuation");
        const auto* harmonic=rule(minor,"project.harmonic-minor.contextual-V");
        check(harmonic && harmonic->source.notes.size()==7,"full destination harmonic minor offered");
        check(harmonic->source.rootPitchClass==circleOfFifthsToPitchClass(key+2),"harmonic-minor root is the contextual destination");
        const int intervals[]={0,2,3,5,7,8,11};
        for(int i=0;i<7;++i) check(harmonic->source.notes[i].semitonesFromRoot==intervals[i],"harmonic minor has minor sixth and raised seventh");
        check(harmonic->nextChord.quality==ChordQuality::dominant && harmonic->nextChord.rootFifths==key+2,"actual D7 retained as next chord");
        check(!harmonic->resolution.confirmed || harmonic->resolution.targetChord.quality!=ChordQuality::minor,"hypothetical Dm is not a confirmed resolution");
        auto altered=rule(minor,"boyko.melodic-minor.bII"), rare=rule(minor,"project.melodic-minor.minor-V-b13");
        check(altered && rare && harmonic->priority>altered->priority && altered->priority>rare->priority,"harmonic minor precedes altered and rare melodic minor");
        check(rare->source.notes[5].semitonesFromRoot==9,"rare melodic minor retains natural sixth, distinct from harmonic minor");
        auto playedMajor=turn(chord(key+4,{0,3,7,10}),chord(key+3,{0,4,7,8,10}),chord(key+2,{0,4,7,11}),key);
        check(!hasContextualMinorIiVTarget(playedMajor.context),"played major tonic overrides hypothetical contextual minor");
        auto wrongKey=turn(chord(key+4,{0,3,7,10}),chord(key+3,{0,4,7,8,10}),chord(key+2,{0,4,7,10}),key+2);
        check(!hasContextualMinorIiVTarget(wrongKey.context) && !rule(wrongKey,"project.harmonic-minor.contextual-V"),"b13 alone does not invent minor destination in D major");
        auto isolated=turn(chord(key,{0,4,7,11}),chord(key+3,{0,4,7,8,10}),chord(key+2,{0,4,7,10}),key);
        check(!rule(isolated,"project.harmonic-minor.contextual-V"),"isolated b13 has no contextual ii-V hypothesis");
        auto naturalNine=turn(chord(key+4,{0,3,7,10}),chord(key+3,{0,2,4,7,8,10}),chord(key+2,{0,4,7,10}),key);
        check(!rule(naturalNine,"project.harmonic-minor.contextual-V"),"written natural nine blocks harmonic minor collection");
    }
    auto slashChord=chord(0,{0,4,7,11}); slashChord.bass=2;
    auto slash=turn(chord(1,{0,4,7,10}),slashChord,chord(1,{0,4,7,10}),0);
    const auto* slashNine=rule(slash,"project.t1.diatonic-nine");
    bool bassPreserved=false;
    if(slashNine) for(const auto& n:slashNine->source.notes)
        if(n.pitchClass==2 && n.role==MaterialNoteRole::bassTone) bassPreserved=true;
    check(bassPreserved,"a written ninth in the slash bass remains a bass anchor");
    TimelineHarmonicSnapshot noKey; noKey.positionAvailable=true;
    noKey.globalKey.available=noKey.globalKey.defined=true;
    for(int n:{0,1,4,7}) noKey.globalKey.intervals.values[n]=0xFF;
    noKey.currentChord=chord(-4,{0,3,6});
    auto standalone=analyzeImprovisation(analyzeHarmonicSituation(noKey));
    check(rule(standalone,"boyko.diminished.whole-half"),"diminished triad also works with a custom key without a tonal interpretation");
    std::cout << "Fix3 regression tests passed\n";
}
