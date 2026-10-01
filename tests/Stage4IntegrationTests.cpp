#include "context/TimelineContextMapper.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/TensionEngine.h"
#include "core/analysis/ExplanationText.h"
#include "core/analysis/ManualTensionKey.h"
#include "core/analysis/MaterialSelection.h"
#include "core/analysis/TensionFilter.h"
#include "core/analysis/TensionTimeline.h"
#include <cstdlib>
#include <iostream>
#include <map>
using namespace smartimproviser::harmony;
void check(bool ok, const char* message)
{
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
SharedHarmonicContextSnapshot song(int key)
{
    SharedHarmonicContextSnapshot s;
    s.connected = s.hostContentAccessAvailable = true; s.musicalContextCount = 1;
    s.keySignaturesAvailable = s.sheetChordsAvailable = s.transportAvailable = true;
    s.keySignatureEventCount = s.keySignatureStoredCount = 1;
    s.keySignatures[0].root = key;
    for (int n : {0,2,4,5,7,9,11}) s.keySignatures[0].intervals[n] = 0xFF;
    s.sheetChordEventCount = s.sheetChordStoredCount = 5;
    const int roots[] = {key+4,key+3,key+2,key+1,key};
    for (int i = 0; i < 5; ++i)
    {
        auto& c = s.sheetChords[i]; c.position = i*4; c.root = c.bass = roots[i];
        for (int n : {0,4,7,10}) c.intervals[n] = 0xFF;
    }
    s.sheetChords[0].intervals[4] = 0; s.sheetChords[0].intervals[3] = 0xFF;
    s.sheetChords[1].intervals[8] = 13; // A7b13 -> D7; presumed Dm, actual D7.
    s.sheetChords[4].intervals[10] = 0; s.sheetChords[4].intervals[11] = 0xFF;
    return s;
}
ImprovisationResult at(const SharedHarmonicContextSnapshot& s, double beat)
{
    auto r = analyzeImprovisation(analyzeHarmonicSituation(mapTimelineHarmonicSnapshot(s,beat),mapPatternTimelineWindow(s,beat)));
    const auto stable = analyzeStableTension(r);
    for (const auto& c : stable.bands[0].alternatives) r.strategies.push_back(c.strategy);
    return r;
}
int find(const ExplanationResult& e, const char* rule)
{
    for (std::size_t i=0;i<e.items.size();++i)
        if (e.items[i].sourceRuleId == rule) return static_cast<int>(i);
    return -1;
}
std::vector<TensionFilterRow> rows(const ImprovisationResult& r, const ExplanationResult& e,
    const std::map<std::string,int>& labels, int filter)
{
    const int base = playingBaseIndex(r,e);
    std::vector<TensionFilterItem> items;
    for (std::size_t i=0;i<e.items.size();++i)
    {
        const auto key = manualTensionKey(r,e.items[i]);
        const auto found = labels.find(key);
        const int level = found == labels.end() ? 0 : found->second;
        items.push_back({static_cast<int>(i),level,static_cast<int>(i)==base,
            compactMaterialHidden(e,static_cast<int>(i)) || (isDiatonicFoundation(e.items[i]) && static_cast<int>(i)!=base)});
    }
    return filteredTensionRows(items,filter);
}
int main()
{
    std::string relativeKey;
    for (int key=-5;key<=6;++key)
    {
        auto s = song(key);
        const auto r = at(s,4.25); const auto e = buildExplanation(r);
        const int source = find(e,"project.harmonic-minor.contextual-V");
        const int arp = find(e,"project.minor-V.bII-dim7-arpeggio");
        check(r.valid && source>=0 && arp>=0,"mapped minor-destination context reaches source catalog");
        const auto manualKey = manualTensionKey(r,e.items[source]);
        check(!manualKey.empty(),"selectable source has a preference key");
        if (relativeKey.empty()) relativeKey = manualKey;
        check(manualKey == relativeKey,"all keys share the same relative manual assignment");
        check(!e.items[source].tensionClassified && !e.items[arp].tensionClassified,
            "whole harmonic minor and diminished arpeggio are not automatically classified");
        const int base = playingBaseIndex(r,e);
        std::map<std::string,int> labels {{manualKey,2}};
        for (int filter : {0,1,2,3})
        {
            const auto filtered = rows(r,e,labels,filter);
            check(base<0 || (!filtered.empty() && filtered.front().materialIndex==base
                && filtered.front().baseMode && filtered.front().level==0),"one foundation stays first outside all tension labels");
            const bool assigned = hasTensionRecommendation(filtered);
            if (filter!=0) check(assigned==(filter==2),"manual assignment alone controls requested recommendations");
            const int selected = filteredMaterialSelection(filtered,-1,filter);
            if (selected>=0)
            {
                const auto view = buildMaterialView(r,e,selected);
                check(view.valid && view.chord.rootPitchClass==r.context.currentChord.rootPitchClass,
                    "selected material viewer retains actual chord root regardless of source tonic");
                bool root=false;
                for (const auto& n : view.current)
                    root = root || (n.pitchClass==view.chord.rootPitchClass && (n.roles&chordRole));
                check(root,"actual root remains a viewer chord anchor for the white fretboard marker");
            }
        }
        auto filtered = rows(r,e,labels,2);
        check(filteredMaterialSelection(filtered,-1,2)==source,"assigned source drives selection and viewer");
        labels[manualKey]=0;
        filtered=rows(r,e,labels,2);
        check(!hasTensionRecommendation(filtered) && filteredMaterialSelection(filtered,source,2)!=source,
            "clearing a shared assignment cannot leave a hidden source selected");

        const auto view = buildMaterialView(r,e,source);
        check(view.sourceRootPitchClass != view.chord.rootPitchClass && view.nextChord.quality==ChordQuality::dominant,
            "harmonic minor source and actual D7 stay separate in viewer");
        const auto destination = dominantDestination(r.context);
        check(destination.minor() && !destination.confirmed,"actual D7 is not a confirmed Dm or a tonicized center");
        const auto text = explanationDiagnosticText(r);
        check(text.find(e.items[source].source.name)!=std::string::npos
            && !e.items[source].applicationConditions.empty()
            && text.find(e.items[source].applicationConditions.front())!=std::string::npos,
            "harmonic source remains traceable in explanation diagnostics");

        // Source F over A7b13 and actual F# as D7's third. Both labels are manual.
        Phrase phrase;
        phrase.notes={{{0,13,-1},0,1,false,PhraseNoteRole::sourceTone},
                      {{1,3,0},4,1,true,PhraseNoteRole::resolutionTarget}};
        const auto& item=e.items[source]; const auto& strategy=r.strategies[item.strategyIndices.front()];
        PhraseSlotRequirement requirement; requirement.chordIndex=0; requirement.sourceRuleId=strategy.ruleId;
        requirement.sourceRuleVersion=strategy.ruleVersion;
        requirement.sourceRootOffset=(strategy.source.rootPitchClass-r.context.currentChord.rootPitchClass+12)%12;
        requirement.destinationQuality=ChordQuality::minor;
        phrase.harmonicRequirements={requirement};
        phrase.tensionProfile={true,{{0,4,TensionLevel::color},{4,5,TensionLevel::stable}}};
        const std::vector<PhraseMatchSlot> slots {{r,0,4},{at(s,8.25),4,8}};
        TensionCurve curve {{{20,24,TensionLevel::color},{24,25,TensionLevel::stable}}};
        auto match = assessPhraseAgainstCurve(phrase,slots,curve,20);
        check(match.eligible(),"ARA analysis -> source application -> descriptive profile -> desired curve agrees");
        phrase.notes[1].pitch.chromaticOffset=-1;
        match=assessPhraseAgainstCurve(phrase,slots,curve,20);
        check(!match.eligible() && match.harmonicMatch.harmonic==PhraseCompatibility::incompatible
            && match.tension==TensionCurveMatch::notEvaluated,"presumed minor source cannot authorize F as the actual D7 third");
        for (double seek : {12.25,0.25,4.25,16.25,8.25,4.25})
        {
            s.transportPpq=seek; s.transportPlaying=false;
            const auto stopped=at(s,seek);
            s.transportPlaying=true;
            const auto playing=at(s,seek);
            check(stopped.context.currentChord.rootPitchClass==playing.context.currentChord.rootPitchClass
                && stopped.strategies.size()==playing.strategies.size(),"PLAY/STOP state does not rewrite harmony at the same seek position");
        }
    }
    SharedHarmonicContextSnapshot missing;
    check(!at(missing,0).valid,"missing ARA harmonic data does not retain a previous catalog");
    std::cout << "Stage 4 integration tests passed\n";
}
