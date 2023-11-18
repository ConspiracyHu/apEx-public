#ifndef __WAVESABRECORE_FALCON_H__
#define __WAVESABRECORE_FALCON_H__

#include "SynthDevice.h"
#include "Envelope.h"

namespace WaveSabreCore
{
	class Falcon : public SynthDevice
	{
	public:
		enum Falcon_ParamIndices
		{
			Falcon_Osc1Waveform,
			Falcon_Osc1RatioCoarse,
			Falcon_Osc1RatioFine,
			Falcon_Osc1Feedback,
			Falcon_Osc1FeedForward,

			Falcon_Osc1Attack,
			Falcon_Osc1Decay,
			Falcon_Osc1Sustain,
			Falcon_Osc1Release,

			Falcon_Osc2Waveform,
			Falcon_Osc2RatioCoarse,
			Falcon_Osc2RatioFine,
			Falcon_Osc2Feedback,

			Falcon_Osc2Attack,
			Falcon_Osc2Decay,
			Falcon_Osc2Sustain,
			Falcon_Osc2Release,

			Falcon_MasterLevel,

			Falcon_VoicesUnisono,
			Falcon_VoicesDetune,
			Falcon_VoicesPan,

			Falcon_VibratoFreq,
			Falcon_VibratoAmount,

			Falcon_Rise,

			Falcon_PitchAttack,
			Falcon_PitchDecay,
			Falcon_PitchSustain,
			Falcon_PitchRelease,
			Falcon_PitchEnvAmt1,
			Falcon_PitchEnvAmt2,

			Falcon_VoiceMode,
			Falcon_SlideTime,

			Falcon_NumParams,
		};

		Falcon();

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	protected:
		class FalconVoice : public Voice
		{
		public:
			FalconVoice(Falcon *falcon);
			virtual WaveSabreCore::SynthDevice *SynthDevice() const;

			virtual void Run(double songPosition, float **outputs, int numSamples);

			virtual void NoteOn(int note, int velocity, float detune, float pan);
			virtual void NoteOff();

		private:
			Falcon *falcon;

			Envelope osc1Env, osc2Env, pitchEnv;

			double osc1Phase, osc2Phase;
			double osc1Output, osc2Output;
		};

		static double ratioScalar(double coarse, double fine);

		float osc1Waveform, osc1RatioCoarse, osc1RatioFine, osc1Feedback, osc1FeedForward;
		float osc1Attack, osc1Decay, osc1Sustain, osc1Release;
		float osc2Waveform, osc2RatioCoarse, osc2RatioFine, osc2Feedback;
		float osc2Attack, osc2Decay, osc2Sustain, osc2Release;
		float masterLevel;
		float pitchAttack, pitchDecay, pitchSustain, pitchRelease, pitchEnvAmt1, pitchEnvAmt2;
	};
}

#endif
