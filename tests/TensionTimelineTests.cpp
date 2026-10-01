#include "core/analysis/TensionTimeline.h"
#include "core/analysis/PhraseMatcher.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace smartimproviser::harmony;
void check(bool value, const char* message)
{
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}
ChordContext chord(int root, std::initializer_list<int> tones)
{
    ChordContext c; c.available = c.defined = true; c.root = c.bass = root;
    for (int n : tones) c.intervals.values[n] = 0xFF;
    return c;
}
ImprovisationResult material(bool tonic = false)
{
    TimelineHarmonicSnapshot s; s.positionAvailable = true;
    s.globalKey.available = s.globalKey.defined = true; s.globalKey.root = 0;
    for (int n : {0,2,4,5,7,9,11}) s.globalKey.intervals.values[n] = 0xFF;
    s.currentChord = tonic ? chord(0,{0,4,7,11}) : chord(1,{0,4,7,10});
    s.nextChordAvailable = !tonic; s.nextChord = chord(0,{0,4,7,11});
    PatternTimelineWindow w; w.chordCount = tonic ? 1 : 2;
    w.chords[0] = s.currentChord; w.chords[1] = s.nextChord;
    return analyzeImprovisation(analyzeHarmonicSituation(s,w));
}
bool sameNotes(const Phrase& a, const Phrase& b)
{
    if (a.notes.size() != b.notes.size()) return false;
    for (std::size_t i = 0; i < a.notes.size(); ++i)
    {
        const auto& x = a.notes[i]; const auto& y = b.notes[i];
        if (x.pitch.chordIndex != y.pitch.chordIndex || x.pitch.degree != y.pitch.degree
            || x.pitch.chromaticOffset != y.pitch.chromaticOffset || x.beatOffset != y.beatOffset
            || x.durationBeats != y.durationBeats || x.target != y.target || x.harmonicRole != y.harmonicRole) return false;
    }
    return true;
}
bool sameSpans(const std::vector<TensionSpan>& a, const std::vector<TensionSpan>& b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i].startBeat != b[i].startBeat || a[i].endBeat != b[i].endBeat || a[i].level != b[i].level) return false;
    return true;
}
int main()
{
    constexpr auto t1 = TensionLevel::stable, t2 = TensionLevel::color, t3 = TensionLevel::outsideMaximum;
    Phrase p;
    for (int i = 0; i < 6; ++i) p.notes.push_back({{0, i % 2 ? 3 : 1, 0}, double(i*2), 2, false, PhraseNoteRole::chordAnchor});
    const std::vector<PhraseMatchSlot> slots {{material(),0,12}};
    check(slots[0].material.valid, "real analyzed harmonic fixture required");
    check(validatePhraseTensionProfile(p).valid, "default profile preserves unassigned legacy data");
    auto r = assessPhraseAgainstCurve(p,slots,{},60);
    check(r.eligible() && r.tension == TensionCurveMatch::unconstrained, "empty desired curve is unconstrained, never T1");
    TensionCurve curve {{{60,64,t1},{64,68,t2},{68,72,t3}}};
    r = assessPhraseAgainstCurve(p,slots,curve,60);
    check(!r.eligible() && r.tension == TensionCurveMatch::unclassified, "desired levels cannot classify an unevaluated phrase");
    p.tensionProfile = {true,{{0,4,t1},{4,8,t2},{8,12,t3}}};
    const Phrase before = p;
    const TensionCurve curveBefore = curve;
    r = assessPhraseAgainstCurve(p,slots,curve,60);
    check(r.eligible() && r.tension == TensionCurveMatch::matches && r.comparisons.size() == 3,
        "descriptive relative profile aligns with absolute song curve at explicit placement");
    check(r.comparisons[1].songStartBeat == 64 && r.comparisons[1].songEndBeat == 68
        && r.comparisons[1].desired == t2 && r.comparisons[1].descriptive == t2,
        "half-open boundary switches to the next level without duplicating a point");
    for (auto level : {t1,t2,t3})
    {
        check(assessPhrase(p,{slots,level}).tension == PhraseTensionMatch::differentLevel,
            "mixed descriptive profile cannot masquerade as one scalar whole-phrase level");
        TensionCurve changed {{{60,72,level}}};
        check(assessPhraseAgainstCurve(p,slots,changed,60).tension == TensionCurveMatch::differentLevel,
            "changing curve compares data without relabeling a phrase");
    }
    check(sameNotes(before,p) && sameSpans(before.tensionProfile.spans,p.tensionProfile.spans)
        && sameSpans(curve.spans,curveBefore.spans) && before.tensionClassified == p.tensionClassified,
        "curve/scalar checks do not mutate notes, rhythm, descriptive assignments or desired curve");
    check(assessPhrase(p,{slots,std::nullopt}).eligible(), "All still accepts valid mixed profiles");
    TensionCurve split {{{59,62,t1},{62,64,t1},{64,67,t2},{67,68,t2},{68,75,t3}}};
    check(assessPhraseAgainstCurve(p,slots,split,60).eligible(),
        "splitting equal-level desired spans or extending them beyond phrase extent preserves matching");
    check(assessPhraseAgainstCurve(p,slots,curve,61).tension == TensionCurveMatch::differentLevel,
        "placement is explicit, not guessed from phrase time or curve");

    // Unassigned desired time does not constrain a phrase. Unassigned descriptive
    // time blocks only an explicit desired interval, without legacy fallback.
    p.tensionClassified = true; p.tensionLevel = t3;
    p.tensionProfile = {true,{{0,4,t1},{8,12,t3}}};
    r = assessPhraseAgainstCurve(p,slots,curve,60);
    check(r.tension == TensionCurveMatch::unclassified, "descriptive gap remains unknown despite a legacy T3 label");
    TensionCurve gaps {{{60,64,t1},{64,68,std::nullopt},{68,72,t3}}};
    check(assessPhraseAgainstCurve(p,slots,gaps,60).eligible(), "explicit null desired interval imposes no constraint");
    gaps.spans.erase(gaps.spans.begin()+1);
    check(assessPhraseAgainstCurve(p,slots,gaps,60).eligible(), "missing desired interval has the same unassigned semantics");
    p.tensionProfile.spans.insert(p.tensionProfile.spans.begin()+1,{4,8,std::nullopt});
    check(assessPhraseAgainstCurve(p,slots,curve,60).tension == TensionCurveMatch::unclassified,
        "explicit unevaluated descriptive interval is not T1");
    TensionCurve mismatchAndUnknown {{{60,64,t2},{64,68,t2},{68,72,t3}}};
    r = assessPhraseAgainstCurve(p,slots,mismatchAndUnknown,60);
    check(r.tension == TensionCurveMatch::differentLevel && r.diagnostics.size() == 2,
        "a known mismatch takes precedence while unknown interval diagnostics remain available");
    p.tensionProfile = {true,{}};
    check(assessPhrase(p,{slots,t3}).tension == PhraseTensionMatch::unclassified,
        "defined empty profile explicitly clears the legacy classification");
    p.tensionProfile = {};
    check(assessPhrase(p,{slots,t3}).eligible(), "absent new profile retains assigned legacy whole-phrase T3");
    check(assessPhraseAgainstCurve(p,slots,TensionCurve{{{60,72,t3}}},60).eligible(), "legacy whole-phrase label feeds curve matching");
    p.tensionClassified = false;
    check(assessPhraseAgainstCurve(p,slots,TensionCurve{{{0,60,t1},{72,76,t3}}},60).eligible(),
        "desired spans merely touching phrase boundaries do not constrain it");

    // Timing validation is strict, supports arbitrary beat lengths, and never
    // repairs overlapping or malformed metadata by averaging/interpolation.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    for (const auto& bad : std::vector<TensionCurve>{
        {{{0,0,t1}}}, {{{-1,2,t1}}}, {{{0,inf,t1}}}, {{{nan,2,t1}}},
        {{{0,3,t1},{2,4,t2}}}, {{{4,6,t1},{0,2,t2}}}, {{{0,2,static_cast<TensionLevel>(0)}}}})
    {
        check(!validateTensionCurve(bad).valid, "invalid curve must be diagnosed");
        check(validateTensionCurve(bad).diagnostics.front().scope == TensionTimelineScope::desiredCurve,
            "diagnostics identify desired metadata separately from descriptive profile");
        check(assessPhraseAgainstCurve(p,slots,bad,60).tension == TensionCurveMatch::invalidData,
            "invalid desired metadata is not treated as unconstrained");
    }
    p.tensionProfile = {false,{{0,12,t1}}};
    check(!validatePhraseTensionProfile(p).valid && !assessPhrase(p,{slots,std::nullopt}).eligible(),
        "spans cannot silently hide behind undefined profile flag even in All");
    p.tensionProfile = {true,{{0,13,t1}}};
    check(!validatePhraseTensionProfile(p).valid, "profile cannot extend beyond phrase extent");
    p.tensionProfile = {true,{{0,7,t1},{6,12,t2}}};
    check(!validatePhraseTensionProfile(p).valid, "overlapping descriptive spans rejected");
    p.tensionProfile = {true,{{0,12,static_cast<TensionLevel>(4)}}};
    check(assessPhrase(p,{slots,t1}).tension == PhraseTensionMatch::invalidProfile, "invalid descriptive enum rejected");
    p.tensionProfile = before.tensionProfile;
    for (double placement : {-1.0,nan,inf,std::numeric_limits<double>::max()})
        check(assessPhraseAgainstCurve(p,slots,curve,placement).tension == TensionCurveMatch::invalidData,
            "invalid placement or loss of finite time precision rejected");
    p.tensionProfile = {true,{{0,3.5,t1},{3.5,9.25,t2},{9.25,12,t3}}};
    TensionCurve fractional {{{1.25,4.75,t1},{4.75,10.5,t2},{10.5,13.25,t3}}};
    check(assessPhraseAgainstCurve(p,slots,fractional,1.25).eligible(), "no fixed meter or integer beat assumption");
    p.tensionProfile = before.tensionProfile;
    p.notes[0].pitch.degree = 2;
    r = assessPhraseAgainstCurve(p,slots,curve,60);
    check(r.harmonicMatch.harmonic == PhraseCompatibility::incompatible && r.tension == TensionCurveMatch::notEvaluated
        && !r.eligible() && r.comparisons.empty(), "matching tension cannot authorize a non-chord anchor");
    p = before;
    r = assessPhraseAgainstCurve(p,{},curve,60);
    check(r.harmonicMatch.harmonic == PhraseCompatibility::insufficientContext && r.tension == TensionCurveMatch::notEvaluated,
        "missing harmonic context cannot be replaced by curve data");

    // Two chord slots retain 0.4e timing and actual-chord constraints.
    Phrase two;
    two.notes = {{{0,1,0},0,4,false,PhraseNoteRole::chordAnchor},{{1,3,0},4,4,true,PhraseNoteRole::resolutionTarget}};
    two.tensionProfile = {true,{{0,4,t2},{4,8,t1}}};
    const std::vector<PhraseMatchSlot> twoSlots {{material(),0,4},{material(true),4,8}};
    check(assessPhraseAgainstCurve(two,twoSlots,TensionCurve{{{20,24,t2},{24,28,t1}}},20).eligible(),
        "profile and curve can cross actual chord slots without altering harmonic data");
    std::cout << "Tension timeline tests passed\n";
}
