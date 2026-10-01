#include "core/analysis/MaterialViewer.h"

#include <cstdlib>
#include <iostream>

using namespace smartimproviser::harmony;

namespace
{
void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

const ViewerNote* find(const std::vector<ViewerNote>& notes, int pc)
{
    for (const auto& note : notes)
        if (note.pitchClass == pc) return &note;
    return nullptr;
}
}

int main()
{
    ImprovisationResult result;
    result.valid = true;
    result.context.valid = true;
    result.context.primaryInterpretationIndex = -1;
    result.context.interpretationCount = 2;

    ImprovisationStrategy anchor;
    anchor.interpretationIndependent = true;
    anchor.source.kind = MaterialKind::chordTones;
    anchor.source.name = "Db anchors";
    anchor.actualChord.valid = true;
    anchor.actualChord.rootFifths = -5;
    anchor.actualChord.rootPitchClass = 1;
    anchor.actualChord.tones[0] = true;
    anchor.source.notes.push_back({1, 0, 1, MaterialNoteRole::chordTone, false, "Db"});
    anchor.guideNotes.push_back({5, 4, 3, MaterialNoteRole::guideTone, false, "F"});
    anchor.nextChord.valid = true;
    anchor.nextChord.rootFifths = -4;
    anchor.nextChord.rootPitchClass = 8;
    anchor.targetNotes.push_back({8, 0, 1, MaterialNoteRole::chordTone, false, "Ab"});
    result.strategies.push_back(anchor);

    auto scale = anchor;
    scale.interpretationIndependent = false;
    scale.interpretationIndex = 0;
    scale.source.kind = MaterialKind::scale;
    scale.source.rootPitchClass = 1;
    scale.source.name = "Db source";
    scale.source.notes.push_back({3, 2, 2, MaterialNoteRole::scaleTone, false, "Eb"});
    scale.source.notes.push_back({5, 4, 3, MaterialNoteRole::guideTone, false, "F"});
    scale.characteristicNotes.push_back({3, 2, 2, MaterialNoteRole::colorTone, true, "Eb"});
    result.strategies.push_back(scale);

    auto alternative = scale;
    alternative.interpretationIndex = 1;
    alternative.source.name = "C# source";
    alternative.source.notes.front().spelling = "C#";
    alternative.source.notes[1].spelling = "D#";
    result.strategies.push_back(alternative);

    const auto explanation = buildExplanation(result);
    expect(explanation.valid && explanation.items.size() == 3,
           "enharmonic alternatives stay distinct in the explanation");
    const auto anchors = buildMaterialView(result, explanation, 0);
    expect(anchors.interpretationIndependent && anchors.interpretationIndices.empty(),
           "unresolved primary defaults to explicitly independent anchors");
    expect(find(anchors.current, 1)->spelling == "Db", "Db spelling survives projection");
    expect(find(anchors.current, 5)->roles & guideRole, "guide outside source is visible");
    expect(! visibleInLayer(*find(anchors.current, 5), ViewerLayer::source),
           "source layer excludes out-of-source guide");
    expect(find(anchors.targets, 8)->spelling == "Ab", "next chord target spelling survives");
    expect(! visibleInLayer(*find(anchors.targets, 8), ViewerLayer::chord),
           "target is not mislabelled as current chord tone");

    const auto db = buildMaterialView(result, explanation, 1);
    const auto cs = buildMaterialView(result, explanation, 2);
    expect(db.interpretationIndices == std::vector<int>{0}
           && cs.interpretationIndices == std::vector<int>{1},
           "each alternative retains its own provenance");
    expect(find(db.current, 3)->spelling == "Eb"
           && find(cs.current, 3)->spelling == "D#",
           "selected source preserves distinct enharmonic notation");
    expect((find(db.current, 3)->roles & characteristicRole) != 0,
           "characteristic role comes from the selected strategy");
    expect(! buildMaterialView(result, explanation, 100).valid,
           "out-of-range selection cannot display stale material");

    auto alteredFifth = anchor.actualChord;
    alteredFifth.rootFifths = 1; // G
    alteredFifth.rootPitchClass = 7;
    alteredFifth.tones[8] = true;
    alteredFifth.degrees[8] = 5;
    expect(chordDegreeLabel(alteredFifth, 3) == "#5",
           "written G7#5 labels D# as #5");
    alteredFifth.degrees[8] = 13;
    expect(chordDegreeLabel(alteredFifth, 3) == "b13",
           "written G7b13 labels Eb as b13 at the same pitch class");
    alteredFifth.tones[8] = false;
    alteredFifth.degrees[8] = 0;
    alteredFifth.tones[4] = true;
    expect(chordDegreeLabel(alteredFifth, 10) == "#9",
           "optional Bb over G7 uses dominant #9 label");

    // Identical source notes with different important-tone roles may not be
    // collapsed into one viewer item that silently inherits the first role set.
    auto distinctRole = scale;
    distinctRole.guideNotes.clear();
    distinctRole.characteristicNotes.clear();
    result.strategies.push_back(distinctRole);
    expect(buildExplanation(result).items.size() == 4,
           "different guide/characteristic roles retain separate material views");

    // A melodic-minor source can legitimately call pc 3 Eb while the C7 #9
    // application calls that same pitch D#. Keep both Core spellings visible.
    ImprovisationResult altered;
    altered.valid = true;
    ImprovisationStrategy c7;
    c7.actualChord.valid = true;
    c7.actualChord.rootFifths = 0;
    c7.actualChord.rootPitchClass = 0;
    c7.actualChord.tones[4] = true;
    c7.source.kind = MaterialKind::scale;
    c7.source.rootPitchClass = 1;
    c7.source.name = "Db melodic minor";
    c7.source.notes.push_back({3, 2, 2, MaterialNoteRole::colorTone, true, "Eb"});
    c7.source.chordRelativeNotes.push_back({3, 3, 9, MaterialNoteRole::colorTone, true, "D#"});
    c7.characteristicNotes.push_back(c7.source.chordRelativeNotes.front());
    altered.strategies.push_back(c7);
    const auto alteredView = buildMaterialView(altered, buildExplanation(altered), 0);
    const auto* color = find(alteredView.current, 3);
    expect(color && color->spelling == "Eb" && color->chordSpelling == "D#",
           "source Eb and C7 #9 D# remain explicit aliases");
    MaterialNote unspelledGuide {4, 4, 0, MaterialNoteRole::guideTone, false, {}};
    expect(spelledChordNote(c7.actualChord, unspelledGuide) == "E",
           "unspelled C7 third uses a note name rather than pitch-class number");

    std::cout << "Material viewer projection: passed\n";
}
