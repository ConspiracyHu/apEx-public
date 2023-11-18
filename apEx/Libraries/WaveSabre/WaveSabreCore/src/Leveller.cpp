#include <WaveSabreCore/Leveller.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Leveller::Leveller()
		: Device((int)Leveller_ParamIndices::Leveller_NumParams)
	{
		lowCutFreq = 20.0f;
		lowCutQ = 1.0f;

		peak1Freq = 1000.0f;
		peak1Gain = 0.0f;
		peak1Q = 1.0f;

		peak2Freq = 3000.0f;
		peak2Gain = 0.0f;
		peak2Q = 1.0f;

		peak3Freq = 7000.0f;
		peak3Gain = 0.0f;
		peak3Q = 1.0f;

		highCutFreq = 20000.0f;
		highCutQ = 1.0f;

		for (int i = 0; i < 2; i++)
		{
			highpass[i].SetType(BiquadFilterType::BiquadFilterType_Highpass);
			peak1[i].SetType(BiquadFilterType::BiquadFilterType_Peak);
			peak2[i].SetType(BiquadFilterType::BiquadFilterType_Peak);
			peak3[i].SetType(BiquadFilterType::BiquadFilterType_Peak);
		}

		master = 1.0f;
	}

	void Leveller::Run(double songPosition, float **inputs, float **outputs, int numSamples)
	{
		for (int i = 0; i < 2; i++)
		{
			highpass[i].SetFreq(lowCutFreq);
			highpass[i].SetQ(lowCutQ);

			lowpass[i].SetFreq(highCutFreq);
			lowpass[i].SetQ(highCutQ);

			peak1[i].SetFreq(peak1Freq);
			peak1[i].SetGain(peak1Gain);
			peak1[i].SetQ(peak1Q);

			peak2[i].SetFreq(peak2Freq);
			peak2[i].SetGain(peak2Gain);
			peak2[i].SetQ(peak2Q);

			peak3[i].SetFreq(peak3Freq);
			peak3[i].SetGain(peak3Gain);
			peak3[i].SetQ(peak3Q);

			for (int j = 0; j < numSamples; j++)
			{
				float sample = inputs[i][j];

				sample = highpass[i].Next(sample);
				if (peak1Gain != 0.0f) sample = peak1[i].Next(sample);
				if (peak2Gain != 0.0f) sample = peak2[i].Next(sample);
				if (peak3Gain != 0.0f) sample = peak3[i].Next(sample);
				sample = lowpass[i].Next(sample);

				outputs[i][j] = sample * master;
			}
		}
	}

	void Leveller::SetParam(int index, float value)
	{
		switch ((Leveller_ParamIndices)index)
		{
		case Leveller_ParamIndices::Leveller_LowCutFreq: lowCutFreq = Helpers::ParamToFrequency(value); break;
		case Leveller_ParamIndices::Leveller_LowCutQ: lowCutQ = Helpers::ParamToQ(value); break;

		case Leveller_ParamIndices::Leveller_Peak1Freq: peak1Freq = Helpers::ParamToFrequency(value); break;
		case Leveller_ParamIndices::Leveller_Peak1Gain: peak1Gain = Helpers::ParamToDb(value); break;
		case Leveller_ParamIndices::Leveller_Peak1Q: peak1Q = Helpers::ParamToQ(value); break;

		case Leveller_ParamIndices::Leveller_Peak2Freq: peak2Freq = Helpers::ParamToFrequency(value); break;
		case Leveller_ParamIndices::Leveller_Peak2Gain: peak2Gain = Helpers::ParamToDb(value); break;
		case Leveller_ParamIndices::Leveller_Peak2Q: peak2Q = Helpers::ParamToQ(value); break;

		case Leveller_ParamIndices::Leveller_Peak3Freq: peak3Freq = Helpers::ParamToFrequency(value); break;
		case Leveller_ParamIndices::Leveller_Peak3Gain: peak3Gain = Helpers::ParamToDb(value); break;
		case Leveller_ParamIndices::Leveller_Peak3Q: peak3Q = Helpers::ParamToQ(value); break;

		case Leveller_ParamIndices::Leveller_HighCutFreq: highCutFreq = Helpers::ParamToFrequency(value); break;
		case Leveller_ParamIndices::Leveller_HighCutQ: highCutQ = Helpers::ParamToQ(value);; break;

		case Leveller_ParamIndices::Leveller_Master: master = value; break;
		}
	}

	float Leveller::GetParam(int index) const
	{
		switch ((Leveller_ParamIndices)index)
		{
		case Leveller_ParamIndices::Leveller_LowCutFreq:
		default:
			return Helpers::FrequencyToParam(lowCutFreq);

		case Leveller_ParamIndices::Leveller_LowCutQ: return Helpers::QToParam(lowCutQ);

		case Leveller_ParamIndices::Leveller_Peak1Freq: return Helpers::FrequencyToParam(peak1Freq);
		case Leveller_ParamIndices::Leveller_Peak1Gain: return Helpers::DbToParam(peak1Gain);
		case Leveller_ParamIndices::Leveller_Peak1Q: return Helpers::QToParam(peak1Q);

		case Leveller_ParamIndices::Leveller_Peak2Freq: return Helpers::FrequencyToParam(peak2Freq);
		case Leveller_ParamIndices::Leveller_Peak2Gain: return Helpers::DbToParam(peak2Gain);
		case Leveller_ParamIndices::Leveller_Peak2Q: return Helpers::QToParam(peak2Q);

		case Leveller_ParamIndices::Leveller_Peak3Freq: return Helpers::FrequencyToParam(peak3Freq);
		case Leveller_ParamIndices::Leveller_Peak3Gain: return Helpers::DbToParam(peak3Gain);
		case Leveller_ParamIndices::Leveller_Peak3Q: return Helpers::QToParam(peak3Q);

		case Leveller_ParamIndices::Leveller_HighCutFreq: return Helpers::FrequencyToParam(highCutFreq);
		case Leveller_ParamIndices::Leveller_HighCutQ: return Helpers::QToParam(highCutQ);
		
		case Leveller_ParamIndices::Leveller_Master: return master;
		}
	}
}
