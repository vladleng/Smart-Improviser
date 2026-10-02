#pragma once
#include "core/model/LibraryRecord.h"
#include <charconv>
#include <cmath>
#include <string_view>

namespace smartimproviser::harmony
{
struct IdeaNoteInput { std::string degree, octave, beat, duration, slot; };
struct IdeaNoteInputResult { IdeaNote note; std::string error; };
inline IdeaNoteInputResult parseIdeaNoteInput(const IdeaNoteInput& input, IdeaNote note = {})
{
    IdeaNoteInputResult result;
    const auto integer = [&](const std::string& text, int& value) {
        const auto parsed = std::from_chars(text.data(), text.data()+text.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == text.data()+text.size();
    };
    if (input.degree.empty()) {
        if (!input.slot.empty()) { result.error="Slot requires a degree."; return result; }
        note.pitch.reset();
    } else {
        std::string_view degree(input.degree); int accidental=0;
        while (!degree.empty() && (degree.front()=='b' || degree.front()=='#')) {
            accidental += degree.front()=='b' ? -1 : 1; degree.remove_prefix(1);
        }
        int number=0, slot=-1;
        if (!integer(std::string(degree), number) || number<1 || number>13 || accidental < -2 || accidental > 2
            || (!input.slot.empty() && (!integer(input.slot,slot) || slot<0))) {
            result.error="Degree: 1..13, b/#; slot: non-negative integer."; return result;
        }
        note.pitch=RelativePitch{slot,number,accidental};
    }
    if (input.octave.empty()) note.octaveOffset.reset();
    else {
        int octave=0;
        if (!integer(input.octave,octave) || octave < -11 || octave > 10) {
            result.error="Octave offset: -11..10."; return result;
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
        result.error="Beat must be >=0; duration >0 (decimal point)."; return result;
    }
    result.note=note; return result;
}
}
