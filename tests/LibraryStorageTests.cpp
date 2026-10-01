#include "library/SharedUserLibrary.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
using namespace smartimproviser::harmony;
void check(bool ok,const char* why) { if(!ok) { std::cerr<<why<<'\n'; std::exit(1); } }
LibraryRecord draft(const std::string& text="Незавершённая идея")
{ LibraryRecord r; r.name="My idea"; std::get<Idea>(r.content).text=text; return r; }
LibraryRecord phrase()
{
    LibraryRecord r; r.name="Registered phrase"; r.explanation="A guide-tone target";
    Phrase p; p.notes={{{0,3,0},0,0.5,false,PhraseNoteRole::guideTone},{{1,1,0},0.5,0.5,true,PhraseNoteRole::resolutionTarget}};
    p.notes[0].octaveOffset=0; p.notes[1].octaveOffset=0; p.registerReferences={{0,55},{1,60}};
    p.tensionProfile.defined=true; p.tensionProfile.spans={{0,0.5,TensionLevel::color},{0.5,1,std::nullopt}};
    r.content=p; return r;
}
bool waitFor(const juce::File& file,int milliseconds=10000)
{
    const auto deadline=juce::Time::getMillisecondCounterHiRes()+milliseconds;
    while(!file.existsAsFile() && juce::Time::getMillisecondCounterHiRes()<deadline) juce::Thread::sleep(5);
    return file.existsAsFile();
}
int worker(int argc,char** argv)
{
    if(argc!=7) return 99;
    juce::File file(juce::String::fromUTF8(argv[2])), ready(juce::String::fromUTF8(argv[4])), barrier(juce::String::fromUTF8(argv[5]));
    const std::string id=argv[3];
    SharedUserLibrary storage(file); PhraseLibrary library([id] { return id; });
    auto loaded=storage.loadInto(library); if(!loaded.succeeded() || !library.addUser(draft(id)).succeeded()) return 99;
    if(!ready.replaceWithText("ready") || !waitFor(barrier)) return 99;
    const auto written=storage.commitFrom(library,loaded.stamp);
    return written.succeeded()?0:written.status==LibraryStorageStatus::conflict?23:99;
}
int main(int argc,char** argv)
{
    if(argc>1 && std::string(argv[1])=="--writer") return worker(argc,argv);
    const auto tempRoot=juce::File::getSpecialLocation(juce::File::tempDirectory);
    const auto folder=tempRoot.getNonexistentChildFile("SmartImproviserLibraryTests","",false);
    check(folder.getParentDirectory()==tempRoot && folder.createDirectory().wasOk(),"private test directory created");
    struct Cleanup { juce::File folder; ~Cleanup() { folder.deleteRecursively(); } } cleanup{folder};
    const auto file=folder.getChildFile(juce::String::fromUTF8("библиотека/user-library.silibrary"));
    SharedUserLibrary store(file), another(file);
    auto missing=store.read();
    check(missing.succeeded() && !missing.stamp.exists && missing.users.records.empty() && !file.exists(),"missing file reads empty without writes");
    int n=0; PhraseLibrary library([&] { return "user:"+std::to_string(++n); });
    auto idea=library.addUser(draft()); auto melody=library.addUser(phrase());
    check(idea.succeeded() && melody.succeeded(),"storage fixtures valid");
    auto copy=library.createVariant({LibraryDomain::user,melody.record->id,1});
    check(copy.succeeded(),"variant fixture valid");
    auto saved=store.commitFrom(library,missing.stamp);
    check(saved.succeeded() && saved.stamp.exists && saved.stamp.generation==1 && file.existsAsFile(),"first save initializes separate library file");
    auto bytes=store.exportData();
    check(bytes.succeeded() && !bytes.exportedBytes.empty(),"portable export");
    PhraseLibrary reopened; SharedUserLibrary fresh(file);
    auto loaded=fresh.loadInto(reopened);
    check(loaded.succeeded() && reopened.list(LibraryDomain::user).size()==3,"new instance reopens independent records");
    const auto registered=reopened.get(LibraryDomain::user,melody.record->id);
    check(phraseExampleMidiPitch(std::get<Phrase>(registered->content),0)==59
        && phraseExampleMidiPitch(std::get<Phrase>(registered->content),1)==60,"stored octave/contour survives reopen");
    check(std::get<Idea>(reopened.get(LibraryDomain::user,idea.record->id)->content).text=="Незавершённая идея","Unicode draft survives reopen");
    check(reopened.get(LibraryDomain::user,copy.record->id)->lineage->parent.id==melody.record->id,"lineage survives disk roundtrip");
    check(encodeLibraryArchive({loaded.stamp.storeId,loaded.stamp.generation,reopened.userSnapshot()}).bytes==bytes.exportedBytes,"full stored snapshot exactly restored");
    auto edit=*reopened.get(LibraryDomain::user,idea.record->id); std::get<Idea>(edit.content).text="Edited in another project";
    check(reopened.updateUser(edit,1).succeeded(),"independent project edit");
    auto updated=another.commitFrom(reopened,loaded.stamp);
    check(updated.succeeded() && updated.stamp.generation==2,"another instance saves with fresh token");
    const auto protectedBytes=another.exportData().exportedBytes;
    check(store.commitFrom(library,saved.stamp).status==LibraryStorageStatus::conflict
        && store.exportData().exportedBytes==protectedBytes,"stale instance cannot erase another project's edits");
    check(reopened.eraseUser(melody.record->id,1).succeeded(),"source deletion");
    auto deleted=store.commitFrom(reopened,updated.stamp);
    check(deleted.succeeded(),"deletion persists");
    PhraseLibrary afterDeletion([] { return "user:2"; });
    check(store.loadInto(afterDeletion).succeeded() && afterDeletion.get(LibraryDomain::user,copy.record->id).has_value()
        && std::get<Phrase>(afterDeletion.get(LibraryDomain::user,copy.record->id)->content).notes.size()==2,"source deletion leaves variant payload intact after reopen");
    check(afterDeletion.addUser(draft()).status==LibraryStatus::idGenerationFailed,"durable deleted ID cannot be reused");
    const auto intact=store.exportData().exportedBytes;
    auto unsaved=*reopened.get(LibraryDomain::user,idea.record->id); std::get<Idea>(unsaved.content).text="Unsaved work";
    check(reopened.updateUser(unsaved,2).succeeded(),"unsaved edit available for failure test");
    for (auto stage:{LibraryWriteStage::beforeWrite,LibraryWriteStage::beforeReplace})
    {
        SharedUserLibrary failing(file,[stage](LibraryWriteStage s) { return s!=stage; });
        check(failing.commitFrom(reopened,deleted.stamp).status==LibraryStorageStatus::writeFailed
            && store.exportData().exportedBytes==intact,"injected write failure preserves prior file byte-for-byte");
        check(std::get<Idea>(reopened.get(LibraryDomain::user,idea.record->id)->content).text=="Unsaved work","write failure does not discard in-memory work");
    }
    SharedUserLibrary throwing(file,[](LibraryWriteStage)->bool { throw 1; });
    check(throwing.commitFrom(reopened,deleted.stamp).status==LibraryStorageStatus::writeFailed
        && store.exportData().exportedBytes==intact,"throwing write hook preserves old file");
    auto bad=intact; bad.back()^=1;
    check(file.replaceWithData(bad.data(),bad.size()),"write isolated corrupt fixture");
    check(store.read().status==LibraryStorageStatus::malformed && !store.loadInto(reopened).succeeded()
        && reopened.list(LibraryDomain::user).size()==2,"failed corrupt-file reload preserves live memory");
    check(!store.commitFrom(reopened,deleted.stamp).succeeded() && !store.importData(intact).succeeded(),"corrupt target is never overwritten by save/import");
    juce::MemoryBlock unchanged; check(file.loadFileAsData(unchanged) && unchanged.getSize()==bad.size()
        && std::equal(bad.begin(),bad.end(),static_cast<const std::uint8_t*>(unchanged.getData())),"corrupt bytes preserved");
    bad=intact; bad[8]=2; check(file.replaceWithData(bad.data(),bad.size()),"write isolated future fixture");
    check(store.read().status==LibraryStorageStatus::futureSchema && store.commitFrom(reopened,deleted.stamp).status==LibraryStorageStatus::futureSchema
        && !store.exportData().succeeded(),"future schema refused without rewrite");
    check(file.replaceWithData(intact.data(),intact.size()),"restore isolated original fixture");
    SharedUserLibrary disappeared(folder.getChildFile("absent.silibrary"));
    check(!disappeared.loadInto(reopened).succeeded() && reopened.list(LibraryDomain::user).size()==2,
        "missing file cannot discard an existing in-memory library");
    const auto importedFile=folder.getChildFile("imported.silibrary"); SharedUserLibrary imported(importedFile);
    const auto restored=imported.importData(intact);
    check(restored.succeeded() && restored.users.records.size()==2 && restored.stamp.storeId!=deleted.stamp.storeId,"portable import has independent store identity");
    const auto importedBytes=imported.exportData().exportedBytes;
    check(imported.importData(intact).status==LibraryStorageStatus::conflict
        && imported.exportData().exportedBytes==importedBytes,"reimport identity conflict is atomic");
    check(imported.importData(bad).status==LibraryStorageStatus::futureSchema
        && imported.exportData().exportedBytes==importedBytes,"future import leaves valid target unchanged");
    auto extra=draft("Portable addition"); extra.id="user:extra"; extra.revision=8;
    const auto batch=encodeLibraryArchive({"store:portable",19,{{extra},{"user:extra"}}});
    check(batch.succeeded() && imported.importData(batch.bytes).succeeded()
        && imported.read().users.records.size()==3,"independent portable identities merged");
    const auto directoryTarget=folder.getChildFile("directory.silibrary"); directoryTarget.createDirectory();
    SharedUserLibrary invalidTarget(directoryTarget);
    check(!invalidTarget.read().succeeded() && !invalidTarget.commit({},{}).succeeded() && directoryTarget.isDirectory(),"directory target cannot be replaced");

    // Two separate OS processes both read generation 1 before either may commit.
    const auto concurrentFile=folder.getChildFile("concurrent.silibrary");
    SharedUserLibrary concurrent(concurrentFile); check(concurrent.commit({},{}).succeeded(),"concurrency fixture initialized");
    const auto ready1=folder.getChildFile("ready1"),ready2=folder.getChildFile("ready2"),barrier=folder.getChildFile("barrier");
    const auto executable=juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName();
    juce::ChildProcess first,second;
    check(first.start(juce::StringArray{executable,"--writer",concurrentFile.getFullPathName(),"user:worker1",ready1.getFullPathName(),barrier.getFullPathName(),"unused"})
        && second.start(juce::StringArray{executable,"--writer",concurrentFile.getFullPathName(),"user:worker2",ready2.getFullPathName(),barrier.getFullPathName(),"unused"}),"child processes started");
    check(waitFor(ready1) && waitFor(ready2) && barrier.replaceWithText("go"),"both processes read before committing");
    check(first.waitForProcessToFinish(10000) && second.waitForProcessToFinish(10000),"child processes completed");
    const auto exit1=first.getExitCode(),exit2=second.getExitCode();
    check((exit1==0 && exit2==23)||(exit1==23 && exit2==0),"exactly one writer wins and stale writer reports conflict");
    const auto concurrentResult=concurrent.read();
    check(concurrentResult.succeeded() && concurrentResult.stamp.generation==2 && concurrentResult.users.records.size()==1,"valid complete winner snapshot persisted");
    std::cout<<"Shared library storage tests passed\n";
}
