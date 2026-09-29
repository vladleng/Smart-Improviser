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
    const int degree = note.degree > 0 ? (note.degree - 1) % 7 :
        (note.semitonesFromRoot == 0 ? 0 : -1);
    if (degree < 0)
    {
        static constexpr const char* names[] =
            {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
        return names[wrap(note.pitchClass, 12)];
    }
    const int letter = wrap(4 * wrap(chord.rootFifths, 7) + degree, 7);
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
        view.current.push_back({note.pitchClass, note.semitonesFromRoot,
                                spelledChordNote(view.chord, note), roles});
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

    // Targets belong to the actual next chord, never the inferred tonic.
    for (const auto& note : item.targetNotes)
        if (note.pitchClass >= 0 && note.pitchClass < 12)
            view.targets.push_back({note.pitchClass, note.semitonesFromRoot,
                                    spelledChordNote(view.nextChord, note), targetRole});
    return view;
}
}
