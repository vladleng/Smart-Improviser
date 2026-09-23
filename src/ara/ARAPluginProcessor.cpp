#include "ara/ARAPluginProcessor.h"
#include "ara/ARAContextDocumentController.h"
#include "context/SharedHarmonicContext.h"

#include <cmath>

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

void SmartImproviserARAProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    static constexpr char state[] = "SmartImproviserARAStateV1";
    destData.replaceAll(state, sizeof(state));
}

void SmartImproviserARAProcessor::setStateInformation(const void*, int) {}

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
