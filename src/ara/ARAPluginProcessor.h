#pragma once

#include <JuceHeader.h>
#include <atomic>

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

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

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

protected:
#if JucePlugin_Enable_ARA
    void didBindToARA() noexcept override;
#endif

private:
    std::atomic<bool> araBound { false };
    bool hasPublishedTransport = false;
    bool lastPublishedTransportAvailable = false;
    bool lastPublishedTransportPlaying = false;
    double lastPublishedTransportSeconds = -1.0;
    double lastPublishedTransportPpq = -1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmartImproviserARAProcessor)
};
