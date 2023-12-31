#ifndef __WAVESABRECORE_SMASHER_H__
#define __WAVESABRECORE_SMASHER_H__

#include "Device.h"
#include "DelayBuffer.h"

namespace WaveSabreCore
{
	class Smasher : public Device
	{
	public:
		enum Smasher_ParamIndices
		{
			Smasher_Sidechain,
			Smasher_InputGain,
			Smasher_Threshold,
			Smasher_Ratio,
			Smasher_Attack,
			Smasher_Release,
			Smasher_OutputGain,

			Smasher_NumParams,
		};

		Smasher();
		virtual ~Smasher();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		const static float lookaheadMs;

		bool sidechain;
		float inputGain;
		float threshold, ratio;
		float attack, release;
		float outputGain;

		DelayBuffer leftBuffer;
		DelayBuffer rightBuffer;
		float peak;
	};
}

#endif
