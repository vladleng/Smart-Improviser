#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/MaterialViewer.h"
#include "core/analysis/MaterialSelection.h"
#include "core/analysis/TensionEngine.h"
#include <cstdlib>
#include <iostream>
using namespace smartimproviser::harmony;
namespace {
void expect(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
ChordContext chord(int root, std::initializer_list<int> tones, int bass = 999) {
    ChordContext c; c.available = c.defined = true; c.root = root;
    c.bass = bass == 999 ? root : bass;
    for (int n : tones) c.intervals.values[n] = 0xFF;
    return c;
}
ImprovisationResult analyze(ChordContext current, ChordContext next, int key = 0) {
    TimelineHarmonicSnapshot t; t.positionAvailable = true; t.ppq = 4;
    t.globalKey.available = t.globalKey.defined = true; t.globalKey.root = key;
    for (int n : {0,2,4,5,7,9,11}) t.globalKey.intervals.values[n] = 0xFF;
    t.currentChord = current; t.nextChordAvailable = true; t.nextChord = next;
    auto r = analyzeImprovisation(analyzeHarmonicSituation(t));
    const auto profile = analyzeStableTension(r);
    for (const auto& candidate : profile.bands[0].alternatives) r.strategies.push_back(candidate.strategy);
    return r;
}
bool has(const std::vector<ViewerNote>& notes, int pc, unsigned role) {
    for (const auto& n : notes) if (n.pitchClass == pc && (n.roles & role)) return true;
    return false;
}
}
int main() {
    // Every transposition preserves the written perfect fifth even when the
    // selected altered source deliberately omits it from the solo line.
    for (int key = -5; key <= 6; ++key) {
        auto r = analyze(chord(key+1,{0,4,7,10}),chord(key,{0,4,7,11}),key);
        auto e = buildExplanation(r); int parent = -1; bool testedAltered = false;
        for (std::size_t i = 0; i < e.items.size(); ++i) {
            const auto projected = buildMaterialView(r,e,i);
            for (const auto& n : projected.current)
                expect((projected.sourceRootPitchClass+n.sourceInterval)%12==n.pitchClass,
                       "external chord anchor keeps correct staff pitch relative to source root");
            if (e.items[i].sourceRuleId == "boyko.melodic-minor.bII") {
                auto v = buildMaterialView(r,e,i);
                const int fifth = (r.context.currentChord.rootPitchClass+7)%12;
                expect(has(v.current,fifth,chordRole),"altered line retains written fifth in chord layer");
                expect(!has(v.current,fifth,sourceRole),"omitted fifth is not inserted into altered source");
                testedAltered = true;
            }
            if (e.items[i].sourceRuleId == "boyko.melodic-minor.V") parent = static_cast<int>(i);
        }
        expect(testedAltered,"confirmed V has an altered source");
        const int subset = stableSubsetIndex(e,parent);
        expect(subset >= 0,"full melodic minor links to its explicit T1 subset");
        expect(!compactMaterialHidden(e,parent) && compactMaterialHidden(e,subset),"one compact row keeps both source forms accessible");
        expect(e.items[parent].source.notes.size()==7 && e.items[subset].source.notes.size()==4,"switching forms preserves seven versus four notes");
        expect(e.items[subset].tensionClassified && !e.items[parent].tensionClassified,"full source cannot inherit subset classification");
    }
    for (auto c : {chord(1,{0,5,7,10}),chord(2,{0,7})}) {
        auto r = analyze(c,chord(0,{0,4,7,11})); auto e = buildExplanation(r);
        int rows = 0; for (std::size_t i=0;i<e.items.size();++i) if (!compactMaterialHidden(e,i)) ++rows;
        expect(rows==0,"unsupported sus/power source does not invent a playing option");
        const int fallback = literalChordIndex(e);
        expect(fallback>=0 && buildMaterialView(r,e,fallback).valid,"empty source list still displays literal chord");
    }
    auto slash = analyze(chord(0,{0,4,7,11},6),chord(1,{0,4,7,10}));
    auto e = buildExplanation(slash);
    auto v = buildMaterialView(slash,e,literalChordIndex(e));
    expect(has(v.current,6,bassRole) && has(v.current,6,chordRole),"non-chord slash bass appears as a separate accompaniment anchor");
    expect(!slash.context.currentChord.hasTone(6),"displaying bass does not rewrite written chord degrees");
    auto targetBass = analyze(chord(1,{0,4,7,10}),chord(0,{0,4,7,11},6));
    bool bassTarget = false;
    for (const auto& n : targetBass.strategies.front().targetNotes) if (n.pitchClass==6 && n.role==MaterialNoteRole::bassTone && n.spelling=="F#") bassTarget=true;
    expect(bassTarget,"actual next chord retains its separately spelled slash bass");
    auto borrowed = analyze(chord(-1,{0,3,7,10}),chord(1,{0,4,7,10}));
    auto merged = buildExplanation(borrowed); int count=0;
    for (const auto& item : merged.items) if (item.sourceRuleId=="boyko.melodic-minor.root") {
        ++count; expect(item.interpretationIndices.size()==2,"one source retains both interpretations");
        expect(item.applicationConditions.size()==2,"merged source retains conditions of each interpretation");
    }
    expect(count==1,"identical melodic minor is shown once across interpretations");
    std::cout << "Material regression tests passed\n";
}
