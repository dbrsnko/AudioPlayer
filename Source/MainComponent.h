#pragma once

#include <JuceHeader.h>


class ListBoxComponent : public juce::Component,
                         public juce::ListBoxModel
                         
{
private:
    std::unique_ptr<juce::Array<juce::File>> source;
    juce::ListBox listBox;
public:
    ListBoxComponent();
    //ListBoxComponent(juce::Array<juce::File>);
    void formTracklist();
    void setSource(juce::Array<juce::File>); //mb overload it later to take pointers as parameter
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void resized() override;

    //juce::Array<juce::File> directories; //places where tracks are located
    juce::Array<juce::File> tracklist; //tracks extracted from directories


};



//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
    public juce::ChangeListener,
    public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
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
    enum DragState {
        Dragging,
        NotDragging
    };



    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Slider progressSlider;
    juce::Label currentPositionLabel;
    juce::Label totalLengthLabel;
    juce::Label hostLabel; //purely for making a layout, providing a space for two labels
    


    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void progressSliderDragStart();
    void progressSliderDragEnd();
    void changeState(TransportState newState);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;
    


    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    TransportState state;
    DragState dragState;

    ListBoxComponent listBox;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
