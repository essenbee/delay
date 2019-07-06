/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 200);

	makeRotorySlider(&wetDryMixSlider, "Wet/Dry Ratio", this);
	wetDryMixValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "wetdrymix", wetDryMixSlider);
	addAndMakeVisible(&wetDryMixSlider);

	makeRotorySlider(&feedbackSlider, " Feedback", this);
	feedbackValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "feedback", feedbackSlider);
	addAndMakeVisible(&feedbackSlider);

	makeRotorySlider(&delaySlider, " Delay s", this);
	delayValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "delay", delaySlider);
	addAndMakeVisible(&delaySlider);

	makeRotorySlider(&chorusRateSlider, " Rate (Hz)", this);
	chorusRateValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "rate", chorusRateSlider);
	addAndMakeVisible(&chorusRateSlider);

	makeRotorySlider(&chorusDepthSlider, " Depth", this);
	chorusDepthValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "depth", chorusDepthSlider);
	addAndMakeVisible(&chorusDepthSlider);

	makeRotorySlider(&phaseOffsetSlider, " Phase Offset", this);
	phaseOffsetValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "phaseOffset", phaseOffsetSlider);
	addAndMakeVisible(&phaseOffsetSlider);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
}

//==============================================================================
void DelayAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void DelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	Grid grid;

	grid.templateRows = { Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr) };
	grid.templateColumns = { Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr) };

	grid.items = {
		GridItem(wetDryMixSlider),
		GridItem(feedbackSlider),
		GridItem(delaySlider),
		GridItem(chorusRateSlider),
		GridItem(chorusDepthSlider),
		GridItem(phaseOffsetSlider)
	 };

	grid.performLayout(getLocalBounds());
}

void DelayAudioProcessorEditor::makeRotorySlider(Slider* slider, String suffix, Component* parent)
{
	slider->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
	slider->setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
	slider->setPopupDisplayEnabled(true, true, parent);
	slider->setTextValueSuffix(suffix);
}
