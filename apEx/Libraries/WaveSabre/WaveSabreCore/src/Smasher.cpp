#include <WaveSabreCore/Smasher.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	const float Smasher::lookaheadMs = 2.0f;

	Smasher::Smasher()
		: Device((int)Smasher_ParamIndices::Smasher_NumParams)
	{
		sidechain = false;
		inputGain = 0.0f;
		threshold = 0.0f;
		ratio = 2.0f;
		attack = 1.0f;
		release = 200.0f;
		outputGain = 0.0f;

		peak = 0.0f;
	}

	Smasher::~Smasher()
	{
	}

	void Smasher::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		leftBuffer.SetLength(lookaheadMs);
		rightBuffer.SetLength(lookaheadMs);

		float inputGainScalar = Helpers::DbToScalar(inputGain);
		float outputGainScalar = Helpers::DbToScalar(outputGain);
		int inputChannelOffset = sidechain ? 2 : 0;

		float envCoeff = (float)(1000.0 / Helpers::CurrentSampleRate);
		float attackScalar = envCoeff / attack;
		float releaseScalar = envCoeff / release;

		float thresholdScalar = Helpers::DbToScalar(threshold);

		for (int i = 0; i < numSamples; i++)
		{
			leftBuffer.WriteSample(inputs[0][i] * inputGainScalar);
			rightBuffer.WriteSample(inputs[1][i] * inputGainScalar);
			float inputLeft = inputs[inputChannelOffset][i] * inputGainScalar;
			float inputRight = inputs[inputChannelOffset + 1][i] * inputGainScalar;
			float inputLeftLevel = fabsf(inputLeft);
			float inputRightLevel = fabsf(inputRight);
			float inputLevel = inputLeftLevel >= inputRightLevel ? inputLeftLevel : inputRightLevel;

			if (inputLevel > peak)
			{
				peak += attackScalar;
				if (peak > inputLevel) peak = inputLevel;
			}
			else
			{
				peak -= releaseScalar;
				if (peak < inputLevel) peak = inputLevel;
			}

			float gainScalar = outputGainScalar;
			if (peak > thresholdScalar) gainScalar *= (thresholdScalar + (peak - thresholdScalar) / ratio) / peak;

			outputs[0][i] = leftBuffer.ReadSample() * gainScalar;
			outputs[1][i] = rightBuffer.ReadSample() * gainScalar;
		}
	}

	void Smasher::SetParam(int index, float value)
	{
		switch ((Smasher_ParamIndices)index)
		{
		case Smasher_ParamIndices::Smasher_Sidechain: sidechain = Helpers::ParamToBoolean(value); break;
		case Smasher_ParamIndices::Smasher_InputGain: inputGain = Helpers::ParamToDb(value, 12.0f); break;
		case Smasher_ParamIndices::Smasher_Threshold: threshold = Helpers::ParamToDb(value / 2.0f, 36.0f); break;
		case Smasher_ParamIndices::Smasher_Attack: attack = Helpers::ScalarToEnvValue(value) / 5.0f; break;
		case Smasher_ParamIndices::Smasher_Release: release = Helpers::ScalarToEnvValue(value); break;
		case Smasher_ParamIndices::Smasher_Ratio: ratio = value * value * 18.0f + 2.0f; break;
		case Smasher_ParamIndices::Smasher_OutputGain: outputGain = Helpers::ParamToDb(value, 12.0f); break;
		}
	}

	float Smasher::GetParam(int index) const
	{
		switch ((Smasher_ParamIndices)index)
		{
		case Smasher_ParamIndices::Smasher_Sidechain:
		default:
			return Helpers::BooleanToParam(sidechain);

		case Smasher_ParamIndices::Smasher_InputGain: return Helpers::DbToParam(inputGain, 12.0f);
		case Smasher_ParamIndices::Smasher_Threshold: return Helpers::DbToParam(threshold, 36.0f) * 2.0f;
		case Smasher_ParamIndices::Smasher_Attack: return Helpers::EnvValueToScalar(attack * 5.0f); break;
		case Smasher_ParamIndices::Smasher_Release: return Helpers::EnvValueToScalar(release); break;
		case Smasher_ParamIndices::Smasher_Ratio: return sqrtf((ratio - 2.0f) / 18.0f);
		case Smasher_ParamIndices::Smasher_OutputGain: return Helpers::DbToParam(outputGain, 12.0f);
		}
		return 0.0f;
	}
}
