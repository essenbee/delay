/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
					 pluginState(*this, nullptr, "PARAMETERS", createParameterLayout()),
					 delayInSamples(0),
					 delayBufferSize(0),
					 readHead(0),
					 writeHead(0),
					 leftChannelFeedback(0),
					 rightChannelFeedback(0)
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

	audioParams.push_back(std::move(wetDryMixParam));
	audioParams.push_back(std::move(feedbackParam));
	audioParams.push_back(std::move(delayParam));

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

	delayInSamples = sampleRate * *pluginState.getRawParameterValue("delay");
	delayBufferSize = sampleRate * 2; // Max Delay in seconds

	leftDelayBuffer.resize(delayBufferSize);
	rightDelayBuffer.resize(delayBufferSize);

	writeHead = 0;
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
        buffer.clear (i, 0, buffer.getNumSamples());

	//delayInSamples = getSampleRate() * *pluginState.getRawParameterValue("delay");

    auto* leftChannelData = buffer.getWritePointer (0);
	auto* rightChannelData = buffer.getWritePointer(1);

	for (int i = 0; i < buffer.getNumSamples(); i++)
	{
		DBG(writeHead);

		leftDelayBuffer[writeHead] = leftChannelData[i] * leftChannelFeedback;
		rightDelayBuffer[writeHead] = rightChannelData[i] * rightChannelFeedback;

		readHead = writeHead - delayInSamples;

		if (readHead < 0)
		{
			readHead += delayBufferSize;
		}

		float leftDelayedSample = leftDelayBuffer[static_cast<int>(readHead)];
		float rightDelayedSample = rightDelayBuffer[static_cast<int>(readHead)];

		leftChannelFeedback = *pluginState.getRawParameterValue("feedback") * leftDelayedSample;
		rightChannelFeedback = *pluginState.getRawParameterValue("feedback") * rightDelayedSample;

		writeHead++;

		if (writeHead >= delayBufferSize)
		{
			writeHead = 0;
		}

		auto wetDryRatio = *pluginState.getRawParameterValue("wetdrymix");

		buffer.setSample(0, i, buffer.getSample(0, i) * (1 - wetDryRatio) + (leftDelayedSample * wetDryRatio));
		buffer.setSample(1, i, buffer.getSample(1, i) * (1 - wetDryRatio) + (rightDelayedSample * wetDryRatio));
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
void DelayAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}
