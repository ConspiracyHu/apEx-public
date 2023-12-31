#ifndef __WAVESABRECORE_TWISTER_H__
#define __WAVESABRECORE_TWISTER_H__

#include "Device.h"
#include "ResampleBuffer.h"
#include "AllPassDelay.h"
#include "StateVariableFilter.h"

namespace WaveSabreCore
{
	enum Spread
	{
		Spread_Mono,
		Spread_FullInvert,
		Spread_ModInvert
	};

	class Twister : public Device
	{
	public:
		enum Twister_ParamIndices
		{
			Twister_Type,

			Twister_Amount,
			Twister_Feedback,

			Twister_Spread,

			Twister_VibratoFreq,
			Twister_VibratoAmount,

			Twister_LowCutFreq,
			Twister_HighCutFreq,

			Twister_DryWet,

			Twister_NumParams,
		};


		Twister();
		virtual ~Twister();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		int type;
		float amount, feedback;
		Spread spread;
		double vibratoFreq;
		float vibratoAmount;
		double vibratoPhase;
		float lowCutFreq, highCutFreq;
		float dryWet;
		float lastLeft, lastRight;

		AllPassDelay allPassLeft[6];
		AllPassDelay allPassRight[6];
		float AllPassUpdateLeft(float input);
		float AllPassUpdateRight(float input);

		ResampleBuffer leftBuffer;
		ResampleBuffer rightBuffer;
		
		StateVariableFilter lowCutFilter[2], highCutFilter[2];
	};
}

#endif
