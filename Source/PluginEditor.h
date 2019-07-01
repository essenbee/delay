/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class DelayAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    DelayAudioProcessorEditor (DelayAudioProcessor&);
    ~DelayAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DelayAudioProcessor& processor;

	Slider wetDryMixSlider;
	Slider feedbackSlider;
	Slider delaySlider;

	void makeRotorySlider(Slider* slider, String suffix, Component* parent);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioProcessorEditor)

public:
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> wetDryMixValue;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> feedbackValue;
	std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> delayValue;
};
