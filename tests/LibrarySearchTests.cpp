#include "core/analysis/LibrarySearch.h"
#include "core/analysis/LibraryArchive.h"
#include "context/LibrarySearchContext.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
using namespace smartimproviser::harmony;
void check(bool value,const char* why){if(!value){std::cerr<<why<<'\n';std::exit(1);}}
ChordContext chord(int root,std::initializer_list<int> tones) {
    ChordContext c;c.available=c.defined=true;c.root=c.bass=root;
    for(int n:tones)c.intervals.values[n]=0xff;return c;
}
ImprovisationResult material() {
    TimelineHarmonicSnapshot s;s.positionAvailable=true;s.ppq=4;
    s.currentChord=chord(1,{0,4,7,10});s.nextChordAvailable=s.previousChordAvailable=true;
    s.previousChord=chord(2,{0,3,7,10});s.nextChord=chord(0,{0,4,7,11});
    s.globalKey.available=s.globalKey.defined=true;
    for(int n:{0,2,4,5,7,9,11})s.globalKey.intervals.values[n]=0xff;
    PatternTimelineWindow w;w.currentIndex=1;w.chordCount=3;w.chords[0]=s.previousChord;w.chords[1]=s.currentChord;w.chords[2]=s.nextChord;
    return analyzeImprovisation(analyzeHarmonicSituation(s,w));
}
LibraryRecord record(const std::string& id,std::optional<TensionLevel> tension={}) {
    LibraryRecord r;r.id=id;r.revision=1;r.name="Проверка поиска: G7";r.tags={"search-check"};
    r.source.permission=ContentPermission::ownWork;r.source.author="Moon River Studio";
    r.source.source="Original software regression fixture";r.source.license="CC0";
    r.source.permissionEvidence="Original three-note chord-anchor test sequence.";
    r.explanation="Проверочные данные, не музыкальная рекомендация. Метка T задана для проверки фильтра.";
    Phrase p;p.notes={{{0,1,0},0,0.5,false,PhraseNoteRole::chordAnchor},
        {{0,3,0},0.5,0.5,false,PhraseNoteRole::chordAnchor},
        {{0,5,0},1,0.5,false,PhraseNoteRole::chordAnchor}};
    for(auto& n:p.notes)n.octaveOffset=0;
    p.registerReferences={{0,55}};p.role=PhraseRole::statement;
    p.conceptRuleIds={"test.chord-anchors"};
    p.harmonicRequirements.push_back({0,"",0,-1,ChordQuality::dominant});
    if(tension){p.tensionClassified=true;p.tensionLevel=*tension;}
    r.content=p;return r;
}
int main(int argc,char** argv) {
    auto a=record("user:search-check-all");a.name+=" · без метки";
    auto b=record("user:search-check-T1",TensionLevel::stable);b.name+=" · T1";
    auto c=record("user:search-check-T2",TensionLevel::color);c.name+=" · T2";
    auto wrong=record("user:search-check-minor",TensionLevel::stable);wrong.name="Проверка поиска: требуется минорный аккорд";
    std::get<Phrase>(wrong.content).harmonicRequirements[0].chordQuality=ChordQuality::minor;
    auto draft=record("user:search-check-draft");draft.name="Проверка поиска: текстовый набросок";draft.content=Idea{};
    std::get<Idea>(draft.content).text="Это Idea без подтверждённой готовности.";
    std::vector<LibraryRecord> records{wrong,draft,c,a,b};
    LibrarySearchQuery q;q.match.slots={{material(),0,4}};
    check(q.match.slots[0].material.valid,"real context required");
    auto result=searchLibrary(records,q);
    check(result.eligibleCount==3 && result.entries.front().eligible(),"All admits unclassified ready phrases");
    q.match.requestedTension=TensionLevel::stable;
    result=searchLibrary(records,q);
    check(result.eligibleCount==1 && result.entries.front().record.id==b.id,"T1 rejects T2/unclassified/harmonic mismatch");
    auto find=[&](const std::string& id)->const LibrarySearchEntry& {
        for(const auto& e:result.entries)if(e.record.id==id)return e;std::abort();
    };
    check(find(a.id).state==LibrarySearchState::tensionUnknown,"unknown is distinct from mismatch");
    check(find(c.id).state==LibrarySearchState::tensionMismatch,"different level is explicit");
    check(find(wrong.id).state==LibrarySearchState::harmonicMismatch && find(wrong.id).assessment.tension==PhraseTensionMatch::notEvaluated,"harmonic gate runs first");
    check(find(draft.id).state==LibrarySearchState::draft,"Idea never promoted");
    q.curve=TensionCurve{{{20,21.5,TensionLevel::color}}};q.placementStartBeat=20;
    result=searchLibrary(records,q);check(result.eligibleCount==1 && result.entries.front().record.id==c.id,"curve replaces scalar and uses explicit placement");
    q.curve.reset();q.match.requestedTension.reset();q.text="текстовый";result=searchLibrary(records,q);
    check(result.eligibleCount==0 && find(draft.id).state==LibrarySearchState::draft,"text filter can find drafts without eligibility");
    q.text.clear();q.tag="absent";result=searchLibrary(records,q);check(result.eligibleCount==0,"tag filter");
    q.tag.clear();q.conceptRuleId="test.chord-anchors";q.role=PhraseRole::statement;
    result=searchLibrary(records,q);check(result.eligibleCount==3,"conceptRuleId/role metadata");
    q.conceptRuleId.clear();q.role.reset();q.function=HarmonicFunction::dominant;
    result=searchLibrary(records,q);check(result.eligibleCount==3,"context function");
    
    q.function.reset();
    auto patterned=a;std::get<Phrase>(patterned.content).harmonicPattern=HarmonicPatternType::majorIiVI;
    q.pattern=HarmonicPatternType::majorIiVI;q.patternPosition=1;
    result=searchLibrary({patterned},q);check(result.eligibleCount==1,"recognized pattern and dominant position");
    q.patternPosition=0;check(searchLibrary({patterned},q).eligibleCount==0,"wrong pattern position");
    q.pattern.reset();q.patternPosition.reset();
    auto ambiguous=q;
    ambiguous.match.slots[0].material.context.harmonic.effectiveFunction=HarmonicFunction::tonic;
    ambiguous.match.slots[0].material.context.localHarmonic.valid=false;
    auto& alternative=ambiguous.match.slots[0].material.context.interpretations[0];
    alternative.valid=alternative.harmonic.valid=true;alternative.harmonic.effectiveFunction=HarmonicFunction::dominant;
    ambiguous.match.slots[0].material.context.interpretationCount=1;ambiguous.function=HarmonicFunction::dominant;
    check(searchLibrary({a},ambiguous).eligibleCount==1
        && ambiguous.match.slots[0].material.context.harmonic.effectiveFunction==HarmonicFunction::tonic,"alternative function retained without hidden winner");
    q.curve=TensionCurve{{{0,1,TensionLevel::stable},{0.5,1.5,TensionLevel::color}}};
    check(searchLibrary({b},q).entries[0].state==LibrarySearchState::invalidTension,"overlapping curve rejected");
    q.curve.reset();q.match.slots.clear();result=searchLibrary(records,q);
    check(result.eligibleCount==0 && find(a.id).state==LibrarySearchState::insufficientContext,"no context yields no winner");
    q.match.slots={{material(),0,4}};
    auto stale=a;auto& p=std::get<Phrase>(stale.content);p.notes[0].harmonicRole=PhraseNoteRole::sourceTone;
    const auto& source=q.match.slots[0].material.strategies.front();
    p.harmonicRequirements[0].sourceRuleId=source.ruleId;p.harmonicRequirements[0].sourceRuleVersion=999;
    result=searchLibrary({stale},q);check(result.entries[0].state==LibrarySearchState::insufficientContext,"obsolete source version requires revalidation");
    auto invalid=a;std::get<Phrase>(invalid.content).notes[0].durationBeats=0;
    check(searchLibrary({invalid},q).entries[0].state==LibrarySearchState::invalidRecord,"invalid data refused");
    auto reordered=records;std::reverse(reordered.begin(),reordered.end());
    auto one=searchLibrary(records,q),two=searchLibrary(reordered,q);
    for(std::size_t i=0;i<one.entries.size();++i)check(one.entries[i].record.id==two.entries[i].record.id,"deterministic order");
    auto before=encodeLibraryArchive({"test",1,{records,{a.id,b.id,c.id,wrong.id,draft.id}}});
    (void)searchLibrary(records,q);
    auto after=encodeLibraryArchive({"test",1,{records,{a.id,b.id,c.id,wrong.id,draft.id}}});
    check(before.succeeded() && before.bytes==after.bytes,"search never mutates stored musical/metadata content");
    SharedHarmonicContextSnapshot shared;shared.connected=shared.transportAvailable=shared.sheetChordsAvailable=true;
    shared.transportPpq=1;shared.sheetChordStoredCount=3;
    for(int i=0;i<3;++i){shared.sheetChords[i].position=i*4;shared.sheetChords[i].root=shared.sheetChords[i].bass=i?0:1;
        for(int n:{0,4,7,10})shared.sheetChords[i].intervals[n]=0xff;}
    shared.keySignaturesAvailable=true;shared.keySignatureStoredCount=1;
    for(int n:{0,2,4,5,7,9,11})shared.keySignatures[0].intervals[n]=0xff;
    auto mapped=makeLibrarySearchQuery(shared,2);
    check(mapped.match.slots.size()==2 && mapped.match.slots[0].startBeat==0 && mapped.match.slots[0].endBeat==3
        && mapped.match.slots[1].startBeat==3 && mapped.placementStartBeat==1,"bounded cursor-relative known chord ranges");
    shared.transportPpq=8;check(makeLibrarySearchQuery(shared,0).match.slots.empty(),"unknown last boundary not fabricated");
    shared.transportAvailable=false;check(makeLibrarySearchQuery(shared,0).match.slots.empty(),"unavailable cursor");
    if(argc==2){
        check(before.succeeded(),"portable fixture");std::ofstream file(argv[1],std::ios::binary);
        file.write(reinterpret_cast<const char*>(before.bytes.data()),static_cast<std::streamsize>(before.bytes.size()));
        check(file.good(),"write fixture");
    }
    std::cout<<"Library search regression passed\n";
}
