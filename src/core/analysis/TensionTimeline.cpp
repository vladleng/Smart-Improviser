#include "core/analysis/TensionTimeline.h"
#include "core/analysis/PhraseMatcher.h"
#include <algorithm>
#include <cmath>

namespace smartimproviser::harmony
{
namespace
{
bool validLevel(TensionLevel level)
{
    const auto n = static_cast<unsigned>(level);
    return n >= 1 && n <= 3;
}
void invalid(TensionTimelineValidation& out, TensionTimelineReason reason,
             int index, double start, double end, const char* message)
{
    out.valid = false;
    out.diagnostics.push_back({reason, index, start, end, message});
}
TensionTimelineValidation validateSpans(const std::vector<TensionSpan>& spans)
{
    TensionTimelineValidation out;
    double previousEnd = 0.0;
    for (std::size_t i = 0; i < spans.size(); ++i)
    {
        const auto& s = spans[i];
        if (!std::isfinite(s.startBeat) || !std::isfinite(s.endBeat)
            || s.startBeat < 0.0 || s.endBeat <= s.startBeat)
            invalid(out, TensionTimelineReason::invalidTiming, static_cast<int>(i),
                    s.startBeat, s.endBeat, "Spans need finite nonnegative starts and positive lengths.");
        if (i > 0 && s.startBeat < previousEnd)
            invalid(out, TensionTimelineReason::overlappingSpans, static_cast<int>(i),
                    s.startBeat, s.endBeat, "Spans must be ordered and must not overlap.");
        if (s.level && !validLevel(*s.level))
            invalid(out, TensionTimelineReason::invalidLevel, static_cast<int>(i),
                    s.startBeat, s.endBeat, "Only explicit T1/T2/T3 assignments are supported.");
        previousEnd = s.endBeat;
    }
    return out;
}
double phraseEnd(const Phrase& phrase)
{
    double end = 0.0;
    for (const auto& n : phrase.notes)
    {
        if (!std::isfinite(n.beatOffset) || !std::isfinite(n.durationBeats)
            || n.beatOffset < 0.0 || n.durationBeats <= 0.0
            || !std::isfinite(n.beatOffset + n.durationBeats)) return -1.0;
        end = std::max(end, n.beatOffset + n.durationBeats);
    }
    return end;
}
std::vector<TensionSpan> description(const Phrase& phrase, double end)
{
    if (phrase.tensionProfile.defined) return phrase.tensionProfile.spans;
    return {{0.0, end, phrase.tensionClassified ? std::optional<TensionLevel>(phrase.tensionLevel) : std::nullopt}};
}
std::optional<TensionLevel> levelAt(const std::vector<TensionSpan>& spans, double beat)
{
    for (const auto& s : spans)
        if (s.startBeat <= beat && beat < s.endBeat) return s.level;
    return std::nullopt;
}
std::vector<double> boundaries(double start, double end,
    const std::vector<TensionSpan>& a, const std::vector<TensionSpan>& b)
{
    std::vector<double> points {start, end};
    for (const auto* spans : {&a, &b})
        for (const auto& s : *spans)
            for (double t : {s.startBeat, s.endBeat})
                if (start < t && t < end) points.push_back(t);
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    return points;
}
}

TensionTimelineValidation validateTensionCurve(const TensionCurve& curve)
{
    auto out = validateSpans(curve.spans);
    for (auto& diagnostic : out.diagnostics) diagnostic.scope = TensionTimelineScope::desiredCurve;
    return out;
}
TensionTimelineValidation validatePhraseTensionProfile(const Phrase& phrase)
{
    auto out = validateSpans(phrase.tensionProfile.spans);
    if (!phrase.tensionProfile.defined && !phrase.tensionProfile.spans.empty())
        invalid(out, TensionTimelineReason::undefinedProfileData, -1, 0, 0,
                "Profile spans exist but the profile is not marked defined.");
    if (!phrase.tensionProfile.defined && phrase.tensionClassified && !validLevel(phrase.tensionLevel))
        invalid(out, TensionTimelineReason::invalidLevel, -1, 0, 0, "Invalid legacy whole-phrase assignment.");
    const auto end = phraseEnd(phrase);
    if (end <= 0.0)
        invalid(out, TensionTimelineReason::invalidTiming, -1, 0, end, "Phrase extent cannot be established from its notes.");
    for (std::size_t i = 0; i < phrase.tensionProfile.spans.size(); ++i)
    {
        const auto& s = phrase.tensionProfile.spans[i];
        if (s.endBeat > end)
            invalid(out, TensionTimelineReason::outsidePhrase, static_cast<int>(i), s.startBeat, s.endBeat,
                    "A descriptive span cannot extend beyond the phrase's last note end.");
    }
    return out;
}
PhraseTensionMatch matchPhraseTension(const Phrase& phrase, std::optional<TensionLevel> requested)
{
    if (!validatePhraseTensionProfile(phrase).valid || (requested && !validLevel(*requested)))
        return PhraseTensionMatch::invalidProfile;
    if (!requested) return PhraseTensionMatch::any;
    const double end = phraseEnd(phrase);
    const auto spans = description(phrase, end);
    const auto points = boundaries(0, end, spans, {});
    bool unknown = false, mismatch = false;
    for (std::size_t i = 0; i + 1 < points.size(); ++i)
    {
        const auto assigned = levelAt(spans, points[i]);
        unknown = unknown || !assigned;
        mismatch = mismatch || (assigned && *assigned != *requested);
    }
    return mismatch ? PhraseTensionMatch::differentLevel
        : unknown ? PhraseTensionMatch::unclassified : PhraseTensionMatch::matches;
}

PhraseCurveMatchResult assessPhraseAgainstCurve(const Phrase& phrase,
    const std::vector<PhraseMatchSlot>& slots, const TensionCurve& curve, double placementStartBeat)
{
    PhraseCurveMatchResult out;
    out.harmonicMatch = assessPhrase(phrase, {slots, std::nullopt});
    if (out.harmonicMatch.harmonic != PhraseCompatibility::compatible) return out;
    const auto profileCheck = validatePhraseTensionProfile(phrase);
    const auto curveCheck = validateTensionCurve(curve);
    out.diagnostics = profileCheck.diagnostics;
    out.diagnostics.insert(out.diagnostics.end(), curveCheck.diagnostics.begin(), curveCheck.diagnostics.end());
    const double end = phraseEnd(phrase);
    const double songEnd = placementStartBeat + end;
    if (!std::isfinite(placementStartBeat) || placementStartBeat < 0.0
        || !std::isfinite(songEnd) || songEnd <= placementStartBeat)
        out.diagnostics.push_back({TensionTimelineReason::invalidPlacement, -1,
            placementStartBeat, songEnd, "Placement must retain a finite positive phrase extent in song beats.", TensionTimelineScope::placement});
    if (!out.diagnostics.empty()) { out.tension = TensionCurveMatch::invalidData; return out; }
    auto spans = description(phrase, end);
    for (std::size_t i = 0; i < spans.size(); ++i)
    {
        auto& s = spans[i];
        s.startBeat += placementStartBeat;
        s.endBeat += placementStartBeat;
        if (!std::isfinite(s.startBeat) || !std::isfinite(s.endBeat) || s.endBeat <= s.startBeat)
            out.diagnostics.push_back({TensionTimelineReason::invalidPlacement, static_cast<int>(i),
                s.startBeat, s.endBeat, "Placement loses the precision of a descriptive interval.", TensionTimelineScope::placement});
    }
    if (!out.diagnostics.empty()) { out.tension = TensionCurveMatch::invalidData; return out; }
    const auto points = boundaries(placementStartBeat, songEnd, spans, curve.spans);
    bool constrained = false, unknown = false, mismatch = false;
    for (std::size_t i = 0; i + 1 < points.size(); ++i)
    {
        const auto desired = levelAt(curve.spans, points[i]);
        const auto assigned = levelAt(spans, points[i]);
        out.comparisons.push_back({points[i], points[i+1], desired, assigned});
        if (!desired) continue;
        constrained = true;
        if (!assigned)
        {
            unknown = true;
            out.diagnostics.push_back({TensionTimelineReason::profileUnclassified, -1, points[i], points[i+1],
                "Requested interval overlaps an unevaluated descriptive interval.", TensionTimelineScope::comparison});
        }
        else if (*assigned != *desired)
        {
            mismatch = true;
            out.diagnostics.push_back({TensionTimelineReason::levelMismatch, -1, points[i], points[i+1],
                "Descriptive phrase level differs from the desired song interval.", TensionTimelineScope::comparison});
        }
    }
    out.tension = mismatch ? TensionCurveMatch::differentLevel : unknown ? TensionCurveMatch::unclassified
        : constrained ? TensionCurveMatch::matches : TensionCurveMatch::unconstrained;
    return out;
}
}
