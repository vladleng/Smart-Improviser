#include "core/analysis/LibraryValidation.h"
#include "core/analysis/PhraseMatcher.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/HarmonicEngine.h"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace smartimproviser::harmony;
void check(bool ok, const char* message)
{
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
LibraryRecord record()
{
    LibraryRecord r;
    r.id = "user:one"; r.revision = 1; r.name = "Guide-tone idea";
    return r;
}
Phrase phrase()
{
    Phrase p;
    // Old aggregate initialization remains valid; new register fields are appended.
    p.notes = {{{0,3,0},0.0,0.5,false,PhraseNoteRole::guideTone},
               {{1,3,0},0.5,0.5,true,PhraseNoteRole::resolutionTarget}};
    p.notes[0].octaveOffset = 0; p.notes[1].octaveOffset = 0;
    p.registerReferences = {{0,55},{1,60}}; // B3 -> E4, NOT B4 -> E4.
    p.conceptRuleIds = {"project.guide-targeting"};
    return p;
}
ChordContext chord(int fifths, std::initializer_list<int> intervals)
{
    ChordContext c; c.available=c.defined=true; c.root=c.bass=fifths;
    for (int n:intervals) c.intervals.values[n]=0xFF;
    return c;
}
int main()
{
    auto r = record();
    auto v = validateLibraryRecord(r);
    check(v.structurallyValid && !v.readyForSearch && !v.distributable,"named empty draft remains a draft");
    auto& idea = std::get<Idea>(r.content);
    idea.text = "Resolve the third upwards";
    IdeaNote partial; partial.pitch=RelativePitch{-1,3,0};
    partial.durationBeats=0.5;
    idea.notes.push_back(partial);
    v = validateLibraryRecord(r);
    check(v.structurallyValid && !v.readyForSearch,"partial note keeps unknown timing/register/context");
    check(!idea.notes[0].beatOffset && !idea.notes[0].octaveOffset && !idea.harmonicPattern,"unknown fields stay unknown");
    idea.notes[0].durationBeats=0;
    check(!validateLibraryRecord(r).structurallyValid,"present zero duration is invalid, not unknown");
    idea.notes[0].durationBeats=0.5;
    idea.notes[0].beatOffset=std::numeric_limits<double>::quiet_NaN();
    check(!validateLibraryRecord(r).structurallyValid,"nonfinite draft timing rejected");
    idea.notes[0].beatOffset.reset();
    r.content=phrase();
    v=validateLibraryRecord(r);
    check(v.structurallyValid && v.readyForSearch && !v.distributable,"complete private phrase needs no publication metadata or tension label");
    auto& p=std::get<Phrase>(r.content);
    check(!p.tensionClassified && !p.tensionProfile.defined,"library does not assign default T1");
    check(phraseExampleMidiPitch(p,0)==59 && phraseExampleMidiPitch(p,1)==64,"cross-slot register preserves an ascending fourth");
    p.notes[0].octaveOffset=1;
    check(phraseExampleMidiPitch(p,0)==71,"octave displacement is explicit");
    p.notes[0].pitch.degree=9; p.notes[0].octaveOffset=0;
    check(phraseExampleMidiPitch(p,0)==69,"compound degree carries its documented octave");
    p.notes[0].pitch.chromaticOffset=-1;
    check(phraseExampleMidiPitch(p,0)==68,"accidental preserves exact example height");
    p=phrase();
    p.notes[0].octaveOffset.reset();
    check(validateLibraryRecord(r).structurallyValid && !validateLibraryRecord(r).readyForSearch
        && !phraseExampleMidiPitch(p,0),"legacy pitch-class phrase stays valid but register is not guessed");
    p=phrase(); p.registerReferences.pop_back();
    check(!validateLibraryRecord(r).readyForSearch,"unknown next-slot root blocks readiness");
    p=phrase(); p.registerReferences.push_back({0,67});
    check(!validateLibraryRecord(r).structurallyValid && !phraseExampleMidiPitch(p,0),"ambiguous root references rejected");
    p=phrase(); p.notes[0].octaveOffset=10;
    check(!validateLibraryRecord(r).readyForSearch && !phraseExampleMidiPitch(p,0),"out-of-MIDI-range reference does not become playable");
    p=phrase(); p.notes[0].octaveOffset=std::numeric_limits<int>::max();
    check(!validateLibraryRecord(r).structurallyValid && !phraseExampleMidiPitch(p,0),"integer overflow cannot manufacture pitch");
    p=phrase(); p.notes[1].beatOffset=-1;
    check(!validateLibraryRecord(r).structurallyValid,"invalid ordering/time rejected");
    p=phrase(); p.notes[0].pitch.degree=14;
    check(!validateLibraryRecord(r).structurallyValid,"unsupported degrees rejected");
    p=phrase(); p.notes[0].harmonicRole=PhraseNoteRole::sourceTone;
    check(!validateLibraryRecord(r).readyForSearch,"source-based note needs explicit source application");
    PhraseSlotRequirement req; req.chordIndex=0; req.sourceRuleId="rule"; req.sourceRuleVersion=1;
    p.harmonicRequirements.push_back(req);
    check(validateLibraryRecord(r).readyForSearch,"exact source identity completes metadata only");
    p.harmonicRequirements[0].sourceRuleVersion=0;
    check(validateLibraryRecord(r).structurallyValid && !validateLibraryRecord(r).readyForSearch,"wildcard source version needs revalidation");
    p.harmonicRequirements[0].sourceRuleVersion=1;
    p.harmonicRequirements.push_back(req);
    check(!validateLibraryRecord(r).structurallyValid,"duplicate source bindings rejected");
    p=phrase(); p.notes[0].harmonicRole=PhraseNoteRole::passingApproach;
    check(!validateLibraryRecord(r).readyForSearch,"unbound chromatic preparation is incomplete");
    p.approaches.push_back({0,"concept",{0,7}});
    check(!validateLibraryRecord(r).structurallyValid,"approach cannot address a missing note");
    p=phrase(); p.notes[0].fingering=PhraseFingering{2,0};
    check(validateLibraryRecord(r).readyForSearch,"optional fingering retains open string");
    p.notes[0].fingering=PhraseFingering{0,-1};
    check(!validateLibraryRecord(r).structurallyValid,"invalid supplied fingering rejected");
    p=phrase(); p.tensionProfile.defined=true;
    p.tensionProfile.spans={{0,0.5,TensionLevel::color},{0.5,1,std::nullopt}};
    check(validateLibraryRecord(r).readyForSearch,"unclassified profile intervals do not block All-search completeness");
    p.tensionProfile.spans.push_back({0.25,0.75,TensionLevel::stable});
    check(!validateLibraryRecord(r).structurallyValid,"existing tension validation rejects overlap");
    p=phrase(); r.revision=0;
    check(!validateLibraryRecord(r).structurallyValid,"library revision zero rejected");
    r.revision=1; r.domain=static_cast<LibraryDomain>(99);
    check(!validateLibraryRecord(r).structurallyValid,"unknown domain rejected");
    r.domain=LibraryDomain::user;

    // Exact provenance and independent value copies; no API/ID generation in 0.5a.
    p=phrase(); p.id="musical:legacy"; p.name="legacy title";
    p.harmonicRequirements.push_back(req);
    p.approaches.push_back({0,"concept",{0,1}});
    p.tensionProfile.defined=true; p.tensionProfile.spans={{0,1,TensionLevel::color}};
    p.notes[0].fingering=PhraseFingering{2,12};
    auto copy=r;
    copy.id="user:copy";
    copy.lineage=LibraryLineage{LibraryDerivation::copy,{r.domain,r.id,r.revision},{}};
    check(validateLibraryRecord(copy).structurallyValid,"copy points at exact source revision");
    auto& cp=std::get<Phrase>(copy.content);
    check(cp.id==p.id && copy.id!=r.id,"payload identity is distinct from library identity");
    cp.notes[0].pitch.degree=5; cp.notes[0].durationBeats=0.25;
    cp.registerReferences[0].rootMidiNote=43; cp.harmonicRequirements[0].sourceRuleVersion=8;
    cp.approaches[0].conceptRuleId="another"; cp.conceptRuleIds[0]="another";
    cp.tensionProfile.spans[0].level=TensionLevel::outsideMaximum; cp.notes[0].fingering->fret=3;
    check(p.notes[0].pitch.degree==3 && p.notes[0].durationBeats==0.5 && p.registerReferences[0].rootMidiNote==55
        && p.harmonicRequirements[0].sourceRuleVersion==1 && p.approaches[0].conceptRuleId=="concept"
        && p.conceptRuleIds[0]=="project.guide-targeting" && p.tensionProfile.spans[0].level==TensionLevel::color
        && p.notes[0].fingering->fret==12,"nested musical payload is value-owned");
    r.revision=2;
    check(copy.lineage->parent.revision==1,"source updates cannot change recorded ancestry");
    copy.lineage->parent.id=copy.id;
    check(!validateLibraryRecord(copy).structurallyValid,"same-item derived identity rejected");
    copy.lineage->parent.id=r.id;
    copy.lineage->ancestors.push_back(copy.lineage->parent);
    check(!validateLibraryRecord(copy).structurallyValid,"repeated ancestry rejected");
    copy.lineage->ancestors.clear();
    copy.lineage->parent.revision=0;
    check(!validateLibraryRecord(copy).structurallyValid,"unknown lineage version cannot claim exact provenance");

    // Publication requirements never block private storage.
    r.content=phrase(); r.lineage.reset(); r.domain=LibraryDomain::common;
    check(validateLibraryRecord(r).readyForSearch && !validateLibraryRecord(r).distributable,"harmonic readiness is independent of permission");
    r.explanation="Guide tones and their resolution.";
    r.source={"original example","Moon River Studio","CC0-1.0","Explicit original-work declaration",ContentPermission::ownWork};
    check(validateLibraryRecord(r).distributable,"common ready item has explicit distribution evidence");
    r.source.permission=ContentPermission::unknown;
    check(!validateLibraryRecord(r).distributable,"license text alone does not imply permission");
    r.source.permission=ContentPermission::ownWork; r.source.permissionEvidence.clear();
    check(!validateLibraryRecord(r).distributable,"permission evidence required for supplied common content");

    // Existing harmonic-first matcher keeps authority, even for complete metadata.
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable=true; snapshot.ppq=4;
    snapshot.globalKey.available=snapshot.globalKey.defined=true;
    snapshot.globalKey.root=0;
    for (int n:{0,2,4,5,7,9,11}) snapshot.globalKey.intervals.values[n]=0xFF;
    snapshot.currentChord=chord(1,{0,4,7,10}); // G7
    snapshot.nextChordAvailable=true; snapshot.nextChord=chord(0,{0,4,7,11});
    auto material=analyzeImprovisation(analyzeHarmonicSituation(snapshot));
    check(material.valid,"real analyzed fixture available");
    Phrase anchor;
    anchor.notes={{{0,3,0},0,0.5,false,PhraseNoteRole::chordAnchor}};
    PhraseMatchRequest query{{{material,0,1}},std::nullopt};
    const auto before=assessPhrase(anchor,query);
    anchor.registerReferences={{0,55}}; anchor.notes[0].octaveOffset=0;
    const auto after=assessPhrase(anchor,query);
    check(before.eligible() && after.eligible() && before.notePitchClasses==after.notePitchClasses,"register extension preserves Stage 4 matcher");
    auto candidate=record(); candidate.content=anchor;
    check(validateLibraryRecord(candidate).readyForSearch,"complete metadata enters context evaluation");
    std::get<Phrase>(candidate.content).notes[0].pitch.chromaticOffset=-1;
    check(validateLibraryRecord(candidate).readyForSearch
        && assessPhrase(std::get<Phrase>(candidate.content),query).harmonic==PhraseCompatibility::incompatible,
        "readiness cannot authorize a wrong chord anchor");
    query.requestedTension=TensionLevel::stable;
    check(assessPhrase(anchor,query).tension==PhraseTensionMatch::unclassified,"new library metadata never silently assigns T1");
    std::cout << "Library contract tests passed\n";
}
