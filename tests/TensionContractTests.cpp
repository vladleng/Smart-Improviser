#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/TensionEngine.h"

#include <cstdlib>
#include <iostream>

using namespace smartimproviser::harmony;

namespace
{
void expect(bool value, const char* message)
{
    if (!value) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

ChordContext chord(int fifths, std::initializer_list<int> tones)
{
    ChordContext result;
    result.available = result.defined = true;
    result.root = result.bass = fifths;
    for (const int interval : tones)
        result.intervals.values[static_cast<std::size_t>(interval)] = 0xFFu;
    return result;
}

TimelineHarmonicSnapshot snapshot()
{
    TimelineHarmonicSnapshot result;
    result.positionAvailable = true;
    result.ppq = 4.0;
    result.globalKey.available = result.globalKey.defined = true;
    result.globalKey.root = 0;
    for (const int tone : {0,2,4,5,7,9,11})
        result.globalKey.intervals.values[static_cast<std::size_t>(tone)] = 0xFFu;
    result.previousChordAvailable = true;
    result.previousChord = chord(2,{0,3,7,10});
    result.currentChord = chord(1,{0,4,7,10});
    result.nextChordAvailable = true;
    result.nextChord = chord(0,{0,4,7,11});
    return result;
}
}

int main()
{
    expect(static_cast<int>(TensionRole::unassigned) == 0
           && static_cast<int>(TensionLevel::outsideMaximum) == 3,
           "universal note role defaults to unassigned, existing levels retain 1-3 ABI");

    const auto analyzed = analyzeImprovisation(analyzeHarmonicSituation(snapshot()));
    expect(analyzed.valid && analyzed.context.resolution.confirmed,
           "real confirmed G7-Cmaj7 context is available");
    const auto untouched = buildTensionProfile(analyzed);
    expect(untouched.valid && untouched.context.resolution.confirmed
           && untouched.context.resolution.targetChord.quality == ChordQuality::major
           && untouched.context.currentChord.rootPitchClass == 7,
           "contract carries actual chord and target from Core without reinterpretation");
    for (const auto& band : untouched.bands)
        expect(band.assessment == TensionAssessment::notEvaluated
               && !band.hasRecommendation() && band.alternatives.empty(),
               "an unclassified catalog is not automatically assigned to T1/T2/T3");
    expect(untouched.band(static_cast<TensionLevel>(0)) == nullptr
           && untouched.band(static_cast<TensionLevel>(4)) == nullptr,
           "unknown level is not silently mapped to T1");

    auto alternativeResult = analyzed;
    alternativeResult.context.primaryInterpretationIndex = -1;
    alternativeResult.context.evidence.markAmbiguous(2);
    const auto source = alternativeResult.strategies.front();
    alternativeResult.strategies.clear();
    for (int interpretation : {0, 1})
    {
        auto strategy = source;
        strategy.tensionClassified = true; // A later policy, simulated here.
        strategy.tension = TensionLevel::color;
        strategy.ruleId = "test.interpretation." + std::to_string(interpretation);
        strategy.interpretationIndex = interpretation;
        strategy.interpretationIndependent = false;
        alternativeResult.strategies.push_back(strategy);
    }
    const auto alternatives = buildTensionProfile(alternativeResult);
    const auto* t2 = alternatives.band(TensionLevel::color);
    expect(t2 && t2->hasRecommendation() && t2->alternatives.size() == 2
           && t2->alternatives[0].strategy.interpretationIndex == 0
           && t2->alternatives[1].strategy.interpretationIndex == 1
           && t2->alternatives[0].noteRoles.empty()
           && alternatives.context.primaryInterpretationIndex == -1
           && alternatives.context.evidence.interpretation == InterpretationStatus::ambiguous,
           "classified alternatives survive without choosing a harmonic winner");
    expect(alternatives.band(TensionLevel::stable)->assessment == TensionAssessment::notEvaluated
           && alternatives.band(TensionLevel::outsideMaximum)->assessment == TensionAssessment::notEvaluated,
           "one supported level does not fabricate support or unavailability at other levels");
    TensionRoleAssignment targetRole;
    expect(targetRole.role == TensionRole::unassigned
           && targetRole.scope == TensionNoteScope::undefined,
           "note role and its current/next-chord scope require explicit assignment");

    auto unknownFuture = snapshot();
    unknownFuture.nextChordAvailable = false;
    const auto unresolved = buildTensionProfile(
        analyzeImprovisation(analyzeHarmonicSituation(unknownFuture)));
    expect(unresolved.valid && !unresolved.context.nextChordAvailable
           && !unresolved.context.resolution.confirmed,
           "unknown future is not silently replaced with a resolution target");

    const auto invalid = buildTensionProfile(ImprovisationResult{});
    expect(!invalid.valid && !invalid.band(TensionLevel::stable)->hasRecommendation(),
           "no harmonic context produces no tension recommendation");

    std::cout << "Tension contract tests passed\n";
}
