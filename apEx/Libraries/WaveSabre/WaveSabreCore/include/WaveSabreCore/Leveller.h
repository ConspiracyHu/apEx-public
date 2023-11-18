#ifndef __WAVESABRECORE_LEVELLER_H__
#define __WAVESABRECORE_LEVELLER_H__

#include "Device.h"
#include "BiquadFilter.h"

namespace WaveSabreCore
{
	class Leveller : public Device
	{
	public:
		enum Leveller_ParamIndices
		{
			Leveller_LowCutFreq,
			Leveller_LowCutQ,

			Leveller_Peak1Freq,
			Leveller_Peak1Gain,
			Leveller_Peak1Q,

			Leveller_Peak2Freq,
			Leveller_Peak2Gain,
			Leveller_Peak2Q,

			Leveller_Peak3Freq,
			Leveller_Peak3Gain,
			Leveller_Peak3Q,

			Leveller_HighCutFreq,
			Leveller_HighCutQ,

			Leveller_Master,

			Leveller_NumParams,
		};

		Leveller();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		float lowCutFreq, lowCutQ;
		float peak1Freq, peak1Gain, peak1Q;
		float peak2Freq, peak2Gain, peak2Q;
		float peak3Freq, peak3Gain, peak3Q;
		float highCutFreq, highCutQ;
		float master;

		BiquadFilter highpass[2];
		BiquadFilter peak1[2];
		BiquadFilter peak2[2];
		BiquadFilter peak3[2];
		BiquadFilter lowpass[2];
	};
}

#endif
