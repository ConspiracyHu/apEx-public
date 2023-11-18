#ifndef __WAVESABRECORE_ADULTERY_H__
#define __WAVESABRECORE_ADULTERY_H__

#include "SynthDevice.h"
#include "Envelope.h"
#include "StateVariableFilter.h"
#include "SamplePlayer.h"

namespace WaveSabreCore
{
	class Adultery : public SynthDevice
	{
	public:
		enum Adultery_ParamIndices
		{
			Adultery_SampleIndex,

			Adultery_AmpAttack,
			Adultery_AmpDecay,
			Adultery_AmpSustain,
			Adultery_AmpRelease,

			Adultery_SampleStart,
			Adultery_LoopMode,
			Adultery_LoopBoundaryMode,
			Adultery_LoopStart,
			Adultery_LoopLength,
			Adultery_Reverse,

			Adultery_InterpolationMode,

			Adultery_CoarseTune,
			Adultery_FineTune,

			Adultery_FilterType,
			Adultery_FilterFreq,
			Adultery_FilterResonance,
			Adultery_FilterModAmt,

			Adultery_ModAttack,
			Adultery_ModDecay,
			Adultery_ModSustain,
			Adultery_ModRelease,

			Adultery_VoicesUnisono,
			Adultery_VoicesDetune,
			Adultery_VoicesPan,

			Adultery_Master,

			Adultery_VoiceMode,
			Adultery_SlideTime,

			Adultery_NumParams,
		};

		Adultery();
		virtual ~Adultery();

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	private:
		class AdulteryVoice : public Voice
		{
		public:
			AdulteryVoice(Adultery *adultery);
			virtual WaveSabreCore::SynthDevice *SynthDevice() const;

			virtual void Run(double songPosition, float **outputs, int numSamples);

			virtual void NoteOn(int note, int velocity, float detune, float pan);
			virtual void NoteOff();

		private:
			double coarseDetune(float detune);
			void calcPitch();

			Adultery *adultery;

			Envelope ampEnv, modEnv;

			SamplePlayer samplePlayer;

			StateVariableFilter filter;

			float velocity;
		};

		int sampleIndex;

		float ampAttack, ampDecay, ampSustain, ampRelease;
		float sampleStart;
		bool reverse;
		LoopMode loopMode;
		LoopBoundaryMode loopBoundaryMode;
		float loopStart, loopLength;

		InterpolationMode interpolationMode;

		float *sampleData;
		int sampleLength;
		int sampleLoopStart, sampleLoopLength;
		float coarseTune, fineTune;
		float masterLevel;
		StateVariableFilterType filterType;
		float filterFreq, filterResonance, filterModAmt;
		float modAttack, modDecay, modSustain, modRelease;
	};
}

#endif
