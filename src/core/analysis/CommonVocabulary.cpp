#include "core/analysis/CommonVocabulary.h"
namespace smartimproviser::harmony
{
namespace
{
PhraseNote note(int slot,int degree,int accidental,double beat,double duration,int octave,
                PhraseNoteRole role=PhraseNoteRole::chordAnchor,bool target=false)
{
    PhraseNote n;n.pitch={slot,degree,accidental};n.beatOffset=beat;n.durationBeats=duration;
    n.octaveOffset=octave;n.harmonicRole=role;n.target=target;return n;
}
LibraryRecord example(bool minor,int kind)
{
    LibraryRecord r;r.domain=LibraryDomain::common;r.revision=1;
    const std::string mode=minor?"minor":"major";
    const std::string suffix=kind==0?"anchors":kind==1?"guides":"approach";
    r.id="common:mvp."+mode+"."+suffix;
    r.name=(minor?"Минор iiø–V–i":"Мажор ii–V–I")+std::string(" · ")
        +(kind==0?"аккордовые опоры":kind==1?"линия guide tones":"подход к терции");
    r.tags={"common-mvp",mode,suffix};
    r.source.source="Smart Improviser original common vocabulary, revision 1";
    r.source.author="Moon River Studio / Smart Improviser";
    r.source.license="CC0-1.0";
    r.source.permission=ContentPermission::ownWork;
    r.source.permissionEvidence="Original short examples authored for this project; content dedication in docs/COMMON_VOCABULARY.md. No book transcription.";
    Phrase p;p.name=r.name;p.harmonicPattern=minor?HarmonicPatternType::minorIiHalfDimVi:HarmonicPatternType::majorIiVI;
    p.role=PhraseRole::statement;p.registerReferences={{0,62},{1,55},{2,60}};
    p.startDegree=kind==1?3:1;p.targetDegree=3;
    for(int slot=0;slot<3;++slot){
        PhraseSlotRequirement requirement;requirement.chordIndex=slot;
        requirement.chordQuality=slot==0?(minor?ChordQuality::halfDiminished:ChordQuality::minor)
            :slot==1?ChordQuality::dominant:(minor?ChordQuality::minor:ChordQuality::major);
        if(slot==1){requirement.destinationQuality=minor?ChordQuality::minor:ChordQuality::major;
            requirement.confirmedDestinationRequired=true;}
        p.harmonicRequirements.push_back(requirement);
    }
    const int third=minor?-1:0;
    if(kind==1){
        p.conceptRuleIds={"concept.guide-targeting"};
        p.notes={note(0,3,-1,0,2,0,PhraseNoteRole::guideTone),
            note(0,7,-1,2,2,-1,PhraseNoteRole::guideTone),
            note(1,7,-1,4,2,0,PhraseNoteRole::guideTone),
            note(1,3,0,6,2,0,PhraseNoteRole::guideTone),
            note(2,3,third,8,4,0,PhraseNoteRole::resolutionTarget,true)};
        r.explanation="Терции и септимы: F4–C4 | F4–B3 | "+std::string(minor?"Eb4":"E4")+
            ". По одному аккорду на 4 beats; 2+2 | 2+2 | 4. Последняя терция — цель разрешения. Tension назначается вручную.";
    } else {
        p.conceptRuleIds={"concept.chord-anchors"};
        p.notes={note(0,1,0,0,1,0),note(0,3,-1,1,1,0),
            note(0,5,minor?-1:0,2,1,0),note(0,7,-1,3,1,0),
            note(1,3,0,4,1,0),note(1,5,0,5,1,0),note(1,7,-1,6,1,0)};
        if(kind==0){
            p.notes.push_back(note(1,5,0,7,1,0));
            r.explanation="Арпеджированные опоры: D4–F4–"+std::string(minor?"Ab4":"A4")+
                "–C5 | B3–D4–F4–D4 | "+(minor?"Eb4–G4–Bb4–G4":"E4–G4–B4–G4")+
                ". Четверти, три аккорда по 4 beats. Терция тоники — цель; остальные ноты — опоры. Tension не оценена.";
        } else {
            p.notes.push_back(note(1,5,minor?0:1,7.5,0.5,0,PhraseNoteRole::passingApproach));
            p.conceptRuleIds.push_back("concept.chromatic-approach-below");
            p.approaches.push_back({1,"concept.chromatic-approach-below",{7,8}});
            r.explanation="Аккордовые опоры ii и V, затем пауза на половину beat и подход снизу: "+
                std::string(minor?"D4 → Eb4":"D#4 → E4")+
                ". Подход на 7.5, приземление в терцию реального i/I на 8. Не вне-контекстная хроматика; Tension не оценена.";
        }
        p.notes.push_back(note(2,3,third,8,kind==0?1:4,0,PhraseNoteRole::resolutionTarget,true));
        if(kind==0){
            p.notes.push_back(note(2,5,0,9,1,0));p.notes.push_back(note(2,7,minor?-1:0,10,1,0));
            p.notes.push_back(note(2,5,0,11,1,0));
        }
    }
    r.content=std::move(p);return r;
}
}
std::vector<LibraryRecord> makeCommonVocabulary()
{
    std::vector<LibraryRecord> result;
    for(bool minor:{false,true})for(int kind=0;kind<3;++kind)result.push_back(example(minor,kind));
    return result;
}
