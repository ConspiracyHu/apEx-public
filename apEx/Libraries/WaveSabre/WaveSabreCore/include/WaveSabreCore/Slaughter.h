#ifndef __WAVESABRECORE_SLAUGHTER_H__
#define __WAVESABRECORE_SLAUGHTER_H__

#include "SynthDevice.h"
#include "Envelope.h"
#include "StateVariableFilter.h"

namespace WaveSabreCore
{
	class Slaughter : public SynthDevice
	{
	public:
		enum Slaughter_ParamIndices
		{
			Slaughter_Osc1Waveform,
			Slaughter_Osc1PulseWidth,
			Slaughter_Osc1Volume,
			Slaughter_Osc1DetuneCoarse,
			Slaughter_Osc1DetuneFine,

			Slaughter_Osc2Waveform,
			Slaughter_Osc2PulseWidth,
			Slaughter_Osc2Volume,
			Slaughter_Osc2DetuneCoarse,
			Slaughter_Osc2DetuneFine,

			Slaughter_Osc3Waveform,
			Slaughter_Osc3PulseWidth,
			Slaughter_Osc3Volume,
			Slaughter_Osc3DetuneCoarse,
			Slaughter_Osc3DetuneFine,

			Slaughter_NoiseVolume,

			Slaughter_FilterType,
			Slaughter_FilterFreq,
			Slaughter_FilterResonance,
			Slaughter_FilterModAmt,

			Slaughter_AmpAttack,
			Slaughter_AmpDecay,
			Slaughter_AmpSustain,
			Slaughter_AmpRelease,

			Slaughter_ModAttack,
			Slaughter_ModDecay,
			Slaughter_ModSustain,
			Slaughter_ModRelease,

			Slaughter_MasterLevel,

			Slaughter_VoicesUnisono,
			Slaughter_VoicesDetune,
			Slaughter_VoicesPan,

			Slaughter_VibratoFreq,
			Slaughter_VibratoAmount,

			Slaughter_Rise,

			Slaughter_PitchAttack,
			Slaughter_PitchDecay,
			Slaughter_PitchSustain,
			Slaughter_PitchRelease,
			Slaughter_PitchEnvAmt,

			Slaughter_VoiceMode,
			Slaughter_SlideTime,

			Slaughter_NumParams,
		};

		Slaughter();

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

	protected:
		class SlaughterVoice : public Voice
		{
		public:
			SlaughterVoice(Slaughter *slaughter);
			virtual WaveSabreCore::SynthDevice *SynthDevice() const;

			virtual void Run(double songPosition, float **outputs, int numSamples);

			virtual void NoteOn(int note, int velocity, float detune, float pan);
			virtual void NoteOff();

		private:
			class Oscillator
			{
			public:
				float Next(double note, float waveform, float pulseWidth);

				double Phase;
				double Integral;
			};

			double coarseDetune(float detune);

			Slaughter *slaughter;

			Envelope ampEnv, modEnv, pitchEnv;

			Oscillator osc1, osc2, osc3;
			StateVariableFilter filter;
		};

		float osc1Waveform, osc1PulseWidth, osc1Volume, osc1DetuneCoarse, osc1DetuneFine;
		float osc2Waveform, osc2PulseWidth, osc2Volume, osc2DetuneCoarse, osc2DetuneFine;
		float osc3Waveform, osc3PulseWidth, osc3Volume, osc3DetuneCoarse, osc3DetuneFine;
		float noiseVolume;
		StateVariableFilterType filterType;
		float filterFreq, filterResonance, filterModAmt;
		float ampAttack, ampDecay, ampSustain, ampRelease;
		float modAttack, modDecay, modSustain, modRelease;
		float pitchAttack, pitchDecay, pitchSustain, pitchRelease, pitchEnvAmt;
		float masterLevel;
	};
}

#endif
