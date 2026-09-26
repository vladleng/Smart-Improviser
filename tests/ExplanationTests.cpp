#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/HarmonicFunction.h"
#include "core/analysis/ImprovisationEngine.h"

#include <cstdlib>
#include <initializer_list>
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

KeyContext makeKey(std::int32_t rootFifths, bool minor = false)
{
    KeyContext key;
    key.available = true;
    key.defined = true;
    key.root = rootFifths;
    const int majorIntervals[] = {0, 2, 4, 5, 7, 9, 11};
    const int minorIntervals[] = {0, 2, 3, 5, 7, 8, 10};
    const auto* values = minor ? minorIntervals : majorIntervals;
    for (int i = 0; i < 7; ++i)
        key.intervals.values[static_cast<std::size_t>(values[i])] = 0xFFu;
    return key;
}

ChordContext makeChord(std::int32_t rootFifths,
                       std::initializer_list<int> relativeTones)
{
    ChordContext chord;
    chord.available = true;
    chord.defined = true;
    chord.root = rootFifths;
    chord.bass = rootFifths;
    for (const auto semitone : relativeTones)
        chord.intervals.values[static_cast<std::size_t>(semitone)] = 0xFFu;
    return chord;
}

TimelineHarmonicSnapshot currentNext(const KeyContext& key,
                                     const ChordContext& current,
                                     const ChordContext& next)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = key;
    snapshot.currentChord = current;
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}

bool hasEvidence(const ExplanationItem& item,
                 ExplanationEvidenceKind kind,
                 ExplanationEvidenceState state)
{
    for (const auto& evidence : item.why)
        if (evidence.kind == kind && evidence.state == state)
            return true;
    return false;
}

bool hasContextLayer(const ExplanationResult& explanation,
                     ExplanationContextScope scope,
                     int rootFifths)
{
    for (const auto& layer : explanation.contextLayers)
    {
        if (layer.scope == scope && layer.center.valid
            && layer.center.key.rootFifths == rootFifths)
            return true;
    }
    return false;
}
}

