#include "core/analysis/LibraryValidation.h"
#include "core/analysis/TensionTimeline.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace smartimproviser::harmony
{
namespace
{
template<class E> bool atMost(E value, E last)
{
    return static_cast<unsigned>(value) <= static_cast<unsigned>(last);
}
bool hasText(const std::string& s) { return s.find_first_not_of(" \t\r\n") != std::string::npos; }
bool validPitch(const RelativePitch& p, bool partial)
{
    return p.chordIndex >= (partial ? -1 : 0) && p.degree >= 1 && p.degree <= 13
        && p.chromaticOffset >= -2 && p.chromaticOffset <= 2;
}
bool validFingering(const PhraseFingering& f) { return f.stringNumber >= 1 && f.fret >= 0; }
bool validOffset(int n) { return n >= -11 && n <= 10; }
bool validReference(const LibraryItemReference& r)
{
    return atMost(r.domain, LibraryDomain::user) && hasText(r.id) && r.revision > 0;
}
}
std::optional<int> phraseExampleMidiPitch(const Phrase& p, std::size_t index)
{
    if (index >= p.notes.size()) return std::nullopt;
    const auto& n = p.notes[index];
    if (!validPitch(n.pitch, false) || !n.octaveOffset || !validOffset(*n.octaveOffset))
        return std::nullopt;
    const PhraseRegisterReference* root = nullptr;
    for (const auto& r : p.registerReferences) if (r.chordIndex == n.pitch.chordIndex)
    {
        if (root || r.rootMidiNote < 0 || r.rootMidiNote > 127) return std::nullopt;
        root = &r;
    }
    if (!root) return std::nullopt;
    constexpr int major[] = {0,2,4,5,7,9,11};
    const int degree = n.pitch.degree - 1;
    const int midi = root->rootMidiNote + major[degree % 7] + 12 * (degree / 7)
        + n.pitch.chromaticOffset + 12 * *n.octaveOffset;
    if (midi < 0 || midi > 127) return std::nullopt;
    return midi;
}
LibraryValidation validateLibraryRecord(const LibraryRecord& record)
{
    LibraryValidation out;
    auto report = [&](LibraryValidationScope scope, const std::string& field, const char* text)
    {
        out.diagnostics.push_back({scope,field,text});
        if (scope == LibraryValidationScope::structure) out.structurallyValid = false;
    };
    auto structural = [&](bool ok, const std::string& field, const char* text)
    { if (!ok) report(LibraryValidationScope::structure,field,text); };
    structural(hasText(record.id) && record.revision > 0, "identity", "Item ID and positive revision are required.");
    structural(atMost(record.domain,LibraryDomain::user), "domain", "Unknown library domain.");
    structural(atMost(record.source.permission,ContentPermission::licensed), "source.permission", "Unknown permission state.");
    if (record.lineage)
    {
        const auto& line = *record.lineage;
        structural(atMost(line.kind,LibraryDerivation::variant), "lineage.kind", "Unknown derivation.");
        std::set<std::pair<unsigned,std::string>> seen;
        auto ref = [&](const LibraryItemReference& r)
        {
            structural(validReference(r), "lineage", "Source identity requires a domain, ID and exact positive revision.");
            structural(!(r.domain == record.domain && r.id == record.id), "lineage", "A derived item needs its own identity.");
            structural(seen.emplace(static_cast<unsigned>(r.domain),r.id).second, "lineage", "Repeated ancestry/cycles are invalid.");
        };
        ref(line.parent);
        for (const auto& ancestor : line.ancestors) ref(ancestor);
    }
    auto registers = [&](const std::vector<PhraseRegisterReference>& refs)
    {
        std::set<int> slots;
        for (const auto& r : refs)
            structural(r.chordIndex >= 0 && r.rootMidiNote >= 0 && r.rootMidiNote <= 127
                && slots.insert(r.chordIndex).second, "registerReferences", "Register references need unique nonnegative slots and MIDI roots 0..127.");
    };
    if (const auto* idea = std::get_if<Idea>(&record.content))
    {
        registers(idea->registerReferences);
        if (idea->harmonicPattern)
            structural(atMost(*idea->harmonicPattern,HarmonicPatternType::halfDiminishedIiViMajor), "idea.pattern", "Unknown harmonic pattern.");
        for (const auto& n : idea->notes)
        {
            if (n.pitch) structural(validPitch(*n.pitch,true), "idea.pitch", "Known pitch needs degree 1..13, accidental -2..2 and slot >= -1.");
            if (n.beatOffset) structural(std::isfinite(*n.beatOffset) && *n.beatOffset >= 0, "idea.beat", "Known onset must be finite and nonnegative.");
            if (n.durationBeats) structural(std::isfinite(*n.durationBeats) && *n.durationBeats > 0, "idea.duration", "Known duration must be finite and positive.");
            if (n.beatOffset && n.durationBeats) structural(std::isfinite(*n.beatOffset + *n.durationBeats), "idea.end", "Note end must be finite.");
            if (n.octaveOffset) structural(validOffset(*n.octaveOffset), "idea.register", "Octave offset is out of supported MIDI range.");
            if (n.harmonicRole) structural(atMost(*n.harmonicRole,PhraseNoteRole::outsideTone), "idea.role", "Unknown note role.");
            if (n.fingering) structural(validFingering(*n.fingering), "idea.fingering", "Fingering needs a positive string and nonnegative fret.");
        }
        report(LibraryValidationScope::searchReadiness, "content", "A draft Idea is retained for browsing, never promoted by its title.");
    }
    else if (const auto* phrase = std::get_if<Phrase>(&record.content))
    {
        registers(phrase->registerReferences);
        structural(atMost(phrase->harmonicPattern,HarmonicPatternType::halfDiminishedIiViMajor)
            && atMost(phrase->role,PhraseRole::release), "phrase.metadata", "Unknown phrase role or pattern.");
        structural(phrase->startDegree >= 0 && phrase->startDegree <= 13
            && phrase->targetDegree >= 0 && phrase->targetDegree <= 13, "phrase.degrees", "Start/target degree must be unknown (0) or 1..13.");
        if (phrase->tensionClassified)
            structural(phrase->tensionLevel >= TensionLevel::stable && phrase->tensionLevel <= TensionLevel::outsideMaximum,
                "phrase.tension", "Unknown assigned tension level.");
        structural(validatePhraseTensionProfile(*phrase).valid, "phrase.tensionProfile", "Invalid descriptive tension profile.");
        bool complete = !phrase->notes.empty();
        if (!complete) report(LibraryValidationScope::searchReadiness,"phrase.notes","A ready phrase needs musical content.");
        double previous = -1;
        for (std::size_t i = 0; i < phrase->notes.size(); ++i)
        {
            const auto& n = phrase->notes[i];
            structural(validPitch(n.pitch,false), "phrase.pitch", "Ready note needs a known slot, degree 1..13 and accidental -2..2.");
            structural(std::isfinite(n.beatOffset) && std::isfinite(n.durationBeats)
                && std::isfinite(n.beatOffset+n.durationBeats) && n.beatOffset >= 0
                && n.durationBeats > 0 && n.beatOffset >= previous,
                "phrase.timing", "Ready notes need ordered finite onsets and positive durations.");
            previous = n.beatOffset;
            structural(atMost(n.harmonicRole,PhraseNoteRole::outsideTone), "phrase.role", "Unknown note role.");
            if (n.octaveOffset) structural(validOffset(*n.octaveOffset), "phrase.register", "Octave offset is out of supported MIDI range.");
            if (n.fingering) structural(validFingering(*n.fingering), "phrase.fingering", "Fingering needs a positive string and nonnegative fret.");
            if (!phraseExampleMidiPitch(*phrase,i))
            {
                complete = false;
                report(LibraryValidationScope::searchReadiness,"phrase.register","Explicit playable register is required; pitch class does not determine octave.");
            }
            const auto effectiveRole = n.target ? PhraseNoteRole::resolutionTarget : n.harmonicRole;
            if (effectiveRole == PhraseNoteRole::sourceTone || effectiveRole == PhraseNoteRole::characteristicTone)
            {
                const bool bound = std::any_of(phrase->harmonicRequirements.begin(),phrase->harmonicRequirements.end(),
                    [&](const auto& r) { return r.chordIndex == n.pitch.chordIndex && hasText(r.sourceRuleId) && r.sourceRuleVersion > 0; });
                if (!bound)
                {
                    complete = false;
                    report(LibraryValidationScope::searchReadiness,"phrase.source","Source-based notes need an exact source-rule identity/version.");
                }
            }
            if (n.harmonicRole == PhraseNoteRole::outsideTone)
            {
                complete = false;
                report(LibraryValidationScope::searchReadiness,"phrase.outside","Outside policy is not implemented; retain the content without claiming readiness.");
            }
            if (n.harmonicRole == PhraseNoteRole::passingApproach)
            {
                const bool grouped = std::any_of(phrase->approaches.begin(),phrase->approaches.end(),
                    [&](const auto& g) { return std::find(g.noteIndices.begin(),g.noteIndices.end(),i) != g.noteIndices.end(); });
                if (!grouped)
                {
                    complete = false;
                    report(LibraryValidationScope::searchReadiness,"phrase.approach","Passing note needs an explicit approach group.");
                }
            }
        }
        std::set<int> boundSlots;
        for (const auto& r : phrase->harmonicRequirements)
        {
            structural(r.chordIndex >= 0 && r.sourceRuleVersion >= 0 && r.sourceRootOffset >= -1 && r.sourceRootOffset <= 11
                && atMost(r.chordQuality,ChordQuality::unknown) && atMost(r.destinationQuality,ChordQuality::unknown)
                && atMost(r.harmonicPattern,HarmonicPatternType::halfDiminishedIiViMajor)
                && boundSlots.insert(r.chordIndex).second, "phrase.requirements", "Invalid or duplicate source application.");
            if (hasText(r.sourceRuleId) && r.sourceRuleVersion == 0)
            {
                complete = false;
                report(LibraryValidationScope::searchReadiness,"phrase.sourceVersion","Legacy wildcard versions require explicit revalidation for library search.");
            }
        }
        std::set<std::size_t> groupedNotes;
        for (const auto& g : phrase->approaches)
        {
            structural(g.chordIndex >= 0 && hasText(g.conceptRuleId) && g.noteIndices.size() >= 2,
                "phrase.approaches", "Approach needs a slot, concept and preparation/landing notes.");
            for (std::size_t k = 0; k < g.noteIndices.size(); ++k)
                structural(g.noteIndices[k] < phrase->notes.size() && groupedNotes.insert(g.noteIndices[k]).second
                    && (k == 0 || g.noteIndices[k] == g.noteIndices[k-1]+1),
                    "phrase.approaches", "Approach indices must be in range, consecutive and nonoverlapping.");
        }
        out.readyForSearch = complete && out.structurallyValid;
    }
    else structural(false,"content","Missing musical payload.");

    // Publication requirements are separate from private draft storage.
    bool distributionMetadata = record.domain == LibraryDomain::common
        && hasText(record.name) && hasText(record.explanation)
        && hasText(record.source.source) && hasText(record.source.author)
        && hasText(record.source.license) && hasText(record.source.permissionEvidence)
        && record.source.permission != ContentPermission::unknown;
    if (!distributionMetadata)
        report(LibraryValidationScope::distribution,"source","Common distribution requires title, explanation, source, author and explicit permission/license evidence.");
    out.readyForSearch = out.readyForSearch && out.structurallyValid;
    out.distributable = distributionMetadata && out.readyForSearch;
    return out;
}
}
