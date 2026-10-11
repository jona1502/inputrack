#include <inputrack/MasterMuteProcessor.h>
#include <cmath>
#include <iostream>

namespace {
int failures = 0;
void expect(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool closeTo(float actual, float expected)
{
    return std::abs(actual - expected) < 0.000001f;
}
}

int main()
{
    std::atomic<bool> muted{false};
    inputrack::MasterMuteProcessor processor(muted);
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buffer(2, 4);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, 0.25f * static_cast<float>(sample + 1));

    processor.processBlock(buffer, midi);
    expect(closeTo(buffer.getSample(0, 3), 1.0f), "unmuted audio passes through unchanged");

    muted.store(true);
    processor.processBlock(buffer, midi);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            expect(closeTo(buffer.getSample(channel, sample), 0.0f), "muted audio is silent");

    muted.store(false);
    buffer.setSample(0, 0, 0.75f);
    processor.processBlock(buffer, midi);
    expect(closeTo(buffer.getSample(0, 0), 0.75f), "unmuting restores pass-through");

    if (failures == 0) std::cout << "MasterMuteProcessorTests passed\n";
    return failures == 0 ? 0 : 1;
}
