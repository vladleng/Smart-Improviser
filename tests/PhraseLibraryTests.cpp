#include "core/analysis/PhraseLibrary.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace smartimproviser::harmony;
void check(bool ok, const char* why) { if (!ok) { std::cerr<<why<<'\n'; std::exit(1); } }
LibraryRecord draft()
{
    LibraryRecord r; r.name="My sketch"; std::get<Idea>(r.content).text="Start from the third";
    return r;
}
LibraryRecord shared()
{
    LibraryRecord r; r.id="common:guide"; r.revision=7; r.domain=LibraryDomain::common;
    r.name="Guide resolution"; r.explanation="Resolve the third to the tonic.";
    r.source={"original example","Moon River Studio","CC0-1.0","Original work declaration",ContentPermission::ownWork};
    Phrase p; p.notes={{{0,3,0},0,0.5,false,PhraseNoteRole::guideTone}};
    p.notes[0].octaveOffset=0; p.notes[0].fingering=PhraseFingering{2,0};
    p.registerReferences={{0,55}}; p.conceptRuleIds={"concept:guide"};
    p.tensionProfile.defined=true; p.tensionProfile.spans={{0,0.5,TensionLevel::color}};
    r.content=p;
    return r;
}
LibraryItemReference ref(const LibraryRecord& r) { return {r.domain,r.id,r.revision}; }
int main()
{
    int counter=0;
    PhraseLibrary library([&] { return "user:"+std::to_string(++counter); });
    auto common=shared();
    auto invalid=common; invalid.source.permission=ContentPermission::unknown;
    check(library.initializeCommonCatalog({common,invalid}).status==LibraryStatus::invalidRecord,"invalid common catalog rejected");
    check(library.list(LibraryDomain::common).empty(),"catalog validation is atomic");
    check(library.initializeCommonCatalog({common,common}).status==LibraryStatus::invalidIdentity,"duplicate catalog identity rejected");
    check(library.list(LibraryDomain::common).empty(),"duplicate bootstrap leaves no partial catalog");
    check(library.initializeCommonCatalog({common}).succeeded(),"common catalog initialized");
    common.name="external mutation"; std::get<Phrase>(common.content).notes[0].pitch.degree=5;
    check(library.get(LibraryDomain::common,common.id)->name=="Guide resolution"
        && std::get<Phrase>(library.get(LibraryDomain::common,common.id)->content).notes[0].pitch.degree==3,
        "catalog owns a detached immutable snapshot");
    check(library.initializeCommonCatalog({}).status==LibraryStatus::readOnly,"catalog cannot be changed through live user store");
    check(library.updateUser(shared(),7).status==LibraryStatus::readOnly
        && library.eraseUser(common.id,7).status==LibraryStatus::readOnly
        && library.addUser(shared()).status==LibraryStatus::readOnly,"common originals reject all user mutations");
    auto masquerade=shared(); masquerade.domain=LibraryDomain::user;
    check(library.updateUser(masquerade,7).status==LibraryStatus::readOnly,"changing domain cannot edit a common identity");
    check(!library.get(LibraryDomain::user,common.id),"lookup respects domain");
    check(!library.get(static_cast<LibraryDomain>(99),common.id)
        && library.list(static_cast<LibraryDomain>(99)).empty(),"unknown domain never falls back to another domain");

    auto input=draft();
    const auto added=library.addUser(input);
    check(added.succeeded() && added.record->revision==1 && added.record->id=="user:1"
        && added.validation->structurallyValid && !added.validation->readyForSearch,"draft receives new identity but not invented readiness");
    check(input.id.empty() && input.revision==0,"insertion does not rewrite caller's draft");
    auto original=*added.record;
    input.name="caller changed"; std::get<Idea>(input.content).text="changed";
    check(library.get(LibraryDomain::user,original.id)->name=="My sketch","caller mutation cannot affect stored item");
    auto returned=library.get(LibraryDomain::user,original.id);
    returned->name="read mutated";
    auto listed=library.list(LibraryDomain::user); listed[0].name="list mutated";
    check(library.get(LibraryDomain::user,original.id)->name=="My sketch","get/list return detached values");
    auto stale=original, next=original; next.name="Revised"; std::get<Idea>(next.content).notes.push_back({});
    auto updated=library.updateUser(next,1);
    check(updated.succeeded() && updated.record->revision==2 && updated.record->name=="Revised","edit increments one revision");
    check(next.revision==1 && stale.name=="My sketch","update does not mutate caller or old snapshots");
    check(library.updateUser(stale,1).status==LibraryStatus::revisionConflict,"stale edit does not overwrite newer data");
    next=*updated.record; next.revision=1;
    check(library.updateUser(next,2).status==LibraryStatus::revisionConflict,"submitted revision must match expected");
    check(library.eraseUser(original.id,1).status==LibraryStatus::revisionConflict,"stale deletion rejected");
    auto bad=*updated.record; std::get<Idea>(bad.content).notes[0].durationBeats=0;
    check(library.updateUser(bad,2).status==LibraryStatus::invalidRecord
        && library.get(LibraryDomain::user,original.id)->revision==2,"failed edit leaves current revision intact");
    check(library.addUser(original).status==LibraryStatus::invalidIdentity,"add is not implicit import/upsert");
    auto lineageDraft=draft(); lineageDraft.lineage=LibraryLineage{LibraryDerivation::copy,ref(shared()),{}};
    check(library.addUser(lineageDraft).status==LibraryStatus::lineageConflict,"original insertion cannot forge ancestry");
    bad=*updated.record; bad.lineage=lineageDraft.lineage;
    check(library.updateUser(bad,2).status==LibraryStatus::lineageConflict,"edit cannot attach forged ancestry");
    bad=*updated.record; bad.id="missing";
    check(library.updateUser(bad,2).status==LibraryStatus::notFound
        && library.eraseUser("missing",1).status==LibraryStatus::notFound,"missing user mutation explicit");

    auto first=library.copyToUser(ref(shared()));
    auto second=library.copyToUser(ref(shared()));
    check(first.succeeded() && second.succeeded() && first.record->id!=second.record->id
        && first.record->domain==LibraryDomain::user && first.record->revision==1,"independent common copies have fresh user identities");
    check(first.record->lineage->kind==LibraryDerivation::copy
        && first.record->lineage->parent.id=="common:guide" && first.record->lineage->parent.revision==7,
        "copy retains exact common provenance");
    auto local=*first.record;
    auto& lp=std::get<Phrase>(local.content);
    lp.notes[0].pitch.degree=5; lp.notes[0].durationBeats=0.25;
    lp.notes[0].fingering->fret=12; lp.registerReferences[0].rootMidiNote=43;
    lp.conceptRuleIds[0]="other concept"; lp.tensionProfile.spans[0].endBeat=0.25;
    lp.tensionProfile.spans[0].level=TensionLevel::outsideMaximum;
    check(library.updateUser(local,1).succeeded(),"copied phrase is editable");
    const auto untouchedCommon=library.get(LibraryDomain::common,"common:guide");
    const auto untouchedCopy=library.get(LibraryDomain::user,second.record->id);
    for (const auto* r:{&*untouchedCommon,&*untouchedCopy})
    {
        const auto& p=std::get<Phrase>(r->content);
        check(p.notes[0].pitch.degree==3 && p.notes[0].durationBeats==0.5 && p.notes[0].fingering->fret==0
            && p.registerReferences[0].rootMidiNote==55 && p.conceptRuleIds[0]=="concept:guide"
            && p.tensionProfile.spans[0].level==TensionLevel::color,"original and sibling retain all nested content");
    }
    auto changedCopy=library.get(LibraryDomain::user,first.record->id);
    auto variant=library.createVariant(ref(*changedCopy));
    check(variant.succeeded() && variant.record->id!=changedCopy->id
        && variant.record->lineage->kind==LibraryDerivation::variant
        && variant.record->lineage->parent.id==changedCopy->id && variant.record->lineage->parent.revision==2
        && variant.record->lineage->ancestors.size()==1
        && variant.record->lineage->ancestors[0].revision==7,"variant records immediate exact revision and older ancestry");
    auto third=library.copyToUser(ref(*variant.record));
    check(third.succeeded() && third.record->lineage->ancestors.size()==2
        && third.record->lineage->ancestors[0].id==changedCopy->id
        && third.record->lineage->ancestors[1].id=="common:guide","multi-generation copy retains ordered lineage");
    auto editedVariant=*variant.record;
    editedVariant.lineage->parent.revision=1;
    check(library.updateUser(editedVariant,1).status==LibraryStatus::lineageConflict,"copy cannot rewrite exact source revision");
    auto outdated=ref(*changedCopy); outdated.revision=1;
    const auto countBefore=library.list(LibraryDomain::user).size();
    check(library.copyToUser(outdated).status==LibraryStatus::revisionConflict
        && library.createVariant(outdated).status==LibraryStatus::revisionConflict
        && library.list(LibraryDomain::user).size()==countBefore,"stale source does not produce a misleading copy");

    auto copiedDraft=library.copyToUser(ref(*updated.record));
    check(copiedDraft.succeeded() && std::holds_alternative<Idea>(copiedDraft.record->content)
        && !copiedDraft.validation->readyForSearch,"draft copy stays a draft");
    auto sourceEdit=*updated.record; std::get<Idea>(sourceEdit.content).text="New source idea";
    check(library.updateUser(sourceEdit,2).succeeded(),"source changes independently");
    check(std::get<Idea>(library.get(LibraryDomain::user,copiedDraft.record->id)->content).text=="Start from the third"
        && copiedDraft.record->lineage->parent.revision==2,"source edit does not rewrite a copied draft");
    check(library.eraseUser(original.id,3).succeeded() && !library.get(LibraryDomain::user,original.id),"source can be deleted");
    check(library.get(LibraryDomain::user,copiedDraft.record->id).has_value()
        && std::get<Idea>(library.get(LibraryDomain::user,copiedDraft.record->id)->content).text=="Start from the third",
        "deleting source preserves copy value and provenance");
    check(library.createVariant(ref(*updated.record)).status==LibraryStatus::notFound,"deleted source explicitly unavailable");
    check(library.eraseUser(changedCopy->id,2).succeeded()
        && std::get<Phrase>(library.get(LibraryDomain::user,variant.record->id)->content).notes[0].pitch.degree==5,
        "deleting parent preserves phrase variant material");
    auto variantEdit=*variant.record; variantEdit.name="After deleting parent";
    check(library.updateUser(variantEdit,1).succeeded(),"variant can be edited without live parent");

    LibraryItemReference unknown{static_cast<LibraryDomain>(99),"x",1};
    check(library.copyToUser(unknown).status==LibraryStatus::invalidIdentity,"unknown source domain rejected");
    check(library.copyToUser({LibraryDomain::common,"common:guide",0}).status==LibraryStatus::invalidIdentity,"source revision zero rejected");
    PhraseLibrary noGenerator(PhraseLibrary::IdGenerator{});
    check(noGenerator.addUser(draft()).status==LibraryStatus::idGenerationFailed,"missing generator is safe");
    PhraseLibrary throwing([]()->std::string { throw std::runtime_error("no entropy"); });
    check(throwing.addUser(draft()).status==LibraryStatus::idGenerationFailed && throwing.list(LibraryDomain::user).empty(),"generator exception does not mutate library");
    PhraseLibrary collision([] { return "same"; });
    auto one=collision.addUser(draft());
    check(one.succeeded() && collision.eraseUser("same",1).succeeded()
        && collision.addUser(draft()).status==LibraryStatus::idGenerationFailed,"deleted IDs remain reserved against lineage reuse");
    int attempts=0;
    PhraseLibrary retry([&] { return ++attempts<3 ? "common:guide" : "new"; });
    check(retry.initializeCommonCatalog({shared()}).succeeded() && retry.addUser(draft()).succeeded()
        && attempts==3,"generator retries collisions across both domains");
    PhraseLibrary whitespace([] { return " \t"; });
    check(whitespace.addUser(draft()).status==LibraryStatus::idGenerationFailed,"blank generated identity rejected");
    auto wrongDomain=draft(); wrongDomain.domain=static_cast<LibraryDomain>(99);
    check(library.addUser(wrongDomain).status==LibraryStatus::invalidIdentity,"unknown insert domain rejected");
    PhraseLibrary atomic([] { return "generated"; });
    auto invalidDraft=draft(); IdeaNote n; n.durationBeats=0; std::get<Idea>(invalidDraft.content).notes={n};
    check(atomic.addUser(invalidDraft).status==LibraryStatus::invalidRecord && atomic.addUser(draft()).succeeded(),
        "invalid draft never reserves an ID or inserts partial data");
    auto uuid=makeLibraryItemId();
    check(uuid.size()==41 && uuid.substr(0,5)=="user:" && uuid[19]=='4'
        && (uuid[24]=='8'||uuid[24]=='9'||uuid[24]=='a'||uuid[24]=='b'),"default generator emits a namespaced UUID v4");
    std::cout<<"Phrase Library API tests passed\n";
}
