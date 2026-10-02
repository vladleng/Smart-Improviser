#pragma once
#include "core/model/LibraryRecord.h"
#include <charconv>
#include <cmath>
#include <string_view>
#include <limits>

namespace smartimproviser::harmony
{
struct IdeaNoteInput { std::string degree, octave, beat, duration, slot; };
struct IdeaNoteInputResult { IdeaNote note; std::string error; };
inline IdeaNoteInputResult parseIdeaNoteInput(const IdeaNoteInput& input, IdeaNote note = {})
{
    IdeaNoteInputResult result;
    const auto integer = [&](const std::string& text, auto& value) {
        const auto parsed = std::from_chars(text.data(), text.data()+text.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == text.data()+text.size();
    };
    if (input.degree.empty()) {
        if (!input.slot.empty()) { result.error="Аккорд можно указать только вместе со ступенью."; return result; }
        note.pitch.reset();
    } else {
        std::string_view degree(input.degree); int accidental=0;
        while (!degree.empty() && (degree.front()=='b' || degree.front()=='#')) {
            accidental += degree.front()=='b' ? -1 : 1; degree.remove_prefix(1);
        }
        int number=0; std::int64_t slot=-1;
        if (!integer(std::string(degree), number) || number<1 || number>13 || accidental < -2 || accidental > 2
            || (!input.slot.empty() && (!integer(input.slot,slot) || slot<1 || slot>static_cast<std::int64_t>(std::numeric_limits<int>::max())+1))) {
            result.error="Ступень: 1–13 с b/#; аккорд: целое число от 1."; return result;
        }
        note.pitch=RelativePitch{input.slot.empty() ? -1 : static_cast<int>(slot-1),number,accidental};
    }
    if (input.octave.empty()) note.octaveOffset.reset();
    else {
        int octave=0;
        if (!integer(input.octave,octave) || octave < -11 || octave > 10) {
            result.error="Октавное смещение: −11…10."; return result;
        }
        note.octaveOffset=octave;
    }
    const auto decimal = [&](const std::string& text, std::optional<double>& value, bool positive) {
        if (text.empty()) { value.reset(); return true; }
        double number=0;
        const auto parsed=std::from_chars(text.data(),text.data()+text.size(),number);
        if (parsed.ec!=std::errc{} || parsed.ptr!=text.data()+text.size() || !std::isfinite(number)
            || (positive ? number<=0 : number<0)) return false;
        value=number; return true;
    };
    if (!decimal(input.beat,note.beatOffset,false) || !decimal(input.duration,note.durationBeats,true)) {
        result.error="Начало: от 0; длительность: больше 0 (десятичная точка)."; return result;
    }
    result.note=note; return result;
}
}
