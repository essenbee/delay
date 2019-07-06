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

		leftDelayBuffer.writeBuffer(xn + leftChannelFeedback);

		double yn = leftDelayBuffer.readBuffer(delayInSamples);

		leftChannelFeedback = parameters.feedback * yn;

		yn = xn * (1 - parameters.wetDryMix) + (yn * parameters.wetDryMix);

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

		leftDelayBuffer.writeBuffer(inputFrame[0] + leftChannelFeedback);
		rightDelayBuffer.writeBuffer(inputFrame[1] + rightChannelFeedback);

		double leftDelayedSample = leftDelayBuffer.readBuffer(delayInSamples);
		double rightDelayedSample = rightDelayBuffer.readBuffer(delayInSamples);

		leftChannelFeedback = parameters.feedback * leftDelayedSample;
		rightChannelFeedback = parameters.feedback * rightDelayedSample;

		outputFrame[0] = inputFrame[0] * (1 - parameters.wetDryMix) + (leftDelayedSample * parameters.wetDryMix);
		outputFrame[1] = inputFrame[1] * (1 - parameters.wetDryMix) + (rightDelayedSample * parameters.wetDryMix);
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
		}

		return *this;
	}

	double wetDryMix = 0.5;
	double feedback = 0.0;
	double rate = 10.0; // Freq. of LFOs in Hz
	double depth = 0.5;
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

		auto leftLfoParams = leftLFO.getParameters();
		auto rightLfoParams = rightLFO.getParameters();
		leftLfoParams.waveform = generatorWaveform::kSin;
		rightLfoParams.waveform = generatorWaveform::kSin;

		leftLFO.setParameters(leftLfoParams);
		rightLFO.setParameters(rightLfoParams);

		return true;
	}

	virtual bool canProcessAudioFrame() override
	{
		return true;
	}

	virtual double processAudioSample(double xn) override
	{
		auto leftLfoParams = leftLFO.getParameters();
		leftLfoParams.frequency_Hz = parameters.rate;
		leftLfoParams.waveform = generatorWaveform::kSin;
		leftLFO.setParameters(leftLfoParams);

		auto leftLfoOutput = leftLFO.renderAudioOutput();
		auto leftLfoValue = static_cast<float>(leftLfoOutput.normalOutput * parameters.depth);

		// Chorus Effect delay values
		float leftLfoMapped = jmap(leftLfoValue, -1.0f, 1.0f, 0.005f, 0.030f);
		leftDelayInSamples = sampleRate * leftLfoMapped;

		leftReadHead = writeHead - leftDelayInSamples;

		leftDelayBuffer.writeBuffer(xn + leftChannelFeedback);

		float leftDelayedSample = leftDelayBuffer.readBuffer(leftDelayInSamples);
		leftChannelFeedback = parameters.feedback * leftDelayedSample;

		float yn = xn * (1 - parameters.wetDryMix) + (leftDelayedSample * parameters.wetDryMix);

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
		auto leftLfoParams = leftLFO.getParameters();
		leftLfoParams.frequency_Hz = parameters.rate;
		leftLfoParams.waveform = generatorWaveform::kTriangle;
		leftLFO.setParameters(leftLfoParams);

		auto rightLfoParams = rightLFO.getParameters();
		rightLfoParams.frequency_Hz = parameters.rate;
		rightLfoParams.waveform = generatorWaveform::kTriangle;
		rightLFO.setParameters(rightLfoParams);

		auto leftLfoOutput = leftLFO.renderAudioOutput();
		auto leftLfoValue = static_cast<float>(leftLfoOutput.normalOutput * parameters.depth);

		auto rightLfoOutput = rightLFO.renderAudioOutput();
		auto rightLfoValue = static_cast<float>(rightLfoOutput.normalOutput * parameters.depth);

		// Chorus Effect delay values
		float leftLfoMapped = jmap(leftLfoValue, -1.0f, 1.0f, 0.005f, 0.030f);
		leftDelayInSamples = sampleRate * leftLfoMapped;
		float rightLfoMapped = jmap(rightLfoValue, -1.0f, 1.0f, 0.005f, 0.030f);
		rightDelayInSamples = sampleRate * rightLfoMapped;

		leftReadHead = writeHead - leftDelayInSamples;
		rightReadHead = writeHead - rightDelayInSamples;

		leftDelayBuffer.writeBuffer(inputFrame[0] + leftChannelFeedback);
		rightDelayBuffer.writeBuffer(inputFrame[1] + rightChannelFeedback);

		float leftDelayedSample = leftDelayBuffer.readBuffer(leftDelayInSamples);
		leftChannelFeedback = parameters.feedback * leftDelayedSample;
		float rightDelayedSample = rightDelayBuffer.readBuffer(rightDelayInSamples);
		rightChannelFeedback = parameters.feedback * rightDelayedSample;

		outputFrame[0] = inputFrame[0] * (1 - parameters.wetDryMix) + (leftDelayedSample * parameters.wetDryMix);
		outputFrame[1] = inputFrame[1] * (1 - parameters.wetDryMix) + (rightDelayedSample * parameters.wetDryMix);
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
	float leftReadHead = 0;
	float rightReadHead = 0;
	int writeHead = 0;

	float leftChannelFeedback = 0.0;
	float rightChannelFeedback = 0.0;

	CircularBuffer<float> leftDelayBuffer;
	CircularBuffer<float> rightDelayBuffer;

	LFO leftLFO;
	LFO rightLFO;
};