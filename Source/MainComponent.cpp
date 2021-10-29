#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
       addAndMakeVisible (frequencySlider);
        frequencySlider.setRange (20.0, 20000.0);
        frequencySlider.setSkewFactorFromMidPoint (500.0);
        frequencySlider.setValue (currentFrequency, juce::dontSendNotification);
        frequencySlider.onValueChange = [this] { targetFrequency = frequencySlider.getValue(); };

        addAndMakeVisible (levelSlider);
        levelSlider.setRange (0.0, 1.0);
        levelSlider.setValue ((double) currentLevel, juce::dontSendNotification);
        levelSlider.onValueChange = [this] { targetLevel = (float) levelSlider.getValue(); };

        setSize(400,400);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
        currentSampleRate = sampleRate;
        updateAngleDelta();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
        auto* leftBuffer  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

        auto localTargetFrequency = targetFrequency;

        if (targetFrequency != currentFrequency)
        {
            auto frequencyIncrement = (targetFrequency - currentFrequency) / bufferToFill.numSamples;

            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                auto currentSample = (float) std::sin (currentAngle);
                currentFrequency += frequencyIncrement;
                updateAngleDelta();
                currentAngle += angleDelta;
                leftBuffer[sample]  = currentSample;
                rightBuffer[sample] = currentSample;
            }

            currentFrequency = localTargetFrequency;
        }
        else
        {
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                auto currentSample = (float) std::sin (currentAngle);
                currentAngle += angleDelta;
                leftBuffer[sample]  = currentSample;
                rightBuffer[sample] = currentSample;
            }
        }

        auto localTargetLevel = targetLevel;
        bufferToFill.buffer->applyGainRamp (bufferToFill.startSample, bufferToFill.numSamples, currentLevel, localTargetLevel);
        currentLevel = localTargetLevel;
}

void MainComponent::releaseResources()
{
    juce::Logger::getCurrentLogger()->writeToLog ("Releasing audio resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
        frequencySlider.setBounds (10, 10, getWidth() - 20, 20);
        levelSlider    .setBounds (10, 40, getWidth() - 20, 20);
}

void MainComponent::updateAngleDelta()
{
        auto cyclesPerSample = currentFrequency / currentSampleRate;
        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;//[3]
}
