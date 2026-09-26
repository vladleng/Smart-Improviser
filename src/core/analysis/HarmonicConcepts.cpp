#include "core/analysis/HarmonicConcepts.h"
#include "core/analysis/ExplanationText.h"
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

namespace smartimproviser::harmony
{
namespace
{
int wrap(int value, int modulus = 12) { return (value % modulus + modulus) % modulus; }
std::string spell(int fifths, int degree, int pitch)
{
    constexpr const char* names[] = {"C","D","E","F","G","A","B"};
    constexpr int natural[] = {0,2,4,5,7,9,11};
    const int letter = wrap(4 * wrap(fifths, 7) + degree - 1, 7);
    int accidental = wrap(pitch - natural[letter]);
    if (accidental > 6) accidental -= 12;
    return std::string(names[letter]) + std::string(static_cast<std::size_t>(std::abs(accidental)), accidental < 0 ? 'b' : '#');
}
MaterialNote spelled(MaterialNote note, const NormalizedChord& chord)
{
    if (!note.degree)
    {
        constexpr int degrees[] = {1,9,9,3,3,11,5,5,13,13,7,7};
        note.degree = degrees[static_cast<std::size_t>(wrap(note.semitonesFromRoot))];
        if (note.semitonesFromRoot == 3 && chord.hasTone(4)) note.degree = 9;
        if (note.semitonesFromRoot == 6 && chord.hasTone(7)) note.degree = 11;
        if (note.semitonesFromRoot == 8 && chord.hasTone(4) && !chord.hasTone(7)) note.degree = 5;
    }
    note.spelling = spell(chord.rootFifths, note.degree, note.pitchClass);
    return note;
}
HarmonicConcept makeConcept(const ImprovisationStrategy& strategy, HarmonicConceptKind kind,
                            const char* id, std::string title)
{
    HarmonicConcept idea;
    idea.kind = kind;
    idea.ruleId = id;
    idea.sourceRuleId = strategy.ruleId;
    idea.interpretationIndex = strategy.interpretationIndex;
    idea.interpretationIndependent = strategy.interpretationIndependent;
    idea.evidence = strategy.evidence;
    idea.actualChord = strategy.actualChord;
    idea.title = std::move(title);
    idea.sourceReference = "Project instruction derived from " + strategy.ruleId;
    return idea;
}
bool containsPitch(const std::vector<MaterialNote>& notes, int pitch)
{
    return std::any_of(notes.begin(), notes.end(), [pitch](const auto& n) { return n.pitchClass == pitch; });
}
void addFoundationIdeas(ImprovisationResult& result)
{
    const auto& foundation = result.strategies.front();
    auto anchors = makeConcept(foundation, HarmonicConceptKind::chordAnchors,
        "concept.chord-anchors", "Chord-tone playing");
    anchors.instruction = "Build the line around the actual chord tones; connect the available thirds and sevenths.";
    anchors.conditions = "Explicit colors remain colors; the list does not make every chord tone equally stable.";
    for (const auto& note : foundation.source.notes) anchors.material.push_back(spelled(note, foundation.actualChord));
    result.concepts.push_back(std::move(anchors));

    if (foundation.guideNotes.empty()) return;
    auto guide = makeConcept(foundation, HarmonicConceptKind::guideTargeting,
        "concept.guide-targeting", "Guide-tone targeting");
    for (const auto& note : foundation.guideNotes) guide.material.push_back(spelled(note, foundation.actualChord));
    if (foundation.resolution.available && foundation.resolution.confirmed)
    {
        // Use only actual guides landing in the actual next chord.
        for (std::size_t i = 0; i < foundation.resolution.moveCount && i < foundation.resolution.moves.size(); ++i)
        {
            const auto& move = foundation.resolution.moves[i];
            if (containsPitch(foundation.guideNotes, move.fromPitchClass) && containsPitch(foundation.targetNotes, move.toPitchClass))
                guide.moves.push_back(move);
        }
        guide.movesAreConfirmed = !guide.moves.empty();
    }
    else
    {
        for (const auto& move : foundation.suggestedTransitions)
            if (containsPitch(foundation.guideNotes, move.fromPitchClass) && containsPitch(foundation.targetNotes, move.toPitchClass))
                guide.moves.push_back(move);
    }
    guide.targetChord = foundation.nextChord;
    guide.targetScope = guide.moves.empty() ? ConceptTargetScope::none : ConceptTargetScope::nextChord;
    guide.instruction = guide.moves.empty() ? "Emphasize the available thirds/sevenths; no next-chord move is asserted."
        : guide.movesAreConfirmed ? "Connect these guides using the confirmed harmonic resolution."
                                  : "Try these melodic connections to the next chord.";
    guide.conditions = "Only present guides are used; optional melodic moves do not establish harmonic function.";
    result.concepts.push_back(std::move(guide));
}
void addSourceIdeas(ImprovisationResult& result)
{
    for (const auto& strategy : result.strategies)
    {
        if (strategy.source.kind != MaterialKind::scale) continue;
        if (strategy.source.mode != DiatonicMode::none)
        {
            auto extensions = makeConcept(strategy, HarmonicConceptKind::diatonicExtensions,
                "concept.diatonic-extensions", "Diatonic colors from " + strategy.source.name);
            for (auto note : strategy.source.chordRelativeNotes)
            {
                const int simple = (note.degree - 1) % 7 + 1;
                if (simple != 2 && simple != 4 && simple != 6) continue;
                note.degree = simple + 7; // Available 9/11/13, including contextual alterations.
                const bool passing = (note.semitonesFromRoot == 5 && strategy.actualChord.hasTone(4))
                    || (note.semitonesFromRoot == 8 && strategy.actualChord.hasTone(7));
                note.role = passing ? MaterialNoteRole::passingTone : MaterialNoteRole::colorTone;
                note.characteristic = true;
                extensions.material.push_back(note);
            }
            if (!extensions.material.empty())
            {
                extensions.instruction = "Use these source colors between chord anchors, preserving the selected harmonic context.";
                extensions.conditions = "Natural 11 against major 3 and b13 against natural 5 are passing colors here, not default landing notes.";
                result.concepts.push_back(std::move(extensions));
            }
        }
        // m6 is already an approved thinking structure from the 0.3d catalog.
        if (!strategy.thinkingStructure.valid || strategy.thinkingStructure.quality != ChordQuality::minor
            || !strategy.thinkingStructure.hasExtension(ChordExtension::sixth)) continue;
        auto skeleton = makeConcept(strategy, HarmonicConceptKind::thinkingArpeggio,
            "concept.minor-sixth-skeleton", "Think " + normalizedChordSymbol(strategy.thinkingStructure)
                + " over " + normalizedChordSymbol(strategy.actualChord));
        for (const auto& note : strategy.source.chordRelativeNotes)
        {
            const int relative = wrap(note.pitchClass - strategy.thinkingStructure.rootPitchClass);
            if (strategy.thinkingStructure.hasTone(relative)) skeleton.material.push_back(note);
        }
        if (skeleton.material.size() != 4) continue; // No synthetic missing source note.
        skeleton.sourceReference = strategy.sourceReference + "; m6 skeleton instruction: project model";
        skeleton.instruction = "Use the four-note structure as a melodic skeleton; connect it with " + strategy.source.name + ".";
        skeleton.conditions = strategy.usageHint + " These notes are shown relative to the actual chord; the full source remains available.";
        for (const auto& move : strategy.sourceTransitions)
            if (containsPitch(skeleton.material, move.fromPitchClass)) skeleton.moves.push_back(move);
        skeleton.targetChord = strategy.nextChord;
        skeleton.targetScope = skeleton.moves.empty() ? ConceptTargetScope::none : ConceptTargetScope::nextChord;
        result.concepts.push_back(std::move(skeleton));
    }
}
void addApproachIdeas(ImprovisationResult& result)
{
    const auto& foundation = result.strategies.front();
    const bool next = foundation.nextChord.valid && !foundation.targetNotes.empty();
    const auto& candidates = next ? foundation.targetNotes : foundation.source.notes;
    if (candidates.empty()) return;
    const auto& chord = next ? foundation.nextChord : foundation.actualChord;
    // Deterministic preference: actual third, then another guide, then root.
    const MaterialNote* target = nullptr;
    for (const auto& note : candidates)
        if (note.role == MaterialNoteRole::guideTone && (note.degree == 3
            || (!note.degree && (note.semitonesFromRoot == 3 || note.semitonesFromRoot == 4)))) { target = &note; break; }
    if (!target)
        for (const auto& note : candidates)
            if (note.role == MaterialNoteRole::guideTone) { target = &note; break; }
    if (!target)
        for (const auto& note : candidates)
            if (note.semitonesFromRoot == 0) { target = &note; break; }
    if (!target) return;

    for (const bool enclosure : {false, true})
    {
        auto idea = makeConcept(foundation, enclosure ? HarmonicConceptKind::enclosure : HarmonicConceptKind::chromaticApproach,
            enclosure ? "concept.chromatic-enclosure" : "concept.chromatic-approach-below",
            enclosure ? "Chromatic enclosure" : "Chromatic approach");
        idea.target = spelled(*target, chord);
        idea.targetChord = chord;
        idea.targetScope = next ? ConceptTargetScope::nextChord : ConceptTargetScope::currentChord;
        idea.instruction = enclosure ? "Above (+1 semitone), below (-1), then target (0)."
                                     : "Below (-1 semitone), then target (0).";
        idea.conditions = next ? "Prepare over the current chord; land when the next chord sounds. Choose rhythm and register yourself."
                               : "Resolve within the current chord. Choose rhythm and register yourself.";
        idea.conditions += " Approach notes are passing; chord membership does not guarantee stability. No Phrase or MIDI is generated.";
        idea.sourceReference = "Project chromatic approach/enclosure template; not a transcribed Boyko phrase";
        const std::vector<int> offsets = enclosure ? std::vector<int>{1,-1,0} : std::vector<int>{-1,0};
        for (const auto offset : offsets)
        {
            const int relative = wrap(target->pitchClass + offset - foundation.actualChord.rootPitchClass);
            idea.approachShape.push_back({offset, offset == 0, relative, foundation.actualChord.hasTone(relative)});
        }
        result.concepts.push_back(std::move(idea));
    }
}
}
void addHarmonicConcepts(ImprovisationResult& result)
{
    result.concepts.clear();
    if (!result.valid || result.strategies.empty()) return;
    addFoundationIdeas(result);
    addSourceIdeas(result);
    addApproachIdeas(result);
}
std::string harmonicConceptsText(const ImprovisationResult& result)
{
    std::ostringstream out;
    const auto explanationText = explanationDiagnosticText(result);
    if (! explanationText.empty())
        out << explanationText << "\n";

    const auto degreeText = [](const MaterialNote& note)
    {
        if (note.degree <= 0) return std::string("?");
        constexpr int major[] = {0,2,4,5,7,9,11};
        int delta = wrap(note.semitonesFromRoot - major[(note.degree - 1) % 7]);
        if (delta > 6) delta -= 12;
        return std::string(static_cast<std::size_t>(std::abs(delta)), delta < 0 ? 'b' : '#') + std::to_string(note.degree);
    };
    const auto pitchInChord = [](int pitch, const NormalizedChord& chord)
    {
        MaterialNote note;
        note.pitchClass = pitch;
        note.semitonesFromRoot = wrap(pitch - chord.rootPitchClass);
        note.degree = chord.degrees[static_cast<std::size_t>(note.semitonesFromRoot)];
        return spelled(note, chord).spelling;
    };
    for (const auto& idea : result.concepts)
    {
        out << idea.title << "\n" << idea.instruction << "\n";
        if (!idea.material.empty())
        {
            out << "Material on " << normalizedChordSymbol(idea.actualChord) << ": ";
            for (const auto& note : idea.material)
            {
                out << note.spelling << " (" << degreeText(note) << ")";
                if (note.role == MaterialNoteRole::passingTone) out << " [passing]";
                out << " ";
            }
            out << "\n";
        }
        if (idea.target.pitchClass >= 0)
            out << "Target: " << idea.target.spelling << " (" << degreeText(idea.target) << ") in "
                << normalizedChordSymbol(idea.targetChord)
                << (idea.targetScope == ConceptTargetScope::nextChord ? " [next chord]\n" : " [current chord]\n");
        if (!idea.moves.empty())
        {
            out << (idea.movesAreConfirmed ? "Confirmed moves: " : "Optional moves: ");
            for (const auto& move : idea.moves)
                out << pitchInChord(move.fromPitchClass, idea.actualChord) << " -> "
                    << pitchInChord(move.toPitchClass, idea.targetChord) << "  ";
            out << "\n";
        }
        if (!idea.approachShape.empty())
        {
            out << "Preparation over " << normalizedChordSymbol(idea.actualChord) << ": ";
            for (const auto& step : idea.approachShape)
            {
                if (step.isTarget) continue;
                out << (step.semitonesFromTarget > 0 ? "+" : "") << step.semitonesFromTarget
                    << (step.belongsToCurrentChord ? " [chord tone] " : " [non-chord tone] ");
            }
            out << "\n";
        }
        out << idea.conditions << "\n\n";
    }
    return out.str();
}

}