int main()
{
    const auto cMajor = makeKey(0);
    const auto g7 = makeChord(1, {0, 4, 7, 10});
    const auto cMaj7 = makeChord(0, {0, 4, 7, 11});

    auto result = analyzeImprovisation(analyzeHarmonicSituation(currentNext(cMajor, g7, cMaj7)));
    auto explanation = explainImprovisation(result);
    expect(explanation.valid && ! explanation.items.empty(), "explanation exists for valid improvisation result");
    expect(! explanation.items.front().idea.empty(), "idea preserved");
    expect(explanation.items.front().source.kind != MaterialKind::undefined, "source preserved");
    expect(! explanation.items.front().importantNotes.empty(), "important notes populated");
    expect(explanation.items.front().resolution.confirmed, "confirmed resolution preserved");
    expect(hasEvidence(explanation.items.front(), ExplanationEvidenceKind::resolution,
                       ExplanationEvidenceState::confirmed),
           "confirmed resolution exposed as Why evidence");
    expect(hasContextLayer(explanation, ExplanationContextScope::global, 0),
           "global C layer exposed explicitly");

    // Identical presentation material from two interpretations must collapse,
    // while provenance remains explicit.
    ImprovisationResult duplicate;
    duplicate.valid = true;
    duplicate.context = result.context;
    duplicate.context.primaryInterpretationIndex = -1;
    duplicate.context.interpretationCount = 2;
    duplicate.context.evidence.markAmbiguous(2);
    auto first = result.strategies.front();
    first.interpretationIndependent = false;
    first.interpretationIndex = 0;
    auto second = first;
    second.interpretationIndex = 1;
    duplicate.strategies = {first, second};

    const auto collapsed = explainImprovisation(duplicate);
    expect(collapsed.items.size() == 1, "identical material collapsed");
    expect(collapsed.items.front().strategyIndices.size() == 2, "both strategies retained in provenance");
    expect(collapsed.items.front().interpretationIndices.size() == 2
           && collapsed.items.front().interpretationIndices[0] == 0
           && collapsed.items.front().interpretationIndices[1] == 1,
           "interpretation provenance preserved");
    expect(hasEvidence(collapsed.items.front(), ExplanationEvidenceKind::interpretation,
                       ExplanationEvidenceState::ambiguous),
           "unresolved ambiguity exposed without hidden winner");

    // Visually different enharmonic spellings must not be merged merely because
    // the pitch class is the same. This protects accepted SubV spelling rules.
    auto dbMaterial = first;
    auto csMaterial = first;
    dbMaterial.actualChord.rootPitchClass = 1;
    dbMaterial.actualChord.rootFifths = -5;
    dbMaterial.source.rootPitchClass = 1;
    dbMaterial.source.rootFifths = -5;
    csMaterial.actualChord.rootPitchClass = 1;
    csMaterial.actualChord.rootFifths = 7;
    csMaterial.source.rootPitchClass = 1;
    csMaterial.source.rootFifths = 7;
    ImprovisationResult enharmonic;
    enharmonic.valid = true;
    enharmonic.context = result.context;
    enharmonic.strategies = {dbMaterial, csMaterial};
    expect(explainImprovisation(enharmonic).items.size() == 2,
           "Db and C# presentation remain distinct");

    // 0.3g context contract: the UI receives global/local levels separately.
    // Synthetic setup mirrors the real Gm7 case: global C, local F.
    auto gm7Result = analyzeImprovisation(analyzeHarmonicSituation(
        currentNext(cMajor, makeChord(1, {0, 3, 7, 10}), makeChord(0, {0, 4, 7, 10}))));
    gm7Result.context.localKey = makeGlobalKeyCenter(normalizeKey(makeKey(-1)));
    gm7Result.context.localKey.scope = KeyCenterScope::local;
    gm7Result.context.localHarmonic = analyzeHarmonicFunction(
        gm7Result.context.currentChord, gm7Result.context.localKey.key, gm7Result.context.nextChord);
    const auto layered = explainImprovisation(gm7Result);
    expect(hasContextLayer(layered, ExplanationContextScope::global, 0),
           "Gm7 keeps global C context layer");
    expect(hasContextLayer(layered, ExplanationContextScope::local, -1),
           "Gm7 exposes local F context layer separately");

    // Fix4 semantics: missing tonic stays a distinct grey/missing fact, while
    // the known actual continuation is a separate contradicted fact.
    auto incomplete = result;
    incomplete.context.incompleteCadence.valid = true;
    incomplete.context.incompleteCadence.positionIndex = 1;
    incomplete.context.incompleteCadence.missingTonicRootFifths = -3; // Eb
    incomplete.context.incompleteCadence.actualContinuation =
        normalizeChord(makeChord(4, {0, 3, 7, 10})); // Em7
    const auto incompleteExplanation = explainImprovisation(incomplete);
    expect(hasEvidence(incompleteExplanation.items.front(), ExplanationEvidenceKind::incompleteCadence,
                       ExplanationEvidenceState::missing),
           "missing tonic exposed separately");
    expect(hasEvidence(incompleteExplanation.items.front(), ExplanationEvidenceKind::continuationConflict,
                       ExplanationEvidenceState::contradicted),
           "actual continuation exposed as contradiction");

    // Fix3 semantics: rootless functional alias is implied, not an error and
    // does not overwrite written chord identity.
    auto implied = result;
    implied.context.impliedDominant.valid = true;
    implied.context.impliedDominant.rootPitchClass = 7; // G
    implied.context.impliedDominant.flatNinth = true;
    implied.context.impliedDominant.evidence.confidence = ConfidenceLevel::medium;
    const auto impliedExplanation = explainImprovisation(implied);
    expect(hasEvidence(impliedExplanation.items.front(), ExplanationEvidenceKind::impliedDominant,
                       ExplanationEvidenceState::implied),
           "rootless dominant exposed as implied evidence");
    expect(impliedExplanation.items.front().actualChord.rootPitchClass
           == result.strategies.front().actualChord.rootPitchClass,
           "written chord identity preserved");

    // Invalid material must not produce a fake explanation.
    ImprovisationResult invalid;
    expect(! explainImprovisation(invalid).valid, "invalid result yields no explanation");

    std::cout << "Explanation tests passed\n";
}
