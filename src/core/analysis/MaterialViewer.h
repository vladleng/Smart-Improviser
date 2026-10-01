#pragma once

#include "core/analysis/Explanation.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

namespace smartimproviser::harmony
{
// A presentation projection only: no key detection, ranking or tension policy.
// The pitch classes and roles come from the selected Core result. The octave is
// deliberately left to the drawing surface, since this source has no register.
enum class ViewerLayer { all, source, chord, guides, characteristic, targets };
enum ViewerRole : unsigned
{
    sourceRole = 1u, chordRole = 2u, guideRole = 4u,
    characteristicRole = 8u, targetRole = 16u
};

struct ViewerNote
{
    int pitchClass = -1;
    int sourceInterval = -1;
    std::string spelling;
    unsigned roles = 0;
    std::string chordSpelling; // Same pitch in the actual-chord context, when Core supplies it.
};

struct MaterialView
{
    bool valid = false;
    std::size_t itemIndex = 0;
    std::string sourceName;
    int sourceRootPitchClass = -1;
    NormalizedChord chord;
    NormalizedChord nextChord;
    std::vector<int> interpretationIndices;
    bool interpretationIndependent = false;
    std::vector<ViewerNote> current;
    std::vector<ViewerNote> targets;
};

inline bool visibleInLayer(const ViewerNote& note, ViewerLayer layer) noexcept
{
    switch (layer)
    {
        case ViewerLayer::all: return true;
        case ViewerLayer::source: return (note.roles & sourceRole) != 0;
        case ViewerLayer::chord: return (note.roles & chordRole) != 0;
        case ViewerLayer::guides: return (note.roles & guideRole) != 0;
        case ViewerLayer::characteristic: return (note.roles & characteristicRole) != 0;
        case ViewerLayer::targets: return (note.roles & targetRole) != 0;
    }
    return false;
}

// Fretboard-only labels relative to the actual written chord. Explicit host
// degrees win: the same pitch can be #5 or b13 without rewriting the chord.
inline std::string chordDegreeLabel(const NormalizedChord& chord, int pitchClass)
{
    if (!chord.valid || pitchClass < 0 || pitchClass >= kPitchClassCount) return {};
    const int interval = (pitchClass - chord.rootPitchClass + 12) % 12;
    int degree = chord.hasTone(interval) ? chord.degrees[static_cast<std::size_t>(interval)] : 0;
    if (degree <= 0 || degree > 13)
    {
        static constexpr int defaultDegrees[12] = {1,9,9,3,3,11,5,5,13,13,7,7};
        degree = defaultDegrees[interval];
        if (interval == 3 && chord.hasTone(4)) degree = 9; // dominant #9, not minor 3
        if (interval == 6 && chord.hasTone(7)) degree = 11;
        if (interval == 8 && !chord.hasTone(7) && chord.hasTone(4)) degree = 5;
    }
    int natural = 0;
    switch (degree)
    {
        case 1: natural = 0; break;
        case 2: case 9: natural = 2; break;
        case 3: natural = 4; break;
        case 4: case 11: natural = 5; break;
        case 5: natural = 7; break;
        case 6: case 13: natural = 9; break;
        case 7: natural = 11; break;
        default: return {};
    }
    int alteration = (interval - natural + 12) % 12;
    if (alteration > 6) alteration -= 12;
    return std::string(static_cast<std::size_t>(alteration < 0 ? -alteration : alteration),
                       alteration < 0 ? 'b' : '#') + std::to_string(degree);
}

inline std::string spelledChordNote(const NormalizedChord& chord, const MaterialNote& note)
{
    if (! note.spelling.empty())
        return note.spelling;
    // This only spells an already supplied pitch/degree; it cannot add notes.
    static constexpr char letters[] = "CDEFGAB";
    static constexpr int naturals[] = {0, 2, 4, 5, 7, 9, 11};
    const auto wrap = [](int value, int modulus) { return (value % modulus + modulus) % modulus; };
    // Unknown degree: use an enharmonic pitch-class label rather than assert
    // a functional interval that Core did not provide.
    int degree = note.degree;
    if (degree <= 0)
    {
        // Existing chord-relative spelling convention from HarmonicConcepts:
        // infer a letter for display only when the host supplies no degree.
        static constexpr int displayDegrees[] = {1, 9, 9, 3, 3, 11, 5, 5, 13, 13, 7, 7};
        const int interval = wrap(note.pitchClass - chord.rootPitchClass, 12);
        degree = displayDegrees[interval];
        if (interval == 3 && chord.hasTone(4)) degree = 9;
        if (interval == 6 && chord.hasTone(7)) degree = 11;
        if (interval == 8 && chord.hasTone(4) && ! chord.hasTone(7)) degree = 5;
    }
    const int letter = wrap(4 * wrap(chord.rootFifths, 7) + degree - 1, 7);
    int accidental = wrap(note.pitchClass - naturals[letter], 12);
    if (accidental > 6) accidental -= 12;
    return std::string(1, letters[letter])
        + std::string(static_cast<std::size_t>(std::abs(accidental)), accidental < 0 ? 'b' : '#');
}

inline MaterialView buildMaterialView(const ImprovisationResult& result,
                                      const ExplanationResult& explanation,
                                      std::size_t itemIndex)
{
    MaterialView view;
    if (! result.valid || ! explanation.valid || itemIndex >= explanation.items.size())
        return view;
    const auto& item = explanation.items[itemIndex];
    if (item.strategyIndices.empty() || item.strategyIndices.front() >= result.strategies.size())
        return view;

    view.valid = true;
    view.itemIndex = itemIndex;
    view.sourceName = item.source.name;
    view.sourceRootPitchClass = item.source.rootPitchClass;
    view.chord = item.actualChord;
    view.nextChord = item.targetChord;
    view.interpretationIndices = item.interpretationIndices;
    view.interpretationIndependent = item.interpretationIndependent;
    const auto& strategy = result.strategies[item.strategyIndices.front()];

    const auto hasPitch = [](const std::vector<MaterialNote>& notes, int pitch)
    {
        return std::any_of(notes.begin(), notes.end(), [pitch](const auto& n)
        { return n.pitchClass == pitch; });
    };
    const auto append = [&](const MaterialNote& note, unsigned roles)
    {
        if (note.pitchClass < 0 || note.pitchClass >= 12) return;
        auto found = std::find_if(view.current.begin(), view.current.end(), [&](const auto& n)
        { return n.pitchClass == note.pitchClass; });
        if (found != view.current.end())
        {
            found->roles |= roles;
            return;
        }
        const auto chordNote = std::find_if(item.source.chordRelativeNotes.begin(),
                                            item.source.chordRelativeNotes.end(),
            [&](const auto& n) { return n.pitchClass == note.pitchClass; });
        const auto chordSpelling = chordNote == item.source.chordRelativeNotes.end()
            ? spelledChordNote(view.chord, note)
            : spelledChordNote(view.chord, *chordNote);
        view.current.push_back({note.pitchClass, note.semitonesFromRoot,
                                spelledChordNote(view.chord, note), roles, chordSpelling});
    };

    for (const auto& note : item.source.notes)
    {
        unsigned roles = sourceRole;
        if (view.chord.valid && view.chord.hasTone((note.pitchClass - view.chord.rootPitchClass + 12) % 12))
            roles |= chordRole;
        if (hasPitch(strategy.guideNotes, note.pitchClass)) roles |= guideRole;
        if (hasPitch(strategy.characteristicNotes, note.pitchClass)) roles |= characteristicRole;
        append(note, roles);
    }
    for (const auto& note : strategy.guideNotes)
        append(note, guideRole | chordRole);
    for (const auto& note : strategy.characteristicNotes)
        append(note, characteristicRole
            | (view.chord.valid && view.chord.hasTone((note.pitchClass - view.chord.rootPitchClass + 12) % 12)
                ? chordRole : 0u));

    // A T1 m6 line omits the dominant root by design. Keep every written
    // chord anchor available in the chord/all layers without adding it to the
    // selected four-note source. The spelling comes from Core's anchor source.
    if (strategy.tensionClassified)
        for (const auto& original : result.strategies)
            if (original.ruleId == "core.explicit-chord-tones")
            {
                for (auto note : original.source.notes)
                {
                    note.semitonesFromRoot = (note.pitchClass - view.sourceRootPitchClass + 12) % 12;
                    append(note, chordRole | (note.role == MaterialNoteRole::guideTone ? guideRole : 0u));
                }
                break;
            }
    if (strategy.tensionClassified)
        std::stable_sort(view.current.begin(), view.current.end(), [](const auto& a, const auto& b)
        { return a.sourceInterval < b.sourceInterval; });

    // Targets belong to the actual next chord, never the inferred tonic.
    for (const auto& note : item.targetNotes)
        if (note.pitchClass >= 0 && note.pitchClass < 12)
            view.targets.push_back({note.pitchClass, note.semitonesFromRoot,
                                    spelledChordNote(view.nextChord, note), targetRole});
    return view;
}
}
