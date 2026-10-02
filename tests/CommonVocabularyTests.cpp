#include "core/analysis/CommonVocabulary.h"
#include "core/analysis/PhraseLibrary.h"
#include "core/analysis/LibraryArchive.h"
#include "core/analysis/LibrarySearch.h"
#include "context/LibrarySearchContext.h"
#include <cstdlib>
#include <iostream>
using namespace smartimproviser::harmony;
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
LibrarySearchQuery query(bool minor,int transpose=0){
    SharedHarmonicContextSnapshot s;s.connected=s.transportAvailable=s.sheetChordsAvailable=true;
    s.transportPpq=0;s.sheetChordStoredCount=4;
    const int roots[]={2,1,0,-1};
    for(int i=0;i<4;++i){
        auto& c=s.sheetChords[i];c.position=i*4;c.root=c.bass=roots[i]+transpose;
        c.intervals[0]=0xff;
        if(i==0){for(int t:{3,minor?6:7,10})c.intervals[t]=0xff;}
        else if(i==1){for(int t:{4,7,10})c.intervals[t]=0xff;}
        else {for(int t:{minor?3:4,7,minor?10:11})c.intervals[t]=0xff;}
    }
    s.keySignaturesAvailable=true;s.keySignatureStoredCount=1;s.keySignatures[0].root=transpose;
    for(int t:{0,2,minor?3:4,5,7,minor?8:9,minor?10:11})s.keySignatures[0].intervals[t]=0xff;
    return makeLibrarySearchQuery(s,0);
}
int main(){
    const auto records=makeCommonVocabulary();check(records.size()==6,"six originals");
    PhraseLibrary library;check(library.initializeCommonCatalog(records).succeeded(),"atomic distributable bootstrap");
    check(!library.initializeCommonCatalog(records).succeeded(),"common immutable");
    const std::vector<std::vector<int>> pitches={
        {62,65,69,72,59,62,65,62,64,67,71,67},{65,60,65,59,64},
        {62,65,69,72,59,62,65,63,64},
        {62,65,68,72,59,62,65,62,63,67,70,67},{65,60,65,59,63},
        {62,65,68,72,59,62,65,62,63}};
    for(std::size_t i=0;i<records.size();++i){
        const auto validation=validateLibraryRecord(records[i]);check(validation.structurallyValid && validation.readyForSearch && validation.distributable,"each example complete");
        const auto& p=std::get<Phrase>(records[i].content);check(!p.tensionClassified && !p.tensionProfile.defined,"no inferred tension");
        check(p.notes.size()==pitches[i].size(),"note count");
        for(std::size_t j=0;j<p.notes.size();++j)check(phraseExampleMidiPitch(p,j)==pitches[i][j],"authored register and contour");
        check(!p.approaches.empty() || i%3!=2,"complete chromatic group");
        const auto copy=library.copyToUser({LibraryDomain::common,records[i].id,1});
        check(copy.succeeded() && copy.record->id!=records[i].id && copy.record->lineage->parent.revision==1,"independent exact source copy");
        auto changed=*copy.record;auto& personal=std::get<Phrase>(changed.content);
        personal.tensionClassified=true;personal.tensionLevel=TensionLevel::color;
        check(library.updateUser(changed,1).succeeded(),"manual label saved");
        check(!std::get<Phrase>(library.get(LibraryDomain::common,records[i].id)->content).tensionClassified,"original remains unclassified");
    }
    for(bool minor:{false,true}){
        auto q=query(minor);check(q.match.slots.size()==3,"known three slot ranges");
        auto found=searchLibrary(records,q);
        if(found.eligibleCount!=3){for(const auto& e:found.entries)if(e.record.tags[1]==(minor?"minor":"major")){
            std::cerr<<e.record.id<<'\n';for(const auto& d:e.assessment.diagnostics)std::cerr<<d.explanation<<'\n';}}
        check(found.eligibleCount==3,"exactly three compatible major/minor originals");
        q.match.requestedTension=TensionLevel::stable;check(searchLibrary(records,q).eligibleCount==0,"unclassified absent from T1");
        q=query(minor,1);check(searchLibrary(records,q).eligibleCount==3,"relative harmonic match in another key");
        q.match.slots.resize(2);check(searchLibrary(records,q).eligibleCount==0,"no fabricated tonic boundary");
    }
    auto snapshot=library.userSnapshot();auto bytes=encodeLibraryArchive({"common-copy-test",1,snapshot});
    check(bytes.succeeded(),"copy persistence includes register/roles/lineage/manual tension");
    auto decoded=decodeLibraryArchive(bytes.bytes);check(decoded.succeeded(),"copy roundtrip");
    std::cout<<"Common vocabulary regression passed\n";
}
