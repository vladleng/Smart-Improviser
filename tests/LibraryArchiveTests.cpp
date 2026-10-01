#include "core/analysis/LibraryArchive.h"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace smartimproviser::harmony;
void check(bool ok,const char* text) { if(!ok) { std::cerr<<text<<'\n'; std::exit(1); } }
LibraryRecord musical()
{
    LibraryRecord r; r.id="user:phrase"; r.revision=9; r.name="Фраза — ♭9"; r.tags={"jazz","разрешение"}; r.explanation="Сначала подход, затем цель.";
    r.source={"Own example","Влад","CC0","Own-work declaration",ContentPermission::ownWork};
    r.lineage=LibraryLineage{LibraryDerivation::variant,{LibraryDomain::user,"user:parent",4},{{LibraryDomain::common,"common:base",2}}};
    Phrase p; p.id="legacy:phrase"; p.name="Original payload"; p.harmonicPattern=HarmonicPatternType::majorIiVI;
    p.role=PhraseRole::preparation; p.startDegree=3; p.targetDegree=1; p.tensionLevel=TensionLevel::color; p.tensionClassified=true;
    p.notes={{{0,3,-1},0.125,0.25,false,PhraseNoteRole::passingApproach},{{1,1,0},0.375,0.625,true,PhraseNoteRole::resolutionTarget}};
    p.notes[0].octaveOffset=0; p.notes[1].octaveOffset=-1; p.notes[0].fingering=PhraseFingering{2,8};
    p.evidence.confidence=ConfidenceLevel::medium; p.evidence.markAmbiguous(2); p.evidence.add(EvidenceFlag::patternMatch);
    p.harmonicRequirements={{0,"source:rule",12,3,ChordQuality::dominant,ChordQuality::major,true,HarmonicPatternType::majorIiVI}};
    p.approaches={{0,"concept:enclosure",{0,1}}};
    p.tensionProfile.defined=true; p.tensionProfile.spans={{0,0.5,TensionLevel::color},{0.5,1,std::nullopt}};
    p.registerReferences={{0,62},{1,67}}; p.conceptRuleIds={"concept:enclosure","concept:target"};
    r.content=p; return r;
}
int main()
{
    LibraryRecord idea; idea.id="user:idea"; idea.revision=3; idea.name="Набросок";
    auto& i=std::get<Idea>(idea.content); i.text="E → F, затем пауза"; i.rhythmNotes="синкопа"; i.harmonyNotes="ii–V";
    i.conceptRuleIds={"concept:guide"}; i.harmonicPattern=HarmonicPatternType::majorIiVI;
    IdeaNote partial; partial.pitch=RelativePitch{-1,9,-1}; partial.durationBeats=0.25;
    partial.octaveOffset=0; partial.target=false; partial.harmonicRole=PhraseNoteRole::sourceTone; partial.fingering=PhraseFingering{1,0};
    i.notes={partial,{}}; i.registerReferences={{0,60}};
    LibraryArchive archive{"store:test",std::numeric_limits<std::uint64_t>::max(),{{musical(),idea},{"user:phrase","user:idea","user:deleted"}}};
    const auto encoded=encodeLibraryArchive(archive);
    check(encoded.succeeded(),"rich archive encodes");
    const auto decoded=decodeLibraryArchive(encoded.bytes);
    check(decoded.succeeded() && decoded.archive->generation==archive.generation && decoded.archive->storeId=="store:test","archive keeps exact uint64 generation");
    const auto& records=decoded.archive->users.records;
    const auto& r=records[0]; const auto original=musical(); const auto& p=std::get<Phrase>(r.content);
    check(r.id==original.id && r.revision==9 && r.domain==LibraryDomain::user && r.name==original.name && r.tags==original.tags
        && r.explanation==original.explanation && r.source.source==original.source.source && r.source.author==original.source.author
        && r.source.license==original.source.license && r.source.permissionEvidence==original.source.permissionEvidence
        && r.source.permission==ContentPermission::ownWork,"wrapper and UTF-8 source metadata retained");
    check(r.lineage->kind==LibraryDerivation::variant && r.lineage->parent.id=="user:parent" && r.lineage->parent.revision==4
        && r.lineage->ancestors[0].domain==LibraryDomain::common && r.lineage->ancestors[0].id=="common:base"
        && r.lineage->ancestors[0].revision==2,"exact provenance retained without available originals");
    check(p.id=="legacy:phrase" && p.name=="Original payload" && p.harmonicPattern==HarmonicPatternType::majorIiVI
        && p.role==PhraseRole::preparation && p.startDegree==3 && p.targetDegree==1 && p.tensionClassified
        && p.tensionLevel==TensionLevel::color,"legacy phrase metadata retained");
    check(p.notes.size()==2 && p.notes[0].pitch.chordIndex==0 && p.notes[0].pitch.degree==3 && p.notes[0].pitch.chromaticOffset==-1
        && p.notes[0].beatOffset==0.125 && p.notes[0].durationBeats==0.25 && !p.notes[0].target
        && p.notes[0].harmonicRole==PhraseNoteRole::passingApproach && p.notes[0].octaveOffset==0
        && p.notes[0].fingering->stringNumber==2 && p.notes[0].fingering->fret==8
        && p.notes[1].pitch.chordIndex==1 && p.notes[1].pitch.degree==1 && p.notes[1].pitch.chromaticOffset==0
        && p.notes[1].beatOffset==0.375 && p.notes[1].durationBeats==0.625 && p.notes[1].target
        && p.notes[1].harmonicRole==PhraseNoteRole::resolutionTarget && p.notes[1].octaveOffset==-1 && !p.notes[1].fingering,
        "notes retain timing, targets, roles, explicit register and optional fingering");
    check(p.evidence.confidence==ConfidenceLevel::medium && p.evidence.interpretation==InterpretationStatus::ambiguous
        && p.evidence.alternativeCount==2 && p.evidence.has(EvidenceFlag::patternMatch),"harmonic evidence retained");
    const auto& req=p.harmonicRequirements[0];
    check(req.chordIndex==0 && req.sourceRuleId=="source:rule" && req.sourceRuleVersion==12 && req.sourceRootOffset==3
        && req.chordQuality==ChordQuality::dominant && req.destinationQuality==ChordQuality::major && req.confirmedDestinationRequired
        && req.harmonicPattern==HarmonicPatternType::majorIiVI,"semantic source application retained");
    check(p.approaches[0].chordIndex==0 && p.approaches[0].conceptRuleId=="concept:enclosure"
        && p.approaches[0].noteIndices==std::vector<std::size_t>({0,1}) && p.conceptRuleIds==std::vector<std::string>({"concept:enclosure","concept:target"})
        && p.registerReferences[0].rootMidiNote==62 && p.registerReferences[1].chordIndex==1 && p.registerReferences[1].rootMidiNote==67
        && p.tensionProfile.defined && p.tensionProfile.spans[0].startBeat==0 && p.tensionProfile.spans[0].endBeat==0.5
        && p.tensionProfile.spans[0].level==TensionLevel::color && !p.tensionProfile.spans[1].level,"concepts, contour references and unevaluated tension retained");
    const auto& restoredIdea=std::get<Idea>(records[1].content);
    check(restoredIdea.text==i.text && restoredIdea.rhythmNotes==i.rhythmNotes && restoredIdea.harmonyNotes==i.harmonyNotes
        && restoredIdea.conceptRuleIds==i.conceptRuleIds && restoredIdea.harmonicPattern==i.harmonicPattern
        && restoredIdea.registerReferences[0].rootMidiNote==60 && restoredIdea.notes[0].pitch->chordIndex==-1
        && restoredIdea.notes[0].pitch->degree==9 && restoredIdea.notes[0].pitch->chromaticOffset==-1
        && !restoredIdea.notes[0].beatOffset && restoredIdea.notes[0].durationBeats==0.25 && restoredIdea.notes[0].octaveOffset==0
        && restoredIdea.notes[0].target==false && restoredIdea.notes[0].harmonicRole==PhraseNoteRole::sourceTone
        && restoredIdea.notes[0].fingering->stringNumber==1 && restoredIdea.notes[0].fingering->fret==0
        && !restoredIdea.notes[1].pitch && !restoredIdea.notes[1].durationBeats && !restoredIdea.notes[1].target,
        "partial Idea retains unknown versus explicit zero/false fields");
    check(encodeLibraryArchive(*decoded.archive).bytes==encoded.bytes,"binary roundtrip is exact");
    PhraseLibrary loaded([] { return "user:deleted"; });
    check(loaded.restoreUserSnapshot(decoded.archive->users).succeeded(),"restore preserves unavailable source lineage");
    LibraryRecord draft; std::get<Idea>(draft.content).text="New";
    check(loaded.addUser(draft).status==LibraryStatus::idGenerationFailed,"deleted ID remains reserved after reopen");
    auto before=encodeLibraryArchive({"store:test",1,loaded.userSnapshot()}).bytes;
    auto broken=decoded.archive->users; broken.records[0].revision=0;
    check(!loaded.restoreUserSnapshot(broken).succeeded() && encodeLibraryArchive({"store:test",1,loaded.userSnapshot()}).bytes==before,
        "failed restore leaves old memory data intact");
    broken=decoded.archive->users; broken.reservedIds.pop_back();
    // Replacing reservations with an empty set cannot orphan any live identity.
    broken.reservedIds.clear();
    check(!loaded.restoreUserSnapshot(broken).succeeded(),"live identities require durable reservations");
    for (std::size_t length:{std::size_t(0),std::size_t(19),encoded.bytes.size()-1})
        check(!decodeLibraryArchive(std::span(encoded.bytes).first(length)).succeeded(),"truncated archive rejected");
    auto bad=encoded.bytes; bad.back()^=1;
    check(decodeLibraryArchive(bad).status==LibraryArchiveStatus::malformed,"checksum rejects damaged content");
    bad=encoded.bytes; bad.push_back(0);
    check(!decodeLibraryArchive(bad).succeeded(),"trailing bytes rejected");
    bad=encoded.bytes; bad[8]=2;
    check(decodeLibraryArchive(bad).status==LibraryArchiveStatus::futureSchema,"future schema has explicit protected status");
    bad[8]=0;
    check(decodeLibraryArchive(bad).status==LibraryArchiveStatus::unsupportedSchema,"unknown old schema not silently migrated");
    auto invalid=archive; invalid.users.records[0].domain=LibraryDomain::common;
    check(!encodeLibraryArchive(invalid).succeeded(),"common catalog is not serialized in user file");
    invalid=archive; invalid.users.records[0].name=std::string("\xff",1);
    check(!encodeLibraryArchive(invalid).succeeded(),"invalid UTF-8 cannot be silently replaced");
    invalid=archive; invalid.users.records[0].revision=0;
    check(!encodeLibraryArchive(invalid).succeeded(),"invalid records cannot be persisted");
    auto target=decoded.archive->users;
    check(mergeLibraryUserSnapshot(target,decoded.archive->users).status==LibraryStatus::revisionConflict,"reimport is explicit identity conflict");
    LibraryUserSnapshot addition; LibraryRecord newIdea; newIdea.id="user:other"; newIdea.revision=5;
    addition.records={newIdea}; addition.reservedIds={"user:other","user:previously-deleted"};
    check(mergeLibraryUserSnapshot(target,addition).succeeded() && target.records.size()==3,"independent portable identities merge");
    check(target.records.back().revision==5,"import retains exact revision");
    addition.records[0].id="user:deleted"; addition.reservedIds={"user:deleted"};
    check(mergeLibraryUserSnapshot(target,addition).status==LibraryStatus::revisionConflict && target.records.size()==3,"tombstone conflict rejects import atomically");
    std::cout<<"Library archive tests passed\n";
}
