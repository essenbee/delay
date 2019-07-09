/*
  ==============================================================================

    CodebaseAlphaFx.h
    Created: 6 Jul 2019 8:36:44pm
    Author:  Codebase Alpha

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct AlphaSimpleDelayParameters
{
	AlphaSimpleDelayParameters& operator= (const AlphaSimpleDelayParameters& parameters)
	{
		if (this != &parameters)
		{
			wetDryMix = parameters.wetDryMix;
			feedback = parameters.feedback;
			delay = parameters.delay;
		}

		return *this;
	}

	double wetDryMix = 0.5;
	double feedback = 0.0;
	double delay = 0.5; // in seconds
};

class AlphaSimpleDelay : public IAudioSignalProcessor
{
public:
	AlphaSimpleDelay() {}
	~AlphaSimpleDelay() {}

	virtual bool reset(double _sampleRate) override
	{
		sampleRate = _sampleRate;
		delayInSamples = sampleRate + parameters.delay;
		delayBufferSize = (sampleRate * 2) + 1;

		leftDelayBuffer.createCircularBuffer(delayBufferSize);
		rightDelayBuffer.createCircularBuffer(delayBufferSize);

		return true;
	}

	virtual bool canProcessAudioFrame() override
	{
		return true;
	}

	virtual double processAudioSample(double xn) override
	{
		smoothedDelay -= 0.0005f * (smoothedDelay - parameters.delay);
        delayInSamples = sampleRate * smoothedDelay;

		double yn = leftDelayBuffer.readBuffer(delayInSamples);

		leftChannelFeedback = parameters.feedback * yn;
		yn = xn * (1 - parameters.wetDryMix) + (yn * parameters.wetDryMix);

		leftDelayBuffer.writeBuffer(xn + leftChannelFeedback);

		return yn;
	}

	virtual bool processAudioFrame(const float* inputFrame,
		float* outputFrame,
		uint32_t inputChannels,
		uint32_t outputChannels) override
	{
		if (inputChannels == 0 || outputChannels == 0)
		{
			return false;
		}

		// Mono - use processAudioSample
		if (outputChannels == 1)
		{
			outputFrame[0] = processAudioSample(inputFrame[0]);
			return true;
		}

		// Stereo processing
		smoothedDelay -= 0.0005f * (smoothedDelay - parameters.delay);
		delayInSamples = sampleRate * smoothedDelay;

		double leftDelayedSample = leftDelayBuffer.readBuffer(delayInSamples);
		double rightDelayedSample = rightDelayBuffer.readBuffer(delayInSamples);

		leftChannelFeedback = parameters.feedback * leftDelayedSample;
		rightChannelFeedback = parameters.feedback * rightDelayedSample;

		leftDelayBuffer.writeBuffer(inputFrame[0] + leftChannelFeedback);
		rightDelayBuffer.writeBuffer(inputFrame[1] + rightChannelFeedback);

		outputFrame[0] = inputFrame[0] * (1 - parameters.wetDryMix) + (leftDelayedSample * parameters.wetDryMix);
		outputFrame[1] = inputFrame[1] * (1 - parameters.wetDryMix) + (rightDelayedSample * parameters.wetDryMix);

		return true;
	}

	AlphaSimpleDelayParameters getParameters()
	{
		return parameters;
	}

	void setParameters(AlphaSimpleDelayParameters _parameters)
	{
		parameters = _parameters;
	}

private:
	AlphaSimpleDelayParameters parameters;
	double sampleRate = 0;
	double delayInSamples = 0;
	double smoothedDelay = 0;
	int delayBufferSize= 0;
	float readHead = 0;
	int writeHead = 0;

	float leftChannelFeedback = 0.0;
	float rightChannelFeedback = 0.0;

	CircularBuffer<float> leftDelayBuffer;
	CircularBuffer<float> rightDelayBuffer;
};


enum class chorusType { kChorus, kFlanger };

struct AlphaChorusParameters
{
	AlphaChorusParameters& operator= (const AlphaChorusParameters& parameters)
	{
		if (this != &parameters)
		{
			wetDryMix = parameters.wetDryMix;
			feedback = parameters.feedback;
			rate = parameters.rate;
			depth = parameters.depth;
			pluginType = parameters.pluginType;
		}

		return *this;
	}

	double wetDryMix = 0.25;
	double feedback = 0.0;
	double rate = 1.0; // Freq. of LFOs in Hz
	double depth = 0.5;
	chorusType pluginType = chorusType::kChorus;
};

class AlphaChorus : public IAudioSignalProcessor
{
public:
	AlphaChorus() {}
	~AlphaChorus() {}

	virtual bool reset(double _sampleRate) override
	{
		sampleRate = _sampleRate;
		delayBufferSize = (sampleRate * 2) + 1;

		leftDelayBuffer.createCircularBuffer(delayBufferSize);
		rightDelayBuffer.createCircularBuffer(delayBufferSize);

		leftLFO.reset(sampleRate);
		rightLFO.reset(sampleRate);
		updateLfoParameters(pluginType);

		return true;
	}

	virtual bool canProcessAudioFrame() override
	{
		return true;
	}

	virtual double processAudioSample(double xn) override
	{
		updateLfoParameters(pluginType);

		auto leftLfoOutput = leftLFO.renderAudioOutput();
		auto leftLfoValue = static_cast<float>(leftLfoOutput.normalOutput * parameters.depth);

		leftDelayInSamples = getDelayInSamples(leftLfoValue, pluginType);
		float leftDelayedSample = leftDelayBuffer.readBuffer(leftDelayInSamples);

		float yn = xn * (1 - parameters.wetDryMix) + (leftDelayedSample * parameters.wetDryMix);
		leftDelayBuffer.writeBuffer(xn + leftChannelFeedback);

		leftChannelFeedback = parameters.feedback * leftDelayedSample;

		return yn;
	}

	virtual bool processAudioFrame(const float* inputFrame,
		float* outputFrame,
		uint32_t inputChannels,
		uint32_t outputChannels) override
	{
		if (inputChannels == 0 || outputChannels == 0)
		{
			return false;
		}

		// Mono - use processAudioSample
		if (outputChannels == 1)
		{
			outputFrame[0] = processAudioSample(inputFrame[0]);
			return true;
		}

		// Stereo processing
		updateLfoParameters(pluginType);

		auto leftLfoOutput = leftLFO.renderAudioOutput();
		auto leftLfoValue = static_cast<float>(leftLfoOutput.normalOutput * parameters.depth);

		auto rightLfoOutput = rightLFO.renderAudioOutput();
		auto rightLfoValue = static_cast<float>(rightLfoOutput.normalOutput * parameters.depth);

		leftDelayInSamples = getDelayInSamples(leftLfoValue, pluginType);
		rightDelayInSamples = getDelayInSamples(rightLfoValue, pluginType);

		float leftDelayedSample = leftDelayBuffer.readBuffer(leftDelayInSamples);
		float rightDelayedSample = rightDelayBuffer.readBuffer(rightDelayInSamples);

		leftDelayBuffer.writeBuffer(inputFrame[0] + leftChannelFeedback);
		rightDelayBuffer.writeBuffer(inputFrame[1] + rightChannelFeedback);

		leftChannelFeedback = parameters.feedback * leftDelayedSample;
		rightChannelFeedback = parameters.feedback * rightDelayedSample;

		outputFrame[0] = inputFrame[0] * (1 - parameters.wetDryMix) + (leftDelayedSample * parameters.wetDryMix);
		outputFrame[1] = inputFrame[1] * (1 - parameters.wetDryMix) + (rightDelayedSample * parameters.wetDryMix);

		return true;
	}

	AlphaChorusParameters getParameters()
	{
		return parameters;
	}

	void setParameters(AlphaChorusParameters _parameters)
	{
		parameters = _parameters;
	}

private:
	AlphaChorusParameters parameters;
	double sampleRate = 0;
	double leftDelayInSamples = 0;
	double rightDelayInSamples = 0;
	int delayBufferSize = 0;
	chorusType pluginType = chorusType::kChorus;

	float leftChannelFeedback = 0.0;
	float rightChannelFeedback = 0.0;

	CircularBuffer<float> leftDelayBuffer;
	CircularBuffer<float> rightDelayBuffer;

	LFO leftLFO;
	LFO rightLFO;

	void updateLfoParameters(chorusType type)
	{
		auto leftLfoParams = leftLFO.getParameters();
		leftLfoParams.frequency_Hz = parameters.rate;

		if (type == chorusType::kChorus)
		{
			leftLfoParams.waveform = generatorWaveform::kTriangle;
		}
		else
		{
			leftLfoParams.waveform = generatorWaveform::kSin;
		}

		leftLFO.setParameters(leftLfoParams);

		auto rightLfoParams = rightLFO.getParameters();
		rightLfoParams.frequency_Hz = parameters.rate;

		if (type == chorusType::kChorus)
		{
			rightLfoParams.waveform = generatorWaveform::kTriangle;
		}
		else
		{
			rightLfoParams.waveform = generatorWaveform::kSin;
		}

		rightLFO.setParameters(rightLfoParams);
	}

	double getDelayInSamples(float lfoValue, chorusType type)
	{
		if (type == chorusType::kChorus)
		{
			// Chorus Effect delay values
			float lfoMapped = jmap(lfoValue, -1.0f, 1.0f, 0.005f, 0.030f);
			return sampleRate * lfoMapped;
		}

		float unipolarLfoValue = bipolarToUnipolar(lfoValue);
		// Flanger Effect delay values
		float lfoMapped = jmap(unipolarLfoValue, 0.0f, 1.0f, 0.0f, 0.005f);
		return sampleRate * lfoMapped;
	}
};

struct AlphaDimensionsChorusParameters
{
	AlphaDimensionsChorusParameters& operator= (const AlphaDimensionsChorusParameters& parameters)
	{
		if (this != &parameters)
		{
			wetDryMixLeft = parameters.wetDryMixLeft;
			wetDryMixRight = parameters.wetDryMixRight;
			rate = parameters.rate;
			depth = parameters.depth;
			cutoffFrequency = parameters.cutoffFrequency;
		}

		return *this;
	}

	double wetDryMixLeft = 0.25;
	double wetDryMixRight = 0.25;
	double rate = 1.0; // Freq. of LFOs in Hz
	double depth = 0.5;
	double cutoffFrequency = 1000.0; // Hz
};

class AlphaDiminesionsChorus : public IAudioSignalProcessor
{
public:
	AlphaDiminesionsChorus() {};
	~AlphaDiminesionsChorus() {};

	virtual bool reset(double _sampleRate) override
	{
		sampleRate = _sampleRate;
		delayBufferSize = (sampleRate * 2) + 1;

		leftDelayBuffer.createCircularBuffer(delayBufferSize);
		rightDelayBuffer.createCircularBuffer(delayBufferSize);

		sharedLfo.reset(sampleRate);
		updateLfoParameters();
		updateFilterParameters();

		return true;
	}

	virtual bool canProcessAudioFrame() override
	{
		return true;
	}

	virtual double processAudioSample(double xn) override
	{
		return xn;
	}

	virtual bool processAudioFrame(const float* inputFrame,
		float* outputFrame,
		uint32_t inputChannels,
		uint32_t outputChannels) override
	{
		if (inputChannels == 0 || outputChannels == 0)
		{
			return false;
		}

		// Mono - use processAudioSample
		if (outputChannels == 1)
		{
			outputFrame[0] = processAudioSample(inputFrame[0]);
			return true;
		}

		// Can only use stereo input
		updateLfoParameters();
		updateFilterParameters();

		auto lfoOutput = sharedLfo.renderAudioOutput();
		auto leftLfoValue = static_cast<float>(lfoOutput.normalOutput * parameters.depth);
		auto rightLfoValue = static_cast<float>(lfoOutput.invertedOutput * parameters.depth);

		auto leftDelayInSamples = getDelayInSamples(leftLfoValue);
		auto rightDelayInSamples = getDelayInSamples(rightLfoValue);

		float leftDelayedSample = leftDelayBuffer.readBuffer(leftDelayInSamples);
		float rightDelayedSample = rightDelayBuffer.readBuffer(rightDelayInSamples);

		leftDelayBuffer.writeBuffer(inputFrame[0]);
		rightDelayBuffer.writeBuffer(inputFrame[1]);

		float leftInvertedSample = leftDelayedSample * -1.0;
		float rightInvertedSample = rightDelayedSample * -1.0;

		float leftFilteredSample = leftHpf.processAudioSample(leftInvertedSample);
		float rightFilteredSample = rightHpf.processAudioSample(rightInvertedSample);

		float leftOutputSample = (1 - parameters.wetDryMixLeft) + (leftDelayedSample * parameters.wetDryMixLeft)
			+ rightFilteredSample;
		float rightOutputSample = (1 - parameters.wetDryMixLeft) + (rightDelayedSample * parameters.wetDryMixLeft)
			+ leftFilteredSample;

		outputFrame[0] = inputFrame[0] + leftOutputSample;
		outputFrame[1] = inputFrame[1] + rightOutputSample;
	}

	AlphaDimensionsChorusParameters getParameters()
	{
		return parameters;
	}

	void setParameters(AlphaDimensionsChorusParameters params)
	{
		parameters = params;
	}

private:
	AlphaDimensionsChorusParameters parameters;

	CircularBuffer<float> leftDelayBuffer;
	CircularBuffer<float> rightDelayBuffer;
	LFO sharedLfo;
	AudioFilter leftHpf;
	AudioFilter rightHpf;

	double sampleRate = 0.0;
	int delayBufferSize = 0;

	void updateLfoParameters()
	{
		auto params = sharedLfo.getParameters();
		params.waveform = generatorWaveform::kTriangle;
		params.frequency_Hz = parameters.rate;
		sharedLfo.setParameters(params);
	}

	void updateFilterParameters()
	{
		auto params = leftHpf.getParameters();
		params.fc = parameters.cutoffFrequency;
		params.algorithm = filterAlgorithm::kHPF2;

		leftHpf.setParameters(params);
		rightHpf.setParameters(params);
	}

	float getDelayInSamples(float lfoValue)
	{
		// Chorus Effect delay values
		float lfoMapped = jmap(lfoValue, -1.0f, 1.0f, 0.005f, 0.030f);
		return sampleRate * lfoMapped;
	}
};