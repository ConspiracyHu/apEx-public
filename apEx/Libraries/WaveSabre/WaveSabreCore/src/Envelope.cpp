#include <WaveSabreCore/Envelope.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Envelope::Envelope()
	{
		State = EnvelopeState::EnvelopeState_Finished;
		Attack = 1.0f;
		Decay = 5.0f;
		Sustain = .5f;
		Release = 1.5f;
	}

	void Envelope::Trigger()
	{
		State = EnvelopeState::EnvelopeState_Attack;
		pos = 0.0f;
	}

	void Envelope::Off()
	{
		releaseValue = GetValue();
		State = EnvelopeState::EnvelopeState_Release;
		pos = 0.0f;
	}

	float Envelope::GetValue() const
	{
		switch (State)
		{
		case EnvelopeState::EnvelopeState_Attack:
			return pos / Attack;

		case EnvelopeState::EnvelopeState_Decay:
			{
				float f = 1.0f - pos / Decay;
				f *= f;
				return 1.0f * f + Sustain * (1.0f - f);
			}

		case EnvelopeState::EnvelopeState_Sustain:
			return Sustain;

		case EnvelopeState::EnvelopeState_Release:
			{
				float f = 1.0f - pos / Release;
				f *= f;
				return releaseValue * f;
			}

		default:
			return 0.0f;
		}
	}

	void Envelope::Next()
	{
		if (State == EnvelopeState::EnvelopeState_Finished) return;
		float posDelta = (float)(1000.0 / Helpers::CurrentSampleRate);
		switch (State)
		{
		case EnvelopeState::EnvelopeState_Attack:
			pos += posDelta;
			if (pos >= Attack)
			{
				State = EnvelopeState::EnvelopeState_Decay;
				pos -= Attack;
			}
			break;

		case EnvelopeState::EnvelopeState_Decay:
			pos += posDelta;
			if (pos >= Decay) State = EnvelopeState::EnvelopeState_Sustain;
			break;

		case EnvelopeState::EnvelopeState_Release:
			pos += posDelta;
			if (pos >= Release) State = EnvelopeState::EnvelopeState_Finished;
		}
	}
}
