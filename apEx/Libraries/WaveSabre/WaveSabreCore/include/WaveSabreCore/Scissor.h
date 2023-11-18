#ifndef __WAVESABRECORE_SCISSOR_H__
#define __WAVESABRECORE_SCISSOR_H__

#include "Device.h"

namespace WaveSabreCore
{
	class Scissor : public Device
	{
	public:
		enum Scissor_ParamIndices
		{
			Scissor_Drive,
			Scissor_Threshold,
			Scissor_Foldover,

			Scissor_DryWet,

			Scissor_Type,

			Scissor_Oversampling,

			Scissor_NumParams,
		};

		Scissor();

		virtual void Run(double songPosition, float **inputs, float **outputs, int numSamples);

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		enum ShaperType
		{
			ShaperType_Clipper,
			ShaperType_Sine,
			ShaperType_Parabola,
		};

		enum Oversampling
		{
			Oversampling_X1,
			Oversampling_X2,
			Oversampling_X4,
		};

		float distort(float v, float driveScalar);

		ShaperType type;
		float drive, threshold, foldover, dryWet;
		Oversampling oversampling;

		float lastSample[2];
	};
}

#endif
