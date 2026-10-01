#include "ara/ARAPluginProcessor.h"
#include "ara/ARAPluginEditor.h"
#include "ara/ARAContextDocumentController.h"
#include "context/SharedHarmonicContext.h"

#include <cmath>
#include <utility>

namespace
{
constexpr double transportComparisonEpsilon = 1.0e-9;

bool transportValueChanged(double current, double previous) noexcept
{
    return std::abs(current - previous) > transportComparisonEpsilon;
}
}

SmartImproviserARAProcessor::SmartImproviserARAProcessor()
    : juce::AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    SharedHarmonicContextBridge::instance().prepareWriter();
    refreshSharedManualTensions();
}

void SmartImproviserARAProcessor::prepareToPlay(double, int) {}
void SmartImproviserARAProcessor::releaseResources() {}

bool SmartImproviserARAProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();
    if (input != output)
        return false;

    return output == juce::AudioChannelSet::mono()
        || output == juce::AudioChannelSet::stereo();
}

void SmartImproviserARAProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    double seconds = -1.0;
    double ppq = -1.0;
    bool playing = false;
    bool transportAvailable = false;

    if (auto* hostPlayHead = getPlayHead())
    {
        if (const auto position = hostPlayHead->getPosition())
        {
            if (const auto timeInSeconds = position->getTimeInSeconds())
            {
                seconds = *timeInSeconds;
                transportAvailable = true;
            }
            if (const auto ppqPosition = position->getPpqPosition())
            {
                ppq = *ppqPosition;
                transportAvailable = true;
            }
            playing = position->getIsPlaying();
        }
    }

    const auto transportChanged = ! hasPublishedTransport
                               || transportAvailable != lastPublishedTransportAvailable
                               || playing != lastPublishedTransportPlaying
                               || transportValueChanged(seconds, lastPublishedTransportSeconds)
                               || transportValueChanged(ppq, lastPublishedTransportPpq);

    if (transportChanged)
    {
        SharedHarmonicContextBridge::instance().publishTransport(
            transportAvailable, seconds, ppq, playing);

        hasPublishedTransport = true;
        lastPublishedTransportAvailable = transportAvailable;
        lastPublishedTransportPlaying = playing;
        lastPublishedTransportSeconds = seconds;
        lastPublishedTransportPpq = ppq;
    }

    juce::ignoreUnused(buffer, midiMessages);
}

juce::AudioProcessorEditor* SmartImproviserARAProcessor::createEditor()
{
    return new SmartImproviserARAEditor(*this);
}

void SmartImproviserARAProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const juce::ScopedLock lock(stateLock);
    juce::MemoryOutputStream stream(destData, false);
    stream.writeString("SmartImproviserARAStateV4");
    auto labels = manualTensions;
    if (const auto shared = sharedManualTensions.snapshot())
        for (const auto& [key, level] : *shared) labels[key] = level;
    stream.writeInt(savedEditorSize.x);
    stream.writeInt(savedEditorSize.y);
    stream.writeInt(static_cast<int>(labels.size()));
    for (const auto& [key, level] : labels)
    {
        stream.writeString(juce::String::fromUTF8(key.c_str()));
        stream.writeInt(level);
    }
    stream.writeByte(fretDegreeLabels ? 1 : 0);
}

void SmartImproviserARAProcessor::setStateInformation(const void* data, int size)
{
    if (data == nullptr || size <= 0) return;
    juce::MemoryInputStream stream(data, static_cast<std::size_t>(size), false);
    const auto stateVersion = stream.readString();
    if (stateVersion != "SmartImproviserARAStateV2"
        && stateVersion != "SmartImproviserARAStateV3"
        && stateVersion != "SmartImproviserARAStateV4") return;
    const auto width = stream.readInt(), height = stream.readInt();
    const auto count = stream.readInt();
    if (width < 940 || width > 1900 || height < 1000 || height > 1800
        || count < 0 || count > 10000) return;
    std::map<std::string, int> restored;
    for (int i = 0; i < count; ++i)
    {
        if (stream.isExhausted()) return;
        auto key = stream.readString().toStdString();
        if (stream.isExhausted()) return;
        const auto level = stream.readInt();
        if (!key.empty() && level >= (stateVersion == "SmartImproviserARAStateV4" ? 0 : 1) && level <= 3)
            restored[std::move(key)] = level;
    }
    const bool restoredFretDegreeLabels = stateVersion != "SmartImproviserARAStateV2"
        && !stream.isExhausted() && stream.readByte() != 0;
    const juce::ScopedLock lock(stateLock);
    sharedManualTensions.importMissing(restored);
    manualTensions = std::move(restored);
    refreshSharedManualTensions();
    fretDegreeLabels = restoredFretDegreeLabels;
    savedEditorSize = {width, height};
}

void SmartImproviserARAProcessor::refreshSharedManualTensions()
{
    const juce::ScopedLock lock(stateLock);
    if (const auto shared = sharedManualTensions.snapshot())
        for (const auto& [key, level] : *shared) manualTensions[key] = level;
}

int SmartImproviserARAProcessor::manualTensionFor(const std::string& key) const
{
    const juce::ScopedLock lock(stateLock);
    const auto found = manualTensions.find(key);
    return found == manualTensions.end() ? 0 : found->second;
}

bool SmartImproviserARAProcessor::setManualTension(const std::string& key, int level)
{
    const juce::ScopedLock lock(stateLock);
    if (!sharedManualTensions.assign(key, level)) return false;
    manualTensions[key] = level;
    return true;
}

bool SmartImproviserARAProcessor::fretDegreeLabelsEnabled() const
{
    const juce::ScopedLock lock(stateLock);
    return fretDegreeLabels;
}

void SmartImproviserARAProcessor::setFretDegreeLabelsEnabled(bool enabled)
{
    const juce::ScopedLock lock(stateLock);
    fretDegreeLabels = enabled;
}

juce::Point<int> SmartImproviserARAProcessor::editorSize() const
{
    const juce::ScopedLock lock(stateLock);
    return savedEditorSize;
}

void SmartImproviserARAProcessor::setEditorSize(juce::Point<int> size)
{
    const juce::ScopedLock lock(stateLock);
    savedEditorSize = size;
}

#if JucePlugin_Enable_ARA
void SmartImproviserARAProcessor::didBindToARA() noexcept
{
    juce::AudioProcessorARAExtension::didBindToARA();
    araBound.store(true, std::memory_order_relaxed);
}
#endif

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmartImproviserARAProcessor();
}

#if JucePlugin_Enable_ARA
const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory()
{
    return juce::ARADocumentControllerSpecialisation::createARAFactory<SmartImproviserARADocumentController>();
}
#endif
