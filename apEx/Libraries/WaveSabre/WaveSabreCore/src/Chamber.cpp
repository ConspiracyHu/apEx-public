#include <WaveSabreCore/Chamber.h>
#include <WaveSabreCore/Helpers.h>

namespace WaveSabreCore
{
	Chamber::Chamber()
		: Device((int)Chamber_ParamIndices::Chamber_NumParams)
	{
		mode = 1;
		feedback = .88f;
		lowCutFreq = 200.0f;
		highCutFreq = 8000.0f;
		dryWet = .27f;
		preDelay = 0.0f;

		for (int i = 0; i < numBuffers; i++) delayBuffers[i] = new DelayBuffer();

		for (int i = 0; i < 2; i++)
		{
			lowCutFilter[i].SetType(StateVariableFilterType::StateVariableFilterType_Highpass);
			highCutFilter[i].SetType(StateVariableFilterType::StateVariableFilterType_Lowpass);
		}
	}

	Chamber::~Chamber()
	{
		for (int i = 0; i < numBuffers; i++) delete delayBuffers[i];
	}

	void Chamber::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		const float delayLengths[] =
		{
			7.0f,
			21.0f,
			17.0f,
			13.0f,
			3.0f,
			11.0f,
			23.0f,
			31.0f
		};
		const float multipliers[] =
		{
			1.0f, 5.0f, 10.0f
		};
		for (int i = 0; i < numBuffers; i++) delayBuffers[i]->SetLength(delayLengths[i] * multipliers[mode]);

		for (int i = 0; i < 2; i++) preDelayBuffers[i].SetLength(preDelay * 500.0f);

		for (int i = 0; i < 2; i++)
		{
			lowCutFilter[i].SetFreq(lowCutFreq);
			highCutFilter[i].SetFreq(highCutFreq);
		}

		for (int i = 0; i < numSamples; i++)
		{
			float inputSamples[2], filteredInputSamples[2];
			for (int j = 0; j < 2; j++)
			{
				inputSamples[j] = inputs[j][i];
				if (preDelay > 0)
				{
					preDelayBuffers[j].WriteSample(inputSamples[j]);
					filteredInputSamples[j] = lowCutFilter[j].Next(highCutFilter[j].Next(preDelayBuffers[j].ReadSample()));
				}
				else
				{
					filteredInputSamples[j] = lowCutFilter[j].Next(highCutFilter[j].Next(inputSamples[j]));
				}
				outputs[j][i] = 0.0f;
			}
			for (int j = 0; j < numBuffers; j++)
			{
				int channelIndex = (int)(j < numBuffers / 2);
				float feedbackSample = delayBuffers[numBuffers - 1 - j]->ReadSample();
				delayBuffers[j]->WriteSample(filteredInputSamples[channelIndex] + feedbackSample * feedback);
				outputs[channelIndex][i] += inputSamples[channelIndex] * (1.0f - dryWet) + feedbackSample * dryWet;
			}
			outputs[0][i] /= (float)(numBuffers / 2);
			outputs[1][i] /= (float)(numBuffers / 2);
		}
	}

	void Chamber::SetParam(int index, float value)
	{
		switch ((Chamber_ParamIndices)index)
		{
		case Chamber_ParamIndices::Chamber_Mode: mode = (int)(value * 2.0f); break;
		case Chamber_ParamIndices::Chamber_Feedback: feedback = value * .5f + .5f; break;
		case Chamber_ParamIndices::Chamber_LowCutFreq: lowCutFreq = Helpers::ParamToFrequency(value); break;
		case Chamber_ParamIndices::Chamber_HighCutFreq: highCutFreq = Helpers::ParamToFrequency(value); break;
		case Chamber_ParamIndices::Chamber_DryWet: dryWet = value; break;
		case Chamber_ParamIndices::Chamber_PreDelay: preDelay = value; break;
		}
	}

	float Chamber::GetParam(int index) const
	{
		switch ((Chamber_ParamIndices)index)
		{
		case Chamber_ParamIndices::Chamber_Mode:
		default:
			return (float)mode / 2.0f;

		case Chamber_ParamIndices::Chamber_Feedback: return (feedback - .5f) * 2.0f;
		case Chamber_ParamIndices::Chamber_LowCutFreq: return Helpers::FrequencyToParam(lowCutFreq);
		case Chamber_ParamIndices::Chamber_HighCutFreq: return Helpers::FrequencyToParam(highCutFreq);
		case Chamber_ParamIndices::Chamber_DryWet: return dryWet;
		case Chamber_ParamIndices::Chamber_PreDelay: return preDelay;
		}
	}
}
