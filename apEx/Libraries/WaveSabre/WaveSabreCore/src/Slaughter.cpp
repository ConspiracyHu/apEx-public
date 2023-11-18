#include <WaveSabreCore/Slaughter.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Slaughter::Slaughter()
		: SynthDevice((int)Slaughter_ParamIndices::Slaughter_NumParams)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new SlaughterVoice(this);

		osc1Waveform = osc2Waveform = osc3Waveform = 0.0f;
		osc1PulseWidth = osc2PulseWidth = osc3PulseWidth = .5f;
		osc1DetuneCoarse = osc2DetuneCoarse = osc3DetuneCoarse = 0.0f;
		osc1DetuneFine = osc2DetuneFine = osc3DetuneFine = 0.0f;
		osc1Volume = 1.0f;
		osc2Volume = osc3Volume = noiseVolume = 0.0f;

		filterType = StateVariableFilterType::StateVariableFilterType_Lowpass;
		filterFreq = 20000.0f - 20.0f;
		filterResonance = 1.0f;
		filterModAmt = .5f;

		ampAttack = 1.0f;
		ampDecay = 5.0f;
		ampSustain = .5f;
		ampRelease = 1.5f;

		modAttack = 1.0f;
		modDecay = 5.0f;
		modSustain = 1.0f;
		modRelease = 1.5f;

		pitchAttack = 1.0f;
		pitchDecay = 5.0f;
		pitchSustain = .5f;
		pitchRelease = 1.5f;
		pitchEnvAmt = 0.0f;

		masterLevel = .5f;
	}

	void Slaughter::SetParam(int index, float value)
	{
		switch ((Slaughter_ParamIndices)index)
		{
		case Slaughter_ParamIndices::Slaughter_Osc1Waveform: osc1Waveform = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc1PulseWidth: osc1PulseWidth = 1.0f - value; break;
		case Slaughter_ParamIndices::Slaughter_Osc1Volume: osc1Volume = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc1DetuneCoarse: osc1DetuneCoarse = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc1DetuneFine: osc1DetuneFine = value; break;

		case Slaughter_ParamIndices::Slaughter_Osc2Waveform: osc2Waveform = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc2PulseWidth: osc2PulseWidth = 1.0f - value; break;
		case Slaughter_ParamIndices::Slaughter_Osc2Volume: osc2Volume = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc2DetuneCoarse: osc2DetuneCoarse = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc2DetuneFine: osc2DetuneFine = value; break;

		case Slaughter_ParamIndices::Slaughter_Osc3Waveform: osc3Waveform = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc3PulseWidth: osc3PulseWidth = 1.0f - value; break;
		case Slaughter_ParamIndices::Slaughter_Osc3Volume: osc3Volume = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc3DetuneCoarse: osc3DetuneCoarse = value; break;
		case Slaughter_ParamIndices::Slaughter_Osc3DetuneFine: osc3DetuneFine = value; break;

		case Slaughter_ParamIndices::Slaughter_NoiseVolume: noiseVolume = value; break;

		case Slaughter_ParamIndices::Slaughter_FilterType: filterType = Helpers::ParamToStateVariableFilterType(value); break;
		case Slaughter_ParamIndices::Slaughter_FilterFreq: filterFreq = Helpers::ParamToFrequency(value); break;
		case Slaughter_ParamIndices::Slaughter_FilterResonance: filterResonance = 1.0f - value; break;
		case Slaughter_ParamIndices::Slaughter_FilterModAmt: filterModAmt = value; break;

		case Slaughter_ParamIndices::Slaughter_AmpAttack: ampAttack = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_AmpDecay: ampDecay = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_AmpSustain: ampSustain = value; break;
		case Slaughter_ParamIndices::Slaughter_AmpRelease: ampRelease = Helpers::ScalarToEnvValue(value); break;

		case Slaughter_ParamIndices::Slaughter_ModAttack: modAttack = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_ModDecay: modDecay = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_ModSustain: modSustain = value; break;
		case Slaughter_ParamIndices::Slaughter_ModRelease: modRelease = Helpers::ScalarToEnvValue(value); break;

		case Slaughter_ParamIndices::Slaughter_PitchAttack: pitchAttack = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_PitchDecay: pitchDecay = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_PitchSustain: pitchSustain = value; break;
		case Slaughter_ParamIndices::Slaughter_PitchRelease: pitchRelease = Helpers::ScalarToEnvValue(value); break;
		case Slaughter_ParamIndices::Slaughter_PitchEnvAmt: pitchEnvAmt = (value - .5f) * 2.0f * 36.0f; break;

		case Slaughter_ParamIndices::Slaughter_MasterLevel: masterLevel = value; break;

		case Slaughter_ParamIndices::Slaughter_VoicesUnisono: VoicesUnisono = Helpers::ParamToUnisono(value); break;
		case Slaughter_ParamIndices::Slaughter_VoicesDetune: VoicesDetune = value; break;
		case Slaughter_ParamIndices::Slaughter_VoicesPan: VoicesPan = value; break;

		case Slaughter_ParamIndices::Slaughter_VibratoFreq: VibratoFreq = Helpers::ParamToVibratoFreq(value); break;
		case Slaughter_ParamIndices::Slaughter_VibratoAmount: VibratoAmount = value; break;

		case Slaughter_ParamIndices::Slaughter_Rise: Rise = value; break;

		case Slaughter_ParamIndices::Slaughter_VoiceMode: SetVoiceMode(Helpers::ParamToVoiceMode(value)); break;
		case Slaughter_ParamIndices::Slaughter_SlideTime: Slide = value; break;
		}
	}

	float Slaughter::GetParam(int index) const
	{
		switch ((Slaughter_ParamIndices)index)
		{
		case Slaughter_ParamIndices::Slaughter_Osc1Waveform:
		default:
			return osc1Waveform;

		case Slaughter_ParamIndices::Slaughter_Osc1PulseWidth: return 1.0f - osc1PulseWidth;
		case Slaughter_ParamIndices::Slaughter_Osc1Volume: return osc1Volume;
		case Slaughter_ParamIndices::Slaughter_Osc1DetuneCoarse: return osc1DetuneCoarse;
		case Slaughter_ParamIndices::Slaughter_Osc1DetuneFine: return osc1DetuneFine;

		case Slaughter_ParamIndices::Slaughter_Osc2Waveform: return osc2Waveform;
		case Slaughter_ParamIndices::Slaughter_Osc2PulseWidth: return 1.0f - osc2PulseWidth;
		case Slaughter_ParamIndices::Slaughter_Osc2Volume: return osc2Volume;
		case Slaughter_ParamIndices::Slaughter_Osc2DetuneCoarse: return osc2DetuneCoarse;
		case Slaughter_ParamIndices::Slaughter_Osc2DetuneFine: return osc2DetuneFine;

		case Slaughter_ParamIndices::Slaughter_Osc3Waveform: return osc3Waveform;
		case Slaughter_ParamIndices::Slaughter_Osc3PulseWidth: return 1.0f - osc3PulseWidth;
		case Slaughter_ParamIndices::Slaughter_Osc3Volume: return osc3Volume;
		case Slaughter_ParamIndices::Slaughter_Osc3DetuneCoarse: return osc3DetuneCoarse;
		case Slaughter_ParamIndices::Slaughter_Osc3DetuneFine: return osc3DetuneFine;

		case Slaughter_ParamIndices::Slaughter_NoiseVolume: return noiseVolume;

		case Slaughter_ParamIndices::Slaughter_FilterType: return Helpers::StateVariableFilterTypeToParam(filterType);
		case Slaughter_ParamIndices::Slaughter_FilterFreq: return Helpers::FrequencyToParam(filterFreq);
		case Slaughter_ParamIndices::Slaughter_FilterResonance: return 1.0f - filterResonance;
		case Slaughter_ParamIndices::Slaughter_FilterModAmt: return filterModAmt;

		case Slaughter_ParamIndices::Slaughter_AmpAttack: return Helpers::EnvValueToScalar(ampAttack);
		case Slaughter_ParamIndices::Slaughter_AmpDecay: return Helpers::EnvValueToScalar(ampDecay);
		case Slaughter_ParamIndices::Slaughter_AmpSustain: return ampSustain;
		case Slaughter_ParamIndices::Slaughter_AmpRelease: return Helpers::EnvValueToScalar(ampRelease);

		case Slaughter_ParamIndices::Slaughter_ModAttack: return Helpers::EnvValueToScalar(modAttack);
		case Slaughter_ParamIndices::Slaughter_ModDecay: return Helpers::EnvValueToScalar(modDecay);
		case Slaughter_ParamIndices::Slaughter_ModSustain: return modSustain;
		case Slaughter_ParamIndices::Slaughter_ModRelease: return Helpers::EnvValueToScalar(modRelease);

		case Slaughter_ParamIndices::Slaughter_PitchAttack: return Helpers::EnvValueToScalar(pitchAttack);
		case Slaughter_ParamIndices::Slaughter_PitchDecay: return Helpers::EnvValueToScalar(pitchDecay);
		case Slaughter_ParamIndices::Slaughter_PitchSustain: return pitchSustain;
		case Slaughter_ParamIndices::Slaughter_PitchRelease: return Helpers::EnvValueToScalar(pitchRelease);
		case Slaughter_ParamIndices::Slaughter_PitchEnvAmt: return pitchEnvAmt / 36.0f / 2.0f + .5f;

		case Slaughter_ParamIndices::Slaughter_MasterLevel: return masterLevel;

		case Slaughter_ParamIndices::Slaughter_VoicesUnisono: return Helpers::UnisonoToParam(VoicesUnisono);
		case Slaughter_ParamIndices::Slaughter_VoicesDetune: return VoicesDetune;
		case Slaughter_ParamIndices::Slaughter_VoicesPan: return VoicesPan;

		case Slaughter_ParamIndices::Slaughter_VibratoFreq: return Helpers::VibratoFreqToParam(VibratoFreq);
		case Slaughter_ParamIndices::Slaughter_VibratoAmount: return VibratoAmount;

		case Slaughter_ParamIndices::Slaughter_Rise: return Rise;

		case Slaughter_ParamIndices::Slaughter_VoiceMode: return Helpers::VoiceModeToParam(GetVoiceMode());
		case Slaughter_ParamIndices::Slaughter_SlideTime: return Slide;
		}
	}

	Slaughter::SlaughterVoice::SlaughterVoice(Slaughter *slaughter)
	{
		this->slaughter = slaughter;

		osc1.Phase = (double)Helpers::RandFloat() * 2.0 * 3.141592;
		osc2.Phase = (double)Helpers::RandFloat() * 2.0 * 3.141592;
		osc3.Phase = (double)Helpers::RandFloat() * 2.0 * 3.141592;
		osc1.Integral = osc2.Integral = osc3.Integral = 0.0;
	}

	SynthDevice *Slaughter::SlaughterVoice::SynthDevice() const
	{
		return slaughter;
	}

	void Slaughter::SlaughterVoice::Run(double songPosition, float **outputs, int numSamples)
	{
		double vibratoFreq = slaughter->VibratoFreq / Helpers::CurrentSampleRate;

		filter.SetType(slaughter->filterType);
		filter.SetQ(slaughter->filterResonance);

		float amp = -16.0f * Helpers::VolumeToScalar(slaughter->masterLevel);
		float panLeft = Helpers::PanToScalarLeft(Pan);
		float panRight = Helpers::PanToScalarRight(Pan);

		double osc1Detune = coarseDetune(slaughter->osc1DetuneCoarse) + (double)slaughter->osc1DetuneFine;
		double osc2Detune = coarseDetune(slaughter->osc2DetuneCoarse) + (double)slaughter->osc2DetuneFine;
		double osc3Detune = coarseDetune(slaughter->osc3DetuneCoarse) + (double)slaughter->osc3DetuneFine;

		float osc1VolumeScalar = slaughter->osc1Volume * slaughter->osc1Volume;
		float osc2VolumeScalar = slaughter->osc2Volume * slaughter->osc2Volume;
		float osc3VolumeScalar = slaughter->osc3Volume * slaughter->osc3Volume;
		float noiseScalar = slaughter->noiseVolume * slaughter->noiseVolume;

		for (int i = 0; i < numSamples; i++)
		{
			filter.SetFreq(Helpers::Clamp(slaughter->filterFreq + modEnv.GetValue() * (20000.0f - 20.0f) * (slaughter->filterModAmt * 2.0f - 1.0f), 0.0f, 20000.0f - 20.0f));

			double baseNote = GetNote() + Detune + pitchEnv.GetValue() * slaughter->pitchEnvAmt + Helpers::FastSin(vibratoPhase) * slaughter->VibratoAmount + slaughter->Rise * 24.0f;
			float oscMix = 0.0;
			if (osc1VolumeScalar > 0.0f) oscMix += (float)(osc1.Next(baseNote + osc1Detune, slaughter->osc1Waveform, slaughter->osc1PulseWidth) * osc1VolumeScalar);
			if (osc2VolumeScalar > 0.0f) oscMix += (float)(osc2.Next(baseNote + osc2Detune, slaughter->osc2Waveform, slaughter->osc2PulseWidth) * osc2VolumeScalar);
			if (osc3VolumeScalar > 0.0f) oscMix += (float)(osc3.Next(baseNote + osc3Detune, slaughter->osc3Waveform, slaughter->osc3PulseWidth) * osc3VolumeScalar);
			if (noiseScalar > 0.0f) oscMix += Helpers::RandFloat() * noiseScalar;
			float out = filter.Next(oscMix) * ampEnv.GetValue() * amp;
			outputs[0][i] += out * panLeft;
			outputs[1][i] += out * panRight;

			ampEnv.Next();
			if (ampEnv.State == EnvelopeState::EnvelopeState_Finished)
			{
				IsOn = false;
				break;
			}
			vibratoPhase += vibratoFreq;
			modEnv.Next();
			pitchEnv.Next();
		}
	}

	void Slaughter::SlaughterVoice::NoteOn(int note, int velocity, float detune, float pan)
	{
		Voice::NoteOn(note, velocity, detune, pan);

		ampEnv.Attack = slaughter->ampAttack;
		ampEnv.Decay = slaughter->ampDecay;
		ampEnv.Sustain = slaughter->ampSustain;
		ampEnv.Release = slaughter->ampRelease;
		ampEnv.Trigger();

		modEnv.Attack = slaughter->modAttack;
		modEnv.Decay = slaughter->modDecay;
		modEnv.Sustain = slaughter->modSustain;
		modEnv.Release = slaughter->modRelease;
		modEnv.Trigger();

		pitchEnv.Attack = slaughter->pitchAttack;
		pitchEnv.Decay = slaughter->pitchDecay;
		pitchEnv.Sustain = slaughter->pitchSustain;
		pitchEnv.Release = slaughter->pitchRelease;
		pitchEnv.Trigger();
	}

	void Slaughter::SlaughterVoice::NoteOff()
	{
		ampEnv.Off();
		modEnv.Off();
		pitchEnv.Off();
	}

	float Slaughter::SlaughterVoice::Oscillator::Next(double note, float waveform, float pulseWidth)
	{
		double phaseMax = Helpers::CurrentSampleRate * .5 / Helpers::NoteToFreq(note);
		double dcOffset = -.498 / phaseMax;

		double phase2 = fmod(Phase + 2.0 * phaseMax * (double)pulseWidth, phaseMax * 2.0) - phaseMax;
		Phase = fmod(Phase + 1.0, phaseMax * 2.0);
		double tmpPhase = Phase - phaseMax;

		double blit1, blit2;
		const double epsilon = .0000001;
		if (tmpPhase > epsilon || tmpPhase < -epsilon)
		{
			tmpPhase *= 3.141592;
			blit1 = Helpers::FastSin(tmpPhase) / tmpPhase;
		}
		else blit1 = 1.0;
		if (phase2 > epsilon || phase2 < -epsilon)
		{
			phase2 *= 3.141592;
			blit2 = Helpers::FastSin(phase2) / phase2;
		}
		else blit2 = 1.0;

		Integral = .998 * Integral + dcOffset * (1.0 - (double)waveform) + blit1 - blit2 * (double)waveform;
		return (float)Integral;
	}

	double Slaughter::SlaughterVoice::coarseDetune(float detune)
	{
		return floor(detune * 24.99f);
	}
}
