#include "core/analysis/IdeaNoteInput.h"
#include <iostream>
using namespace smartimproviser::harmony;
int main()
{
    int failures=0;
    const auto check=[&](bool ok){ if(!ok) ++failures; };
    auto blank=parseIdeaNoteInput({});
    check(blank.error.empty() && !blank.note.pitch && !blank.note.durationBeats && !blank.note.octaveOffset);
    auto partial=parseIdeaNoteInput({"b9","","","0.5",""});
    check(partial.error.empty() && partial.note.pitch->degree==9 && partial.note.pitch->chromaticOffset==-1
        && partial.note.pitch->chordIndex==-1 && !partial.note.beatOffset);
    IdeaNote original; original.target=true; original.harmonicRole=PhraseNoteRole::resolutionTarget;
    original.fingering=PhraseFingering{2,7};
    auto edited=parseIdeaNoteInput({"#11","-1","2.5","1","0"},original);
    check(edited.error.empty() && edited.note.target==true && edited.note.fingering->fret==7
        && edited.note.harmonicRole==PhraseNoteRole::resolutionTarget && edited.note.octaveOffset==-1);
    for(const auto& input : {IdeaNoteInput{"14","","","",""}, IdeaNoteInput{"b","","","",""},
        IdeaNoteInput{"3","1x","","",""},IdeaNoteInput{"3","","nan","",""},
        IdeaNoteInput{"3","","","0",""},IdeaNoteInput{"","","","","0"},
        IdeaNoteInput{"3","","0.5x","",""},IdeaNoteInput{"3","","","","-1"}})
        check(!parseIdeaNoteInput(input).error.empty());
    check(parseIdeaNoteInput({"3","","0","0.25","1"}).error.empty());
    std::cout << failures << " failures\n"; return failures ? 1 : 0;
}
