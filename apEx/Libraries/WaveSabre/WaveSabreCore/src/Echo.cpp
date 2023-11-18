#include <WaveSabreCore/Echo.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Echo::Echo()
		: Device((int)Echo_ParamIndices::Echo_NumParams)
	{
		leftDelayCoarse = 3;
		leftDelayFine = 0;
		rightDelayCoarse = 4;
		rightDelayFine = 0;
		lowCutFreq = 20.0f;
		highCutFreq = 20000.0f- 20.0f;
		feedback = .5f;
		cross = 0.0f;
		dryWet = .5f;

		for (int i = 0; i < 2; i++)
		{
			lowCutFilter[i].SetType(StateVariableFilterType::StateVariableFilterType_Highpass);
			highCutFilter[i].SetType(StateVariableFilterType::StateVariableFilterType_Lowpass);
		}
	}

	Echo::~Echo()
	{
	}

	void Echo::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		double delayScalar = 120.0 / (double)Helpers::CurrentTempo / 8.0 * 1000.0;
		float leftBufferLengthMs = (float)((double)leftDelayCoarse * delayScalar + (double)leftDelayFine);
		float rightBufferLengthMs = (float)((double)rightDelayCoarse * delayScalar + (double)rightDelayFine);
		leftBuffer.SetLength(leftBufferLengthMs);
		rightBuffer.SetLength(rightBufferLengthMs);

		for (int i = 0; i < 2; i++)
		{
			lowCutFilter[i].SetFreq(lowCutFreq);
			highCutFilter[i].SetFreq(highCutFreq);
		}

		for (int i = 0; i < numSamples; i++)
		{
			float leftInput = inputs[0][i];
			float rightInput = inputs[1][i];

			float leftDelay = lowCutFilter[0].Next(highCutFilter[0].Next(leftBuffer.ReadSample()));
			float rightDelay = lowCutFilter[1].Next(highCutFilter[1].Next(rightBuffer.ReadSample()));

			leftBuffer.WriteSample(leftInput + (leftDelay * (1.0f - cross) + rightDelay * cross) * feedback);
			rightBuffer.WriteSample(rightInput + (rightDelay * (1.0f - cross) + leftDelay * cross) * feedback);

			outputs[0][i] = leftInput * (1.0f - dryWet) + leftDelay * dryWet;
			outputs[1][i] = rightInput * (1.0f - dryWet) + rightDelay * dryWet;
		}
	}

	void Echo::SetParam(int index, float value)
	{
		switch ((Echo_ParamIndices)index)
		{
		case Echo_ParamIndices::Echo_LeftDelayCoarse: leftDelayCoarse = (int)(value * 16.0f); break;
		case Echo_ParamIndices::Echo_LeftDelayFine: leftDelayFine = (int)(value * 200.0f); break;
		case Echo_ParamIndices::Echo_RightDelayCoarse: rightDelayCoarse = (int)(value * 16.0f); break;
		case Echo_ParamIndices::Echo_RightDelayFine: rightDelayFine = (int)(value * 200.0f); break;
		case Echo_ParamIndices::Echo_LowCutFreq: lowCutFreq = Helpers::ParamToFrequency(value); break;
		case Echo_ParamIndices::Echo_HighCutFreq: highCutFreq = Helpers::ParamToFrequency(value); break;
		case Echo_ParamIndices::Echo_Feedback: feedback = value; break;
		case Echo_ParamIndices::Echo_Cross: cross = value; break;
		case Echo_ParamIndices::Echo_DryWet: dryWet = value; break;
		}
	}

	float Echo::GetParam(int index) const
	{
		switch ((Echo_ParamIndices)index)
		{
		case Echo_ParamIndices::Echo_LeftDelayCoarse:
		default:
			return (float)leftDelayCoarse / 16.0f;

		case Echo_ParamIndices::Echo_LeftDelayFine: return (float)leftDelayFine / 200.0f;
		case Echo_ParamIndices::Echo_RightDelayCoarse: return (float)rightDelayCoarse / 16.0f;
		case Echo_ParamIndices::Echo_RightDelayFine: return (float)rightDelayFine / 200.0f;
		case Echo_ParamIndices::Echo_LowCutFreq: return Helpers::FrequencyToParam(lowCutFreq);
		case Echo_ParamIndices::Echo_HighCutFreq: return Helpers::FrequencyToParam(highCutFreq);
		case Echo_ParamIndices::Echo_Feedback: return feedback;
		case Echo_ParamIndices::Echo_Cross: return cross;
		case Echo_ParamIndices::Echo_DryWet: return dryWet;
		}
	}
}
