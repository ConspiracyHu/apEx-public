#include <WaveSabreCore/Twister.h>
#include <WaveSabreCore/Helpers.h>

int const lookAhead = 4;

namespace WaveSabreCore
{
	Twister::Twister()
		: Device((int)Twister_ParamIndices::Twister_NumParams)
	{
		type = 0;
		amount = 0;
		feedback = 0.0f;
		spread = Spread::Spread_Mono;
		vibratoFreq = Helpers::ParamToVibratoFreq(0.0f);
		vibratoAmount = 0.0f;
		
		vibratoPhase = 0.0;
		
		lowCutFreq = 20.0f;
		highCutFreq = 20000.0f- 20.0f;

		dryWet = .5f;

		leftBuffer.SetLength(1000);
		rightBuffer.SetLength(1000);

		lastLeft = 0.0f;
		lastRight = 0.0f;

		for (int i = 0; i < 2; i++)
		{
			lowCutFilter[i].SetType(StateVariableFilterType::StateVariableFilterType_Highpass);
			highCutFilter[i].SetType(StateVariableFilterType::StateVariableFilterType_Lowpass);
		}
	}

	Twister::~Twister()
	{
	}

	void Twister::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		double vibratoDelta = (vibratoFreq / Helpers::CurrentSampleRate) * 0.25f;
		float outputLeft = 0.0f;
		float outputRight = 0.0f;
		float positionLeft = 0.0f;
		float positionRight = 0.0f;

		for (int i = 0; i < 2; i++)
		{
			lowCutFilter[i].SetFreq(lowCutFreq);
			highCutFilter[i].SetFreq(highCutFreq);
		}

