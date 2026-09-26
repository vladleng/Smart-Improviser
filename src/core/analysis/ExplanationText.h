#pragma once

#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/HarmonicFunction.h"
#include "core/model/KeyModel.h"

#include <sstream>
#include <string>

namespace smartimproviser::harmony
{
namespace explanation_text_detail
{
inline const char* scopeName(ExplanationContextScope scope) noexcept
{
    switch (scope)
    {
        case ExplanationContextScope::global: return "GLOBAL";
        case ExplanationContextScope::local: return "LOCAL";
        case ExplanationContextScope::modal: return "MODAL";
        default: return "CONTEXT";
    }
}

inline const char* stateMark(ExplanationEvidenceState state) noexcept
{
    switch (state)
    {
        case ExplanationEvidenceState::confirmed: return "[OK]";
        case ExplanationEvidenceState::missing: return "[MISSING]";
        case ExplanationEvidenceState::implied: return "[IMPLIED]";
        case ExplanationEvidenceState::contradicted: return "[CONFLICT]";
        case ExplanationEvidenceState::ambiguous: return "[AMBIGUOUS]";
        case ExplanationEvidenceState::neutral:
        default: return "[INFO]";
    }
}

inline std::string keyName(const KeyCenter& center)
{
    if (! center.valid || ! center.key.valid)
        return "-";

    // Keep this formatter host-neutral and deterministic. Enharmonic spelling
    // comes from rootFifths; normalizedChordSymbol handles chord spelling.
    static constexpr const char* fifthNames[] =
    {
        "Cb", "Gb", "Db", "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B", "F#", "C#"
    };
    const auto fifths = center.key.rootFifths;
    std::string root;
    if (fifths >= -7 && fifths <= 7)
        root = fifthNames[static_cast<std::size_t>(fifths + 7)];
    else
        root = std::to_string(center.key.rootPitchClass);
    return root + " " + keyModeName(center.key.mode);
}

inline std::string noteName(const MaterialNote& note)
{
    if (! note.spelling.empty())
        return note.spelling;
    return std::to_string(note.pitchClass);
}
}

// Diagnostic presentation adapter for 0.3g. Canonical explanation data stays
// in ExplanationResult; this helper only serializes it for the current text UI.
inline std::string explanationDiagnosticText(const ImprovisationResult& result)
{
    const auto explanation = explainImprovisation(result);
    if (! explanation.valid)
        return {};

    std::ostringstream out;
    out << "ПОЧЕМУ / КОНТЕКСТ\n";
    for (const auto& layer : explanation.contextLayers)
    {
        out << explanation_text_detail::scopeName(layer.scope) << ": "
            << explanation_text_detail::keyName(layer.center) << " • "
            << scaleDegreeName(layer.harmonic.rootScaleDegree) << " • "
            << harmonicFunctionName(layer.harmonic.effectiveFunction) << "\n";
    }

    for (std::size_t itemIndex = 0; itemIndex < explanation.items.size(); ++itemIndex)
    {
        const auto& item = explanation.items[itemIndex];
        out << "\nИДЕЯ " << (itemIndex + 1) << ": " << item.idea << "\n";
        if (item.source.kind != MaterialKind::undefined)
            out << "ИСТОЧНИК: " << item.source.name << "\n";

        if (! item.importantNotes.empty())
        {
            out << "ВАЖНЫЕ НОТЫ: ";
            for (const auto& note : item.importantNotes)
                out << explanation_text_detail::noteName(note) << " ";
            out << "\n";
        }

        if (item.resolution.available && item.resolution.targetChord.valid)
        {
            out << "ЦЕЛЬ: " << normalizedChordSymbol(item.resolution.targetChord)
                << (item.resolution.confirmed ? " [OK]" : " [EXPECTED]") << "\n";
        }
        else if (item.targetChord.valid)
        {
            out << "СЛЕДУЮЩИЙ АККОРД: " << normalizedChordSymbol(item.targetChord) << "\n";
        }

        if (! item.interpretationIndices.empty())
        {
            out << "ТРАКТОВКИ: ";
            for (const auto index : item.interpretationIndices)
                out << (index + 1) << " ";
            out << "\n";
        }

        if (! item.why.empty())
        {
            out << "ПОЧЕМУ:\n";
            for (const auto& evidence : item.why)
            {
                out << "  " << explanation_text_detail::stateMark(evidence.state)
                    << " " << evidence.ruleId;
                if (evidence.chord.valid)
                    out << " • " << normalizedChordSymbol(evidence.chord);
                else if (evidence.pitchClass >= 0)
                    out << " • pc=" << evidence.pitchClass;
                if (evidence.pattern.recognized())
                    out << " • pattern=" << static_cast<int>(evidence.pattern.type)
                        << " " << (evidence.pattern.positionIndex + 1)
                        << "/" << evidence.pattern.length;
                out << "\n";
            }
        }
    }

    return out.str();
}
}
