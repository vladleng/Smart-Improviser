#include "ara/ARAPluginProcessor.h"
#include "ara/ARAPluginEditor.h"
#include "ara/ARAContextDocumentController.h"
#include "context/SharedHarmonicContext.h"

#include "ara/PluginViewState.h"
#include "core/analysis/CommonVocabulary.h"
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
    const auto catalog = userLibrary.initializeCommonCatalog(smartimproviser::harmony::makeCommonVocabulary());
    jassert(catalog.succeeded());
    reloadUserLibrary();
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
    PluginViewState state;
    state.width = savedEditorSize.x; state.height = savedEditorSize.y;
    state.labels = manualTensions;
    if (const auto shared = sharedManualTensions.snapshot())
        for (const auto& [key, level] : *shared) state.labels[key] = level;
    state.degreeLabels = fretDegreeLabels;
    state.tensionFilter = requestedTensionFilter;
    state.write(destData);
}

void SmartImproviserARAProcessor::setStateInformation(const void* data, int size)
{
    const auto state = PluginViewState::read(data, size);
    if (!state) return;
    const juce::ScopedLock lock(stateLock);
    sharedManualTensions.importMissing(state->labels);
    manualTensions = state->labels;
    refreshSharedManualTensions();
    fretDegreeLabels = state->degreeLabels;
    requestedTensionFilter = state->tensionFilter;
    savedEditorSize = {state->width, state->height};
}

int SmartImproviserARAProcessor::tensionFilter() const
{
    const juce::ScopedLock lock(stateLock);
    return requestedTensionFilter;
}

void SmartImproviserARAProcessor::setTensionFilter(int level)
{
    const juce::ScopedLock lock(stateLock);
    requestedTensionFilter = smartimproviser::harmony::normalizedTensionFilter(level);
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

smartimproviser::harmony::LibraryStorageResult SmartImproviserARAProcessor::reloadUserLibrary()
{
    const juce::ScopedLock lock(stateLock);
    lastLibraryLoad = sharedUserLibrary.loadInto(userLibrary);
    return lastLibraryLoad; // Invalid/future files preserve the previous userLibrary value.
}
smartimproviser::harmony::LibraryUserSnapshot SmartImproviserARAProcessor::userLibrarySnapshot() const
{
    const juce::ScopedLock lock(stateLock);
    return userLibrary.userSnapshot();
}
smartimproviser::harmony::LibraryStorageStatus SmartImproviserARAProcessor::userLibraryStorageStatus() const
{
    const juce::ScopedLock lock(stateLock);
    return lastLibraryLoad.status;
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

std::vector<smartimproviser::harmony::LibraryRecord> SmartImproviserARAProcessor::libraryRecords(
    smartimproviser::harmony::LibraryDomain domain) const
{
    const juce::ScopedLock lock(stateLock);
    return userLibrary.list(domain);
}
smartimproviser::harmony::LibraryStorageResult SmartImproviserARAProcessor::editLibrary(
    LibraryEdit edit, const smartimproviser::harmony::LibraryRecord& draft,
    smartimproviser::harmony::LibraryRecord& saved)
{
    using namespace smartimproviser::harmony;
    const juce::ScopedLock lock(stateLock);
    if (!lastLibraryLoad.succeeded()) return lastLibraryLoad;
    auto candidate=userLibrary; // Commit detached state only after durable write.
    LibraryOperation operation;
    if (edit==LibraryEdit::save)
        operation=draft.id.empty() ? candidate.addUser(draft) : candidate.updateUser(draft,draft.revision);
    else {
        LibraryItemReference source{draft.domain,draft.id,draft.revision};
        operation=edit==LibraryEdit::copy ? candidate.copyToUser(source) : candidate.createVariant(source);
    }
    if (!operation.succeeded()) {
        LibraryStorageResult failure; failure.status=LibraryStorageStatus::invalidData;
        failure.explanation=operation.explanation; return failure;
    }
    auto written=sharedUserLibrary.commitFrom(candidate,lastLibraryLoad.stamp);
    if (written.succeeded()) {
        userLibrary=std::move(candidate); lastLibraryLoad=written; saved=*operation.record;
    }
    return written;
}

smartimproviser::harmony::LibraryStorageResult SmartImproviserARAProcessor::importLibrary(std::span<const std::uint8_t> bytes) {
    const juce::ScopedLock lock(stateLock);
    auto result=sharedUserLibrary.importData(bytes);
    if(result.succeeded())lastLibraryLoad=sharedUserLibrary.loadInto(userLibrary);
    return result.succeeded()?lastLibraryLoad:result;
}
