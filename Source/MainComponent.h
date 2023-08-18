#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    enum TransportState
    {
        Stopped, //Audio playback is stopped and ready to be started
        Starting, //Audio playback hasn't yet started but it has been told to start
        Playing, //Audio is playing
        Stopping, //Audio is playing but playback has been told to stop, after this it will return to the Stopped 
        Pausing, //Similar to Stop/Stopping
        Paused
    };

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void changeState(TransportState newState);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};