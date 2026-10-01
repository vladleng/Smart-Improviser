#pragma once
#include <juce_core/juce_core.h>
#include "core/analysis/TensionFilter.h"
#include <map>
#include <optional>
#include <string>

// Instance preferences and a recovery snapshot of shared assignments.
struct PluginViewState
{
    int width = 1120, height = 1000;
    std::map<std::string, int> labels;
    bool degreeLabels = false;
    int tensionFilter = 0;

    void write(juce::MemoryBlock& data) const
    {
        juce::MemoryOutputStream stream(data, false);
        stream.writeString("SmartImproviserARAStateV5");
        stream.writeInt(width); stream.writeInt(height);
        stream.writeInt(static_cast<int>(labels.size()));
        for (const auto& [key, level] : labels)
        {
            stream.writeString(juce::String::fromUTF8(key.c_str()));
            stream.writeInt(level);
        }
        stream.writeByte(degreeLabels ? 1 : 0);
        stream.writeByte(static_cast<char>(smartimproviser::harmony::normalizedTensionFilter(tensionFilter)));
    }

    static std::optional<PluginViewState> read(const void* data, int size)
    {
        if (!data || size <= 0) return std::nullopt;
        juce::MemoryInputStream stream(data, static_cast<std::size_t>(size), false);
        const auto version = stream.readString();
        if (version != "SmartImproviserARAStateV2" && version != "SmartImproviserARAStateV3"
            && version != "SmartImproviserARAStateV4" && version != "SmartImproviserARAStateV5")
            return std::nullopt;
        if (stream.getNumBytesRemaining() < 12) return std::nullopt;
        PluginViewState state;
        state.width = stream.readInt(); state.height = stream.readInt();
        const int count = stream.readInt();
        if (state.width < 940 || state.width > 1900 || state.height < 1000 || state.height > 1800
            || count < 0 || count > 10000) return std::nullopt;
        const bool tombstones = version == "SmartImproviserARAStateV4" || version == "SmartImproviserARAStateV5";
        for (int i = 0; i < count; ++i)
        {
            if (stream.isExhausted()) return std::nullopt;
            auto key = stream.readString().toStdString();
            if (stream.getNumBytesRemaining() < 4) return std::nullopt;
            const int level = stream.readInt();
            if (!key.empty() && level >= (tombstones ? 0 : 1) && level <= 3)
                state.labels[std::move(key)] = level;
        }
        state.degreeLabels = version != "SmartImproviserARAStateV2" && !stream.isExhausted() && stream.readByte() != 0;
        state.tensionFilter = version == "SmartImproviserARAStateV5" && !stream.isExhausted()
            ? smartimproviser::harmony::normalizedTensionFilter(stream.readByte()) : 0;
        return state;
    }
};
