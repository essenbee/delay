/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CodebaseAlphaFx.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
					 pluginState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

DelayAudioProcessor::~DelayAudioProcessor()
{
}

AudioProcessorValueTreeState::ParameterLayout DelayAudioProcessor::createParameterLayout()
{
	std::vector<std::unique_ptr<RangedAudioParameter>> audioParams;

	auto wetDryMixParam = std::make_unique<AudioParameterFloat>("wetdrymix", "Wet/Dry Mix", 0.0f, 1.0f, 0.5f);
	auto feedbackParam = std::make_unique<AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.98f, 0.0f);
	auto delayParam = std::make_unique<AudioParameterFloat>("delay", "Delay", 0.0f, 2.0f, 0.5f);

	auto rateParam = std::make_unique<AudioParameterFloat>("rate", "Rate", 0.1f, 20.0f, 1.0f);
	auto depthParam = std::make_unique<AudioParameterFloat>("depth", "Depth", 0.0f, 1.0f, 0.25f);

	// Dimensions Chorus
	auto wetDryMixLeftParam = std::make_unique<AudioParameterFloat>("wetdrymixleft", "Left Wet/Dry Mix", 0.0f, 1.0f, 0.25f);
	auto wetDryMixRightParam = std::make_unique<AudioParameterFloat>("wetdrymixright", "Right Wet/Dry Mix", 0.0f, 1.0f, 0.25f);
	auto cutoffFreqParam = std::make_unique<AudioParameterFloat>("cutoff", "Cutoff Freq.", 20.0f, 22000.0f, 1000.0f);
	
	auto phaseOffsetParam = std::make_unique<AudioParameterFloat>("phaseOffset", "Phase Offset", 0.0f, 1.0f, 0.0f);

	audioParams.push_back(std::move(wetDryMixParam));
	audioParams.push_back(std::move(feedbackParam));
	audioParams.push_back(std::move(delayParam));

	audioParams.push_back(std::move(rateParam));
	audioParams.push_back(std::move(depthParam));

	audioParams.push_back(std::move(wetDryMixLeftParam));
	audioParams.push_back(std::move(wetDryMixRightParam));
	audioParams.push_back(std::move(cutoffFreqParam));

	audioParams.push_back(std::move(phaseOffsetParam));

	return { audioParams.begin(), audioParams.end() };
}

//==============================================================================
const String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{
}

const String DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

	delay.reset(sampleRate);
	chorus.reset(sampleRate);
	dimensionsChorus.reset(sampleRate);
	updateParameters();
}

void DelayAudioProcessor::updateParameters()
{
	auto params = delay.getParameters();
	params.delay = *pluginState.getRawParameterValue("delay");
	params.feedback = *pluginState.getRawParameterValue("feedback");
	params.wetDryMix = *pluginState.getRawParameterValue("wetdrymix");
	delay.setParameters(params);

	auto chorusParams = chorus.getParameters();
	chorusParams.wetDryMix = *pluginState.getRawParameterValue("wetdrymix");
	chorusParams.feedback = *pluginState.getRawParameterValue("feedback");
	chorusParams.rate = *pluginState.getRawParameterValue("rate");
	chorusParams.depth = *pluginState.getRawParameterValue("depth");
	chorusParams.pluginType = chorusType::kFlanger;
	chorus.setParameters(chorusParams);

	auto dimensionsParams = dimensionsChorus.getParameters();
	dimensionsParams.cutoffFrequency = *pluginState.getRawParameterValue("cutoff");
	dimensionsParams.depth = *pluginState.getRawParameterValue("depth");
	dimensionsParams.rate = *pluginState.getRawParameterValue("rate");
	dimensionsParams.wetDryMixLeft = *pluginState.getRawParameterValue("wetdrymixleft");
	dimensionsParams.wetDryMixRight = *pluginState.getRawParameterValue("wetdrymixright");
	dimensionsChorus.setParameters(dimensionsParams);
}

void DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DelayAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	{
		buffer.clear(i, 0, buffer.getNumSamples());
	}

	auto* leftChannelData = buffer.getWritePointer (0);
	auto* rightChannelData = buffer.getWritePointer(1);

	for (int i = 0; i < buffer.getNumSamples(); i++)
	{
		updateParameters();

		float inputFrame[2]{ leftChannelData[i], rightChannelData[i] };
		float outputFrame[2];

		// delay.processAudioFrame(inputFrame, outputFrame, totalNumInputChannels, totalNumOutputChannels)
		// chorus.processAudioFrame(inputFrame, outputFrame, totalNumInputChannels, totalNumOutputChannels);
		dimensionsChorus.processAudioFrame(inputFrame, outputFrame, totalNumInputChannels, totalNumOutputChannels);

		buffer.setSample(0, i, outputFrame[0]);
		buffer.setSample(1, i, outputFrame[1]);
	}
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
    return new DelayAudioProcessorEditor (*this);
}

//==============================================================================
void DelayAudioProcessor::getStateInformation(MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.

	auto state = pluginState.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
	if (xmlState.get() != nullptr)
	{
		if (xmlState->hasTagName(pluginState.state.getType()))
		{
			pluginState.replaceState(ValueTree::fromXml(*xmlState));
		}
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}
