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
    setSize (500, 300);

	Font headerFont;
	headerFont.setSizeAndStyle(24.0, "header", 1.0f, 0.1f);
	headerFont.bold;
	headerFont.setTypefaceName("Calibri");

	pluginName.setText("Codebase Alpha Delay/Chorus/Flanger", dontSendNotification);
	pluginName.setFont(headerFont);
	pluginName.setJustificationType(Justification::centredTop);
	addAndMakeVisible(&pluginName);

	//makeRotorySlider(&wetDryMixSlider, "Wet/Dry Ratio", this);
	//wetDryMixValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "wetdrymix", wetDryMixSlider);
	//addAndMakeVisible(&wetDryMixSlider);

	//wetDryMixLabel.setText("Wet/Dry Mix", dontSendNotification);
	//wetDryMixLabel.setJustificationType(Justification::centredTop);
	//addAndMakeVisible(&wetDryMixLabel);

	//makeRotorySlider(&feedbackSlider, " Feedback", this);
	//feedbackValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "feedback", feedbackSlider);
	//addAndMakeVisible(&feedbackSlider);

	//feedbackLabel.setText("Feedback", dontSendNotification);
	//feedbackLabel.setJustificationType(Justification::centredTop);
	//addAndMakeVisible(&feedbackLabel);

	//makeRotorySlider(&delaySlider, " Delay (s)", this);
	//delayValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "delay", delaySlider);
	//addAndMakeVisible(&delaySlider);

	//delayLabel.setText("Delay (s)", dontSendNotification);
	//delayLabel.setJustificationType(Justification::centredTop);
	//addAndMakeVisible(&delayLabel);

	makeRotorySlider(&wetDryMixLeftSlider, "Left Wet/Dry Mix", this);
	wetDryMixLeftValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "wetdrymixleft", wetDryMixLeftSlider);
	addAndMakeVisible(&wetDryMixLeftSlider);

	wetDryMixLeftLabel.setText("Left Wet/Dry Mix", dontSendNotification);
	wetDryMixLeftLabel.setJustificationType(Justification::centredTop);
	addAndMakeVisible(&wetDryMixLeftLabel);

	makeRotorySlider(&wetDryMixRightSlider, "Right Wet/Dry Mix", this);
	wetDryMixRightValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "wetdrymixright", wetDryMixRightSlider);
	addAndMakeVisible(&wetDryMixRightSlider);

	wetDryMixRightLabel.setText("Right Wet/Dry Mix", dontSendNotification);
	wetDryMixRightLabel.setJustificationType(Justification::centredTop);
	addAndMakeVisible(&wetDryMixRightLabel);

	makeRotorySlider(&cutoffFreqSlider, " Cutoff (Hz)", this);
	cutoffFreqValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "cutoff", cutoffFreqSlider);
	addAndMakeVisible(&cutoffFreqSlider);

	cutoffFreqLabel.setText("Cutoff Freq.", dontSendNotification);
	cutoffFreqLabel.setJustificationType(Justification::centredTop);
	addAndMakeVisible(&cutoffFreqLabel);

	makeRotorySlider(&chorusRateSlider, " Rate (Hz)", this);
	chorusRateValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "rate", chorusRateSlider);
	addAndMakeVisible(&chorusRateSlider);

	rateLabel.setText("Rate (Hz)", dontSendNotification);
	rateLabel.setJustificationType(Justification::centredTop);
	addAndMakeVisible(&rateLabel);

	makeRotorySlider(&chorusDepthSlider, " Depth", this);
	chorusDepthValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "depth", chorusDepthSlider);
	addAndMakeVisible(&chorusDepthSlider);

	depthLabel.setText("Depth", dontSendNotification);
	depthLabel.setJustificationType(Justification::centredTop);
	addAndMakeVisible(&depthLabel);

	//makeRotorySlider(&phaseOffsetSlider, " Phase Offset", this);
	//phaseOffsetValue = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.pluginState, "phaseOffset", phaseOffsetSlider);
	//addAndMakeVisible(&phaseOffsetSlider);

	//phaseOffsetLabel.setText("Phase Offset", dontSendNotification);
	//phaseOffsetLabel.setJustificationType(Justification::centredTop);
	//addAndMakeVisible(&phaseOffsetLabel);
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

	grid.templateRows = { Grid::TrackInfo(1_fr), Grid::TrackInfo(2_fr), Grid::TrackInfo(1_fr), 
		Grid::TrackInfo(2_fr), Grid::TrackInfo(1_fr) };
	grid.templateColumns = { Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr), Grid::TrackInfo(1_fr) };

	grid.items = {
		GridItem(pluginName)
			.withArea(GridItem::Property(1), GridItem::Property(1), GridItem::Property(1), GridItem::Property(4)),
		//GridItem(wetDryMixSlider),
		//GridItem(feedbackSlider),
		//GridItem(delaySlider),
		//GridItem(wetDryMixLabel),
		//GridItem(feedbackLabel),
		//GridItem(delayLabel),
		GridItem(wetDryMixLeftSlider),
		GridItem(wetDryMixRightSlider),
		GridItem(cutoffFreqSlider),

		GridItem(wetDryMixLeftLabel),
		GridItem(wetDryMixRightLabel),
		GridItem(cutoffFreqLabel),

		GridItem(chorusRateSlider),
		GridItem(chorusDepthSlider),
		GridItem(),
		//GridItem(phaseOffsetSlider),

		GridItem(rateLabel),
		GridItem(depthLabel),
		//GridItem(phaseOffsetLabel)
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
