#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : state(Stopped),dragState(NotDragging)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
    

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
        setAudioChannels (0, 2);
    }

    formatManager.registerBasicFormats();

    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible(&progressSlider);
    progressSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    progressSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 25); 
    progressSlider.setEnabled(false);
    progressSlider.onDragStart = [this] {progressSliderDragStart(); };
    progressSlider.onDragEnd = [this] {progressSliderDragEnd(); };
    
    addAndMakeVisible(&currentPositionLabel);
    currentPositionLabel.setText("Stopped", juce::dontSendNotification);
    addAndMakeVisible(&totalLengthLabel);
    totalLengthLabel.setJustificationType(juce::Justification::right);
    totalLengthLabel.setText("00:00", juce::dontSendNotification);
    addAndMakeVisible(&hostLabel);

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);

    transportSource.addChangeListener(this);
    startTimer(20); //20 ms interval
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
    //progressSliderprogressSliderprogressSliderprogressSliderprogressSliderprogressSlider
    // Your audio-processing code goes here!
    // For more details, see the help for AudioProcessor::getNextAudioBlock()
    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
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
    auto area = getLocalBounds();
    auto contentItemHeight = 30;

    auto margin = 5;

    openButton.setBounds(area.removeFromTop(contentItemHeight));
    auto labelArea = area.removeFromTop(contentItemHeight);

    hostLabel.setBounds(labelArea);
    currentPositionLabel.setBounds(labelArea.removeFromLeft(labelArea.getWidth()/2));
    totalLengthLabel.setBounds(labelArea.removeFromRight(labelArea.getWidth()));

    progressSlider.setBounds(area.removeFromTop(contentItemHeight)); 
    playButton.setBounds(area.removeFromTop(contentItemHeight));
    stopButton.setBounds(area.removeFromTop(contentItemHeight));
    
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) 
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState(Playing);
        else if ((state == Stopping) || (state == Playing))
            changeState(Stopped);
        else if (Pausing == state)
            changeState(Paused);
    }
}
void MainComponent::timerCallback()
{
    
        if (transportSource.isPlaying())
        {
            juce::RelativeTime position(transportSource.getCurrentPosition());
            auto minutes = ((int)position.inMinutes()) % 60;
            auto seconds = ((int)position.inSeconds()) % 60;
            auto positionString = juce::String::formatted("%02d:%02d", minutes, seconds);
            currentPositionLabel.setText(positionString, juce::dontSendNotification);
            progressSlider.setValue(transportSource.getCurrentPosition());

        }
        else
        {   
            if (dragState == Dragging) {
                juce::RelativeTime position(progressSlider.getValue());
                auto minutes = ((int)position.inMinutes()) % 60;
                auto seconds = ((int)position.inSeconds()) % 60;
                auto positionString = juce::String::formatted("%02d:%02d", minutes, seconds);
                currentPositionLabel.setText(positionString, juce::dontSendNotification);
            }
            else {
                progressSlider.setValue(transportSource.getCurrentPosition());
                if (transportSource.getCurrentPosition() == 0)
                    currentPositionLabel.setText("00:00", juce::dontSendNotification);
            }
            

        }
    
}
void MainComponent::changeState(TransportState newState){
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
        case Stopped:
            playButton.setButtonText("Play");
            stopButton.setEnabled(false);
            transportSource.setPosition(0.0);
            break;

        case Starting:
            transportSource.start();
            break;

        case Playing:
            playButton.setButtonText("Pause");
            stopButton.setEnabled(true);
            break;

        case Pausing:
            transportSource.stop();
            break;

        case Paused:
            if(dragState == NotDragging)
            playButton.setButtonText("Resume");
            break;

        case Stopping:
            transportSource.stop();
            break;
        }
    }
}

void MainComponent::openButtonClicked() {
    chooser = std::make_unique<juce::FileChooser>("Select a file to play...",
        juce::File{},
        "*.wav,*.mp3,*.aiff");                     
    auto chooserFlags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)     
        {
            auto file = fc.getResult();

            if (file != juce::File{})                                               
            {
                auto* reader = formatManager.createReaderFor(file);                

                if (reader != nullptr)
                {
                    changeState(Stopped);//changes state so no play blocking occurs (if song is playing during selection of another song it causes soft block)
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);                     
                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       
                    playButton.setEnabled(true);
                    progressSlider.setEnabled(true);

                    juce::RelativeTime position(transportSource.getLengthInSeconds());
                    auto minutes = ((int)position.inMinutes()) % 60;
                    auto seconds = ((int)position.inSeconds()) % 60;
                    auto positionString = juce::String::formatted("%02d:%02d", minutes, seconds);                    
                    totalLengthLabel.setText(positionString, juce::dontSendNotification);

                    progressSlider.setRange(0, transportSource.getLengthInSeconds(), 0); //sets timer slider limits, juce::RelativeTime is incompatible so *getLengthInSeconds()*
                    this->playButtonClicked(); //plays song automatically                  
                    readerSource.reset(newSource.release());                                          
                }
            }
        });
    
    DBG("clicked");

}
void MainComponent::playButtonClicked() {
    if ((state == Stopped) || (state == Paused))
        changeState(Starting);
    else if (state == Playing)
        changeState(Pausing);
    DBG("clicked");
}
void MainComponent::stopButtonClicked() {
    
    if (state == Paused)
        changeState(Stopped);
    else
        changeState(Stopping);
    
    DBG("clicked");
}
void MainComponent::progressSliderDragStart() {
    dragState = Dragging;
    changeState(Pausing);
   
}
void MainComponent::progressSliderDragEnd() {
    changeState(Starting);
    dragState = NotDragging;
    transportSource.setPosition(progressSlider.getValue());

}
