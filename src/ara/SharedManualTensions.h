#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <optional>
#include <string>

// User preferences shared by all plugin instances/projects. Read-modify-write is
// protected across host processes; writes replace the file through a temporary file.
class SharedManualTensions
{
public:
    using Labels = std::map<std::string, int>;

    static juce::File defaultFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("Moon River Studio/Smart Improviser/manual-tensions.xml");
    }

    explicit SharedManualTensions(juce::File file = defaultFile())
        : storage(std::move(file)), processLock("SmartImproviserTensions-"
            + juce::String::toHexString(storage.getFullPathName().hashCode64())) {}

    std::optional<Labels> snapshot() const
    {
        const juce::ScopedLock guard(threadLock);
        Lock guardProcess(processLock);
        if (!guardProcess.entered) return std::nullopt;
        return read();
    }

    bool assign(const std::string& key, int level)
    {
        if (key.empty() || key.size() > 4096 || level < 0 || level > 3) return false;
        const juce::ScopedLock guard(threadLock);
        Lock guardProcess(processLock);
        if (!guardProcess.entered) return false;
        auto labels = read();
        if (!labels) return false; // Never overwrite a damaged/unrecognized file.
        (*labels)[key] = level; // Zero is a tombstone: old projects cannot resurrect it.
        return write(*labels);
    }

    bool importMissing(const Labels& projectLabels)
    {
        const juce::ScopedLock guard(threadLock);
        Lock guardProcess(processLock);
        if (!guardProcess.entered) return false;
        auto labels = read();
        if (!labels) return false;
        bool changed = false;
        for (const auto& [key, level] : projectLabels)
            if (!key.empty() && key.size() <= 4096 && level >= 0 && level <= 3)
                changed = labels->emplace(key, level).second || changed;
        return !changed || write(*labels);
    }

private:
    struct Lock
    {
        explicit Lock(juce::InterProcessLock& value) : lock(value), entered(lock.enter(2000)) {}
        ~Lock() { if (entered) lock.exit(); }
        juce::InterProcessLock& lock;
        bool entered;
    };

    std::optional<Labels> read() const
    {
        if (!storage.exists()) return Labels{};
        if (!storage.existsAsFile() || storage.getSize() > 16 * 1024 * 1024) return std::nullopt;
        auto xml = juce::XmlDocument::parse(storage);
        if (!xml || !xml->hasTagName("SmartImproviserManualTensions")
            || xml->getIntAttribute("version") != 1 || xml->getNumChildElements() > 10000)
            return std::nullopt;
        Labels labels;
        for (auto* entry : xml->getChildIterator())
        {
            const auto key = entry->getStringAttribute("key").toStdString();
            const auto value = entry->getStringAttribute("level");
            if (!entry->hasTagName("label") || key.empty() || key.size() > 4096
                || value.length() != 1 || !value.containsOnly("0123")) return std::nullopt;
            labels[key] = value.getIntValue();
        }
        return labels;
    }

    bool write(const Labels& labels) const
    {
        if (labels.size() > 10000 || storage.getParentDirectory().createDirectory().failed()) return false;
        juce::XmlElement xml("SmartImproviserManualTensions");
        xml.setAttribute("version", 1);
        for (const auto& [key, level] : labels)
        {
            auto* entry = xml.createNewChildElement("label");
            entry->setAttribute("key", juce::String::fromUTF8(key.c_str()));
            entry->setAttribute("level", level);
        }
        juce::TemporaryFile temporary(storage);
        return xml.writeTo(temporary.getFile()) && temporary.overwriteTargetFileWithTemporary();
    }

    juce::File storage;
    inline static juce::CriticalSection threadLock; // POSIX file locks alone do not separate threads.
    mutable juce::InterProcessLock processLock;
};