		for (int i = 0; i < numSamples; i++)
		{
			float leftInput = inputs[0][i];
			float rightInput = inputs[1][i];

			double freq = Helpers::FastSin(vibratoPhase) * vibratoAmount;

			switch (spread)
			{
			case Spread::Spread_Mono: 
			default:
				positionLeft = Helpers::Clamp((amount + (float)freq), 0.0f, 1.0f);
				positionRight = positionLeft;
				break;
			case Spread::Spread_FullInvert:
				positionLeft = Helpers::Clamp((amount + (float)freq), 0.0f, 1.0f);
				positionRight = (1.0f - Helpers::Clamp((amount + (float)freq), 0.0f, 1.0f));
				break;
			case Spread::Spread_ModInvert:
				positionLeft = Helpers::Clamp((amount + (float)freq), 0.0f, 1.0f);
				positionRight = Helpers::Clamp((amount - (float)freq), 0.0f, 1.0f);
				break;
			}

			switch (type)
			{
			case 0:
				positionLeft *= 132.0f;
				positionRight *= 132.0f;
				outputLeft = highCutFilter[0].Next(lowCutFilter[0].Next(leftBuffer.ReadPosition(positionLeft + 2)));
				outputRight = highCutFilter[1].Next(lowCutFilter[1].Next(rightBuffer.ReadPosition(positionRight + 2)));
				leftBuffer.WriteSample(leftInput + (outputLeft * feedback));
				rightBuffer.WriteSample(rightInput + (outputRight * feedback));
				break;
			case 1:
				positionLeft *= 132.0f;
				positionRight *= 132.0f;
				outputLeft = highCutFilter[0].Next(lowCutFilter[0].Next(leftBuffer.ReadPosition(positionLeft + 2)));
				outputRight = highCutFilter[1].Next(lowCutFilter[1].Next(rightBuffer.ReadPosition(positionRight + 2)));
				leftBuffer.WriteSample(leftInput - (outputLeft * feedback));
				rightBuffer.WriteSample(rightInput - (outputRight * feedback));
				break;
			case 2:
				for (int i = 0; i<6; i++) allPassLeft[i].Delay(positionLeft);
				for (int i = 0; i<6; i++) allPassRight[i].Delay(positionRight);
				outputLeft = highCutFilter[0].Next(lowCutFilter[0].Next(AllPassUpdateLeft(leftInput + lastLeft * feedback)));
				outputRight = highCutFilter[1].Next(lowCutFilter[1].Next(AllPassUpdateRight(rightInput + lastRight * feedback)));
				lastLeft = outputLeft;
				lastRight = outputRight;
				break;
			case 3:
				for (int i = 0; i<6; i++) allPassLeft[i].Delay(positionLeft);
				for (int i = 0; i<6; i++) allPassRight[i].Delay(positionRight);
				outputLeft = highCutFilter[0].Next(lowCutFilter[0].Next(AllPassUpdateLeft(leftInput - lastLeft * feedback)));
				outputRight = highCutFilter[1].Next(lowCutFilter[1].Next(AllPassUpdateRight(rightInput - lastRight * feedback)));
				lastLeft = outputLeft;
				lastRight = outputRight;
				break;
			default:
				outputLeft = 0.0f;
				outputRight = 0.0f;
				break;
			}

			outputs[0][i] = (leftInput * (1.0f - dryWet)) + (outputLeft * dryWet);
			outputs[1][i] = (rightInput * (1.0f - dryWet)) + (outputRight * dryWet);

			vibratoPhase += vibratoDelta;
		}
	}

	float Twister::AllPassUpdateLeft(float input)
	{
		return(
			allPassLeft[0].Update(
			allPassLeft[1].Update(
			allPassLeft[2].Update(
			allPassLeft[3].Update(
			allPassLeft[4].Update(
			allPassLeft[5].Update(input)))))));
	}

	float Twister::AllPassUpdateRight(float input)
	{
		return(
			allPassRight[0].Update(
			allPassRight[1].Update(
			allPassRight[2].Update(
			allPassRight[3].Update(
			allPassRight[4].Update(
			allPassRight[5].Update(input)))))));
	}

	void Twister::SetParam(int index, float value)
	{
		switch ((Twister_ParamIndices)index)
		{
		case Twister_ParamIndices::Twister_Type: type = (int)(value * 3.0f); break;
		case Twister_ParamIndices::Twister_Amount: amount = value; break;
		case Twister_ParamIndices::Twister_Feedback: feedback = value; break;
		case Twister_ParamIndices::Twister_Spread: spread = Helpers::ParamToSpread(value); break;
		case Twister_ParamIndices::Twister_VibratoFreq: vibratoFreq = Helpers::ParamToVibratoFreq(value); break;
		case Twister_ParamIndices::Twister_VibratoAmount: vibratoAmount = value; break;
		case Twister_ParamIndices::Twister_LowCutFreq: lowCutFreq = Helpers::ParamToFrequency(value); break;
		case Twister_ParamIndices::Twister_HighCutFreq: highCutFreq = Helpers::ParamToFrequency(value); break;
		case Twister_ParamIndices::Twister_DryWet: dryWet = value; break;
		}
	}

	float Twister::GetParam(int index) const
	{
		switch ((Twister_ParamIndices)index)
		{
		case Twister_ParamIndices::Twister_Type: 
		default: 
			return type / 3.0f;

		case Twister_ParamIndices::Twister_Amount: return amount;
		case Twister_ParamIndices::Twister_Feedback: return feedback;
		case Twister_ParamIndices::Twister_Spread: return Helpers::SpreadToParam(spread);
		case Twister_ParamIndices::Twister_VibratoFreq: return Helpers::VibratoFreqToParam(vibratoFreq);
		case Twister_ParamIndices::Twister_VibratoAmount: return vibratoAmount;
		case Twister_ParamIndices::Twister_LowCutFreq: return Helpers::FrequencyToParam(lowCutFreq);
		case Twister_ParamIndices::Twister_HighCutFreq: return Helpers::FrequencyToParam(highCutFreq);
		case Twister_ParamIndices::Twister_DryWet: return dryWet;
		}
	}
}
