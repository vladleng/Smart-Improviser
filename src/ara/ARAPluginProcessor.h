#pragma once

#include <JuceHeader.h>
#include "ara/SharedManualTensions.h"
#include "library/SharedUserLibrary.h"
#include <atomic>
#include <map>
#include <string>

class SmartImproviserARAProcessor final : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
                                        , public juce::AudioProcessorARAExtension
#endif
{
public:
    SmartImproviserARAProcessor();
    ~SmartImproviserARAProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool isAraBound() const noexcept { return araBound.load(std::memory_order_relaxed); }
    int tensionFilter() const;
    void setTensionFilter(int level);
    void refreshSharedManualTensions();
    int manualTensionFor(const std::string& key) const;
    bool setManualTension(const std::string& key, int level);
    smartimproviser::harmony::LibraryStorageResult reloadUserLibrary();
    smartimproviser::harmony::LibraryUserSnapshot userLibrarySnapshot() const;
    smartimproviser::harmony::LibraryStorageStatus userLibraryStorageStatus() const;
    enum class LibraryEdit { save, copy, variant };
    std::vector<smartimproviser::harmony::LibraryRecord> libraryRecords(smartimproviser::harmony::LibraryDomain) const;
    smartimproviser::harmony::LibraryStorageResult editLibrary(LibraryEdit,
        const smartimproviser::harmony::LibraryRecord&, smartimproviser::harmony::LibraryRecord& saved);
    bool fretDegreeLabelsEnabled() const;
    void setFretDegreeLabelsEnabled(bool enabled);
    juce::Point<int> editorSize() const;
    void setEditorSize(juce::Point<int> size);

protected:
#if JucePlugin_Enable_ARA
    void didBindToARA() noexcept override;
#endif

private:
    std::atomic<bool> araBound { false };
    mutable juce::CriticalSection stateLock;
    std::map<std::string, int> manualTensions; // Host backup for migration/recovery.
    SharedManualTensions sharedManualTensions;
    smartimproviser::harmony::SharedUserLibrary sharedUserLibrary;
    smartimproviser::harmony::PhraseLibrary userLibrary;
    smartimproviser::harmony::LibraryStorageResult lastLibraryLoad;
    bool fretDegreeLabels = false;
    int requestedTensionFilter = 0;
    juce::Point<int> savedEditorSize {1120, 1000};
    bool hasPublishedTransport = false;
    bool lastPublishedTransportAvailable = false;
    bool lastPublishedTransportPlaying = false;
    double lastPublishedTransportSeconds = -1.0;
    double lastPublishedTransportPpq = -1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmartImproviserARAProcessor)
};
