#ifndef __WAVESABRECORE_ENVELOPE_H__
#define __WAVESABRECORE_ENVELOPE_H__

namespace WaveSabreCore
{
	enum EnvelopeState
	{
		EnvelopeState_Attack,
		EnvelopeState_Decay,
		EnvelopeState_Sustain,
		EnvelopeState_Release,
		EnvelopeState_Finished,
	};

	class Envelope
	{
	public:
		Envelope();

		void Trigger();
		void Off();

		float GetValue() const;
		void Next();

		EnvelopeState State;

		float Attack, Decay, Sustain, Release;

	private:
		float pos;
		float releaseValue;
	};
}

#endif
