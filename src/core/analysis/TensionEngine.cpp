#include "core/analysis/TensionEngine.h"
#include "core/analysis/MajorIiVPalette.h"

#include <algorithm>

namespace smartimproviser::harmony
{
namespace
{
int wrap12(int value) noexcept { return (value % 12 + 12) % 12; }

const ImprovisationStrategy* chordAnchors(const ImprovisationResult& result)
{
    for (const auto& strategy : result.strategies)
        if (strategy.ruleId == "core.explicit-chord-tones") return &strategy;
    return nullptr;
}

void addRole(TensionCandidate& candidate, const MaterialNote& note,
             TensionRole role, TensionNoteScope scope = TensionNoteScope::currentChord)
{
    candidate.noteRoles.push_back({note, role, scope});
}

void addAnchors(TensionCandidate& candidate, const ImprovisationStrategy& anchors)
{
    for (const auto& note : anchors.source.notes)
        addRole(candidate, note, note.role == MaterialNoteRole::guideTone
            ? TensionRole::guideTone : TensionRole::chordAnchor);
}

bool compatibleStructure(const NormalizedChord& actual, const NormalizedChord& structure)
{
    if (!actual.valid || !structure.valid) return false;
    for (int interval = 0; interval < 12; ++interval)
    {
        if (!actual.hasTone(interval) || interval == 0) continue;
        const int pitch = wrap12(actual.rootPitchClass + interval);
        if (!structure.hasTone(wrap12(pitch - structure.rootPitchClass))) return false;
    }
    // A slash bass sounds even if the root is omitted in the line.
    return !actual.slashBass
        || structure.hasTone(wrap12(actual.bassPitchClass - structure.rootPitchClass));
}

bool containsWrittenTones(const NormalizedChord& actual, const SourceMaterial& source)
{
    for (int interval = 0; interval < 12; ++interval)
        if (actual.hasTone(interval)
            && std::none_of(source.notes.begin(), source.notes.end(), [&](const auto& note)
            { return note.pitchClass == wrap12(actual.rootPitchClass + interval); }))
            return false;
    return !actual.slashBass
        || std::any_of(source.notes.begin(), source.notes.end(), [&](const auto& note)
        { return note.pitchClass == actual.bassPitchClass; });
}

TensionCandidate makeAnchors(const ImprovisationStrategy& anchors)
{
    TensionCandidate candidate;
    candidate.strategy = anchors;
    auto& strategy = candidate.strategy;
    strategy.tension = TensionLevel::stable;
    strategy.tensionClassified = true;
    strategy.ruleId = "project.t1.explicit-anchors";
    strategy.ruleVersion = 1;
    strategy.idea = "T1: use the written chord tones and available guide tones.";
    strategy.conditions = "The written chord, including any alteration or slash bass, remains the harmony; no scale or tonic is inferred.";
    strategy.usageHint = "T1 chord anchors; connect to actual next-chord targets only when known.";
    addAnchors(candidate, anchors);
    return candidate;
}

TensionCandidate makeNaturalNine(const ImprovisationStrategy& source,
                                  const ImprovisationStrategy& anchors)
{
    TensionCandidate candidate;
    candidate.strategy = source;
    auto& strategy = candidate.strategy;
    strategy.tension = TensionLevel::stable;
    strategy.tensionClassified = true;
    strategy.ruleId = "project.t1.diatonic-nine";
    strategy.ruleVersion = 1;
    strategy.source.name = normalizedChordSymbol(anchors.actualChord) + " + 9";
    strategy.source.mode = DiatonicMode::none; // Five-note subset, not the whole mode.
    const auto full = source.source;
    strategy.source.notes.clear();
    strategy.source.chordRelativeNotes.clear();
    for (std::size_t i = 0; i < full.notes.size(); ++i)
    {
        const auto& note = full.notes[i];
        const auto relative = wrap12(note.pitchClass - anchors.actualChord.rootPitchClass);
        if (!anchors.actualChord.hasTone(relative) && relative != 2
            && note.role != MaterialNoteRole::bassTone) continue;
        strategy.source.notes.push_back(note);
        if (i < full.chordRelativeNotes.size())
            strategy.source.chordRelativeNotes.push_back(full.chordRelativeNotes[i]);
        addRole(candidate, note, (anchors.actualChord.hasTone(relative)
            || note.role == MaterialNoteRole::bassTone)
            ? (note.role == MaterialNoteRole::guideTone
                ? TensionRole::guideTone : TensionRole::chordAnchor)
            : TensionRole::stableExtension);
    }
    if (anchors.actualChord.quality == ChordQuality::major)
        for (const auto& note : full.notes)
            if (wrap12(note.pitchClass - anchors.actualChord.rootPitchClass) == 5
                && !anchors.actualChord.hasTone(5))
                addRole(candidate, note, TensionRole::passingApproach);
    strategy.characteristicNotes.erase(
        std::remove_if(strategy.characteristicNotes.begin(), strategy.characteristicNotes.end(),
            [&](const auto& note)
            {
                return std::none_of(strategy.source.notes.begin(), strategy.source.notes.end(),
                    [&](const auto& present) { return present.pitchClass == note.pitchClass; });
            }), strategy.characteristicNotes.end());
    strategy.sourceTransitions.clear(); // Movements from omitted mode tones are not T1.
    strategy.idea = "T1: written chord anchors with the natural 9 from the confirmed diatonic source.";
    strategy.conditions = "Only the 9 is added; the natural 4th against a major third is passing, never a stable landing note.";
    strategy.usageHint = "Natural 9 is a stable extension; a major chord's 4th is passing only.";
    return candidate;
}

TensionCandidate makeDominantSix(const ImprovisationStrategy& source,
                                  const ImprovisationStrategy& anchors)
{
    TensionCandidate candidate;
    candidate.strategy = source;
    auto& strategy = candidate.strategy;
    strategy.tension = TensionLevel::stable;
    strategy.tensionClassified = true;
    strategy.ruleId = "project.t1.major-V-m6";
    strategy.ruleVersion = 1;
    strategy.source.name = normalizedChordSymbol(strategy.thinkingStructure);
    strategy.source.mode = DiatonicMode::none; // Four-note m6, not full melodic minor.
    const auto full = source.source;
    strategy.source.notes.clear();
    strategy.source.chordRelativeNotes.clear();
    for (std::size_t i = 0; i < full.notes.size(); ++i)
    {
        const auto& note = full.notes[i];
        if (!strategy.thinkingStructure.hasTone(
                wrap12(note.pitchClass - strategy.thinkingStructure.rootPitchClass))) continue;
        strategy.source.notes.push_back(note);
        if (i < full.chordRelativeNotes.size())
            strategy.source.chordRelativeNotes.push_back(full.chordRelativeNotes[i]);
        const auto relative = wrap12(note.pitchClass - anchors.actualChord.rootPitchClass);
        addRole(candidate, note, anchors.actualChord.hasTone(relative)
            ? (relative == 4 || relative == 3 || relative == 10 || relative == 11
                ? TensionRole::guideTone : TensionRole::chordAnchor)
            : TensionRole::stableExtension);
    }
    strategy.characteristicNotes.erase(
        std::remove_if(strategy.characteristicNotes.begin(), strategy.characteristicNotes.end(),
            [&](const auto& note)
            {
                return std::none_of(strategy.source.notes.begin(), strategy.source.notes.end(),
                    [&](const auto& present) { return present.pitchClass == note.pitchClass; });
            }), strategy.characteristicNotes.end());
    strategy.sourceTransitions.clear(); // The full melodic-minor #11 is outside this line.
    // The written dominant root remains an external chord anchor, not an
    // invented member of the four-note m6 arpeggio.
    for (const auto& note : anchors.source.notes)
        if (note.pitchClass == anchors.actualChord.rootPitchClass)
            addRole(candidate, note, TensionRole::chordAnchor);
    // E on Dm6/G7 is an optional step from D toward F. C# (#11) is not T1.
    for (const auto& note : full.notes)
        if (note.semitonesFromRoot == 2
            && !anchors.actualChord.hasTone(wrap12(note.pitchClass - anchors.actualChord.rootPitchClass)))
            addRole(candidate, note, TensionRole::passingApproach);
    strategy.idea = "T1: play the four-note m6 thinking structure over the written dominant.";
    strategy.conditions = "The dominant root and any written tones remain harmonic anchors outside this m6 line. The full melodic-minor scale, especially #11, is not assigned T1.";
    strategy.usageHint = "m6 structure only; optional diatonic passing note is not a stable target.";
    return candidate;
}
}

TensionProfile buildTensionProfile(const ImprovisationResult& result)
{
    TensionProfile profile;
    profile.context = result.context;
    profile.valid = result.valid && result.context.valid;
    if (!profile.valid) return profile;

    for (const auto& strategy : result.strategies)
    {
        // Default TensionLevel::stable is a storage value, not T1 evidence.
        if (!strategy.tensionClassified) continue;
        const auto* band = profile.band(strategy.tension);
        if (!band) continue;
        auto& destination = profile.bands[static_cast<unsigned>(strategy.tension) - 1];
        destination.assessment = TensionAssessment::supported;
        destination.alternatives.push_back({strategy, {}});
    }
    return profile;
}

TensionProfile analyzeStableTension(const ImprovisationResult& result)
{
    auto profile = buildTensionProfile(result);
    if (!profile.valid) return profile;
    const auto* anchors = chordAnchors(result);
    if (!anchors) return profile;

    auto& stable = profile.bands[0];
    const auto& chord = result.context.currentChord;
    const bool confirmedMajorV = chord.quality == ChordQuality::dominant
        && result.dominantContext == DominantContext::toMajor
        && result.context.resolution.confirmed
        && !result.substituteDominant;
    const bool missingMajorV = chord.quality == ChordQuality::dominant
        && hasIncompleteMajorIiVPalette(result.context)
        && !result.substituteDominant;
    const auto& localPattern = result.context.localPattern;
    const bool pendingMajorV = chord.quality == ChordQuality::dominant
        && !result.context.nextChordAvailable && !result.context.resolution.confirmed
        && localPattern.type == HarmonicPatternType::majorIiVI
        && localPattern.positionIndex == 1 && !result.substituteDominant;

    if (confirmedMajorV || missingMajorV || pendingMajorV)
        for (const auto& source : result.strategies)
            if (source.ruleId == "boyko.melodic-minor.V"
                && source.thinkingStructure.valid
                && source.missingTonicApplication == missingMajorV
                && compatibleStructure(chord, source.thinkingStructure))
                stable.alternatives.push_back(makeDominantSix(source, *anchors));

    if (stable.alternatives.empty()
        && (chord.quality == ChordQuality::major || chord.quality == ChordQuality::minor)
        && !chord.hasTone(1)
        && !(chord.quality == ChordQuality::major && chord.hasTone(3)))
        for (const auto& source : result.strategies)
            if (source.kind == ImprovisationStrategyKind::diatonicColor
                && !source.missingTonicApplication
                && (source.source.mode == DiatonicMode::ionian
                    || source.source.mode == DiatonicMode::dorian
                    || source.source.mode == DiatonicMode::aeolian)
                && containsWrittenTones(chord, source.source)
                && std::any_of(source.source.notes.begin(), source.source.notes.end(),
                    [&](const auto& note)
                    { return wrap12(note.pitchClass - chord.rootPitchClass) == 2; }))
                stable.alternatives.push_back(makeNaturalNine(source, *anchors));

    if (stable.alternatives.empty()) stable.alternatives.push_back(makeAnchors(*anchors));
    stable.assessment = TensionAssessment::supported;
    return profile;
}
}
