#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/ContextRanking.h"
#include "core/analysis/DiatonicSources.h"
#include "core/analysis/SpecialSources.h"
#include "core/analysis/HarmonicConcepts.h"
#include <utility>
#include <algorithm>
#include <cstdlib>

namespace smartimproviser::harmony
{
namespace
{
std::vector<MaterialNote> chordMaterial(const NormalizedChord& chord)
{
    std::vector<MaterialNote> notes;
    if (! chord.valid) return notes;
    for (int interval = 0; interval < kPitchClassCount; ++interval)
    {
        if (! chord.hasTone(interval)) continue;
        const int degree = chord.degrees[static_cast<std::size_t>(interval)];
        const bool third = degree == 3 || (degree == 0 &&
            (interval == 4 || (interval == 3 && ! chord.hasTone(4))));
        const bool seventh = degree == 7 || (degree == 0 && (interval == 10 || interval == 11));
        const bool guide = third || seventh;
        const bool characteristic = !guide && interval != 0 && interval != 7;
        notes.push_back({ (chord.rootPitchClass + interval) % kPitchClassCount,
            interval, degree, guide ? MaterialNoteRole::guideTone :
                (characteristic ? MaterialNoteRole::colorTone : MaterialNoteRole::chordTone), characteristic, {} });
    }
    return notes;
}

int nearestDelta(int from, int to)
{
    const int delta = (to - from + 12) % 12;
    return delta > 6 ? delta - 12 : delta;
}

NormalizedChord functionalSubVSpelling(const HarmonicSituation& situation)
{
    auto chord = situation.currentChord;
    if (! chord.valid || ! situation.resolution.available || ! situation.resolution.confirmed
        || ! situation.resolution.targetChord.valid)
        return chord;

    // A confirmed tritone-substitute dominant is the bII7 of its real target.
    // Derive notation from that target rather than from a host-canonicalized
    // C#/Db pitch name. This changes spelling only; pitch classes stay intact.
    const auto candidateRootFifths = situation.resolution.targetChord.rootFifths - 5;
    if (circleOfFifthsToPitchClass(candidateRootFifths) != chord.rootPitchClass)
        return chord;

    const bool bassIsRoot = chord.bassPitchClass == chord.rootPitchClass;
    chord.rootFifths = candidateRootFifths;
    if (bassIsRoot)
        chord.bassFifths = candidateRootFifths;
    chord.slashBass = chord.bassPitchClass != chord.rootPitchClass;
    return chord;
}

void addTargets(ImprovisationStrategy& strategy, const HarmonicSituation& situation)
{
    if (! situation.nextChordAvailable || ! situation.nextChord.valid) return;
    strategy.nextChord = situation.nextChord;
    strategy.targetNotes = chordMaterial(situation.nextChord);
    // Functional tendency moves remain exactly those established by Stage 2.
    if (strategy.resolution.confirmed) return;
    // For other transitions, propose nearest root/guide targets. These are
    // optional melodic connections, not a claim of a dominant resolution.
    const auto& starts = strategy.guideNotes.empty() ? strategy.source.notes : strategy.guideNotes;
    for (const auto& from : starts)
    {
        const MaterialNote* best = nullptr;
        int distance = 13;
        for (const auto& to : strategy.targetNotes)
        {
            if (to.semitonesFromRoot != 0 && to.role != MaterialNoteRole::guideTone) continue;
            const auto candidate = std::abs(nearestDelta(from.pitchClass, to.pitchClass));
            if (candidate < distance) { best = &to; distance = candidate; }
        }
        if (best != nullptr)
            strategy.suggestedTransitions.push_back({from.pitchClass, best->pitchClass,
                nearestDelta(from.pitchClass, best->pitchClass), ResolutionImportance::optional});
    }
}
}

ImprovisationResult analyzeImprovisation(const HarmonicSituation& situation)
{
    ImprovisationResult result;
    result.context = situation;
    if (! situation.valid || ! situation.currentChord.valid)
    {
        result.unavailableReason = "Waiting for valid position, chord and key context.";
        return result;
    }

    const HarmonicAnalysis* harmonic = nullptr;
    const auto index = situation.primaryInterpretationIndex;
    if (index >= 0 && index < situation.interpretationCount
        && static_cast<std::size_t>(index) < situation.interpretations.size()
        && situation.interpretations[static_cast<std::size_t>(index)].valid)
        harmonic = &situation.interpretations[static_cast<std::size_t>(index)].harmonic;

    // No fallback to global interpretation when Stage 2 left primary unresolved.
    // Secondary is relative to the global key even when a local V is primary.
    result.secondaryDominant = situation.harmonic.appliedDominantConfirmed;
    result.substituteDominant = harmonic != nullptr && harmonic->substituteDominantConfirmed;
    const auto& chord = situation.currentChord;
    const auto displayChord = result.substituteDominant ? functionalSubVSpelling(situation) : chord;
    const bool dominant = chord.quality == ChordQuality::dominant
        || (harmonic != nullptr
            && (harmonic->effectiveFunction == HarmonicFunction::dominant
                || harmonic->effectiveFunction == HarmonicFunction::substituteDominant));
    result.contextDescription = normalizedChordSymbol(displayChord);
    if (dominant)
    {
        result.dominantContext = DominantContext::unresolved;
        if (situation.resolution.available && situation.resolution.confirmed
            && situation.resolution.targetChord.valid)
        {
            switch (situation.resolution.targetQuality)
            {
                case ChordQuality::major: result.dominantContext = DominantContext::toMajor; break;
                case ChordQuality::minor: result.dominantContext = DominantContext::toMinor; break;
                case ChordQuality::dominant: result.dominantContext = DominantContext::toDominant; break;
                default: result.dominantContext = DominantContext::toOther; break;
            }
            result.contextDescription += " -> " + normalizedChordSymbol(situation.resolution.targetChord);
        }
        switch (result.dominantContext)
        {
            case DominantContext::toMajor: result.contextDescription += " | dominant to major"; break;
            case DominantContext::toMinor: result.contextDescription += " | dominant to minor"; break;
            case DominantContext::toDominant: result.contextDescription += " | dominant chain"; break;
            case DominantContext::toOther: result.contextDescription += " | other target"; break;
            default: result.contextDescription += " | resolution unconfirmed"; break;
        }
        if (result.secondaryDominant) result.contextDescription += " | secondary";
        if (result.substituteDominant) result.contextDescription += " | SubV";
    }
    if (harmonic == nullptr)
        result.contextDescription += " | interpretation unresolved";

    ImprovisationStrategy strategy;
    strategy.kind = ImprovisationStrategyKind::chordToneBased;
    strategy.ruleId = "core.explicit-chord-tones";
    strategy.priority = 100;
    strategy.interpretationIndependent = true;
    strategy.interpretationIndex = -1;
    strategy.evidence = situation.evidence;
    strategy.actualChord = displayChord;
    strategy.thinkingStructure = displayChord;
    strategy.source.kind = MaterialKind::chordTones;
    strategy.source.rootFifths = displayChord.rootFifths;
    strategy.source.rootPitchClass = chord.rootPitchClass;
    strategy.source.name = normalizedChordSymbol(displayChord);
    strategy.source.notes = chordMaterial(chord);
    for (const auto& note : strategy.source.notes)
    {
        if (note.role == MaterialNoteRole::guideTone) strategy.guideNotes.push_back(note);
        if (note.characteristic) strategy.characteristicNotes.push_back(note);
    }
    if (strategy.source.notes.empty())
    {
        result.unavailableReason = "No explicit chord tones available.";
        return result;
    }
    strategy.resolution = situation.resolution;
    strategy.ruleVersion = 3;
    addTargets(strategy, situation);
    strategy.idea = "Use chord tones; connect available thirds and sevenths to the next harmony.";
    strategy.explanation = "Guide tones and colors are drawn only from explicit chord tones.";
    strategy.conditions = "Confirmed moves come from harmonic analysis; suggested moves are optional.";
    result.strategies.push_back(std::move(strategy));
    result.valid = true;

    // Collect every admissible interpretation-specific source before sorting.
    // The ranking layer changes presentation order only; it never changes
    // harmonic evidence or chooses a primary interpretation on behalf of Stage 2.
    addDiatonicSource(result);
    addSpecialSources(result);
    rankImprovisationStrategies(result);
    addHarmonicConcepts(result);
    return result;
}
}
