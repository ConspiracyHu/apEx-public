#ifndef __WAVESABRECORE_SAMPLEPLAYER_H__
#define __WAVESABRECORE_SAMPLEPLAYER_H__

namespace WaveSabreCore
{
	enum InterpolationMode
	{
		InterpolationMode_Nearest,
		InterpolationMode_Linear,

		InterpolationMode_NumInterpolationModes,
	};

	enum LoopMode
	{
		LoopMode_Disabled,
		LoopMode_Repeat,
		LoopMode_PingPong,

		LoopMode_NumLoopModes,
	};

	enum LoopBoundaryMode
	{
		LoopBoundaryMode_FromSample,
		LoopBoundaryMode_Manual,

		LoopBoundaryMode_NumLoopBoundaryModes,
	};

	class SamplePlayer
	{
	public:
		SamplePlayer();

		void CalcPitch(double note);
		void InitPos();
		void RunPrep();
		float Next();

		bool IsActive;

		float SampleStart;
		bool Reverse;
		WaveSabreCore::LoopMode LoopMode;
		WaveSabreCore::LoopBoundaryMode LoopBoundaryMode;
		float LoopStart, LoopLength;

		WaveSabreCore::InterpolationMode InterpolationMode;

		float *SampleData;
		int SampleLength;
		int SampleLoopStart, SampleLoopLength;

	private:
		double samplePos, sampleDelta;
		int roundedLoopStart, roundedLoopLength, roundedLoopEnd;
		bool reverse;
	};
}

#endif
