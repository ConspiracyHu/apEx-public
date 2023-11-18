#include <WaveSabreCore/Falcon.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	Falcon::Falcon()
		: SynthDevice((int)Falcon_ParamIndices::Falcon_NumParams)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new FalconVoice(this);

		osc1Waveform = 0.0f;
		osc1RatioCoarse = 0.0f;
		osc1RatioFine = .5f;
		osc1Feedback = 0.0f;
		osc1FeedForward = 0.0f;

		osc1Attack = 1.0f;
		osc1Decay = 1.0f;
		osc1Sustain = 1.0f;
		osc1Release = 1.0f;

		osc2Waveform = 0.0f;
		osc2RatioCoarse = 0.0f;
		osc2RatioFine = .5f;
		osc2Feedback = 0.0f;

		osc2Attack = 1.0f;
		osc2Decay = 5.0f;
		osc2Sustain = .75f;
		osc2Release = 1.5f;

		masterLevel = .8f;

		pitchAttack = 1.0f;
		pitchDecay = 5.0f;
		pitchSustain = .5f;
		pitchRelease = 1.5f;
		pitchEnvAmt1 = 0.0f;
		pitchEnvAmt2 = 0.0f;
	}

	void Falcon::SetParam(int index, float value)
	{
		switch ((Falcon_ParamIndices)index)
		{
		case Falcon_ParamIndices::Falcon_Osc1Waveform: osc1Waveform = value; break;
		case Falcon_ParamIndices::Falcon_Osc1RatioCoarse: osc1RatioCoarse = value; break;
		case Falcon_ParamIndices::Falcon_Osc1RatioFine: osc1RatioFine = value; break;
		case Falcon_ParamIndices::Falcon_Osc1Feedback: osc1Feedback = value; break;
		case Falcon_ParamIndices::Falcon_Osc1FeedForward: osc1FeedForward = value; break;

		case Falcon_ParamIndices::Falcon_Osc1Attack: osc1Attack = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_Osc1Decay: osc1Decay = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_Osc1Sustain: osc1Sustain = value; break;
		case Falcon_ParamIndices::Falcon_Osc1Release: osc1Release = Helpers::ScalarToEnvValue(value); break;

		case Falcon_ParamIndices::Falcon_Osc2Waveform: osc2Waveform = value; break;
		case Falcon_ParamIndices::Falcon_Osc2RatioCoarse: osc2RatioCoarse = value; break;
		case Falcon_ParamIndices::Falcon_Osc2RatioFine: osc2RatioFine = value; break;
		case Falcon_ParamIndices::Falcon_Osc2Feedback: osc2Feedback = value; break;

		case Falcon_ParamIndices::Falcon_Osc2Attack: osc2Attack = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_Osc2Decay: osc2Decay = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_Osc2Sustain: osc2Sustain = value; break;
		case Falcon_ParamIndices::Falcon_Osc2Release: osc2Release = Helpers::ScalarToEnvValue(value); break;

		case Falcon_ParamIndices::Falcon_MasterLevel: masterLevel = value; break;

		case Falcon_ParamIndices::Falcon_VoicesUnisono: VoicesUnisono = Helpers::ParamToUnisono(value); break;
		case Falcon_ParamIndices::Falcon_VoicesDetune: VoicesDetune = value; break;
		case Falcon_ParamIndices::Falcon_VoicesPan: VoicesPan = value; break;

		case Falcon_ParamIndices::Falcon_VibratoFreq: VibratoFreq = Helpers::ParamToVibratoFreq(value); break;
		case Falcon_ParamIndices::Falcon_VibratoAmount: VibratoAmount = value; break;

		case Falcon_ParamIndices::Falcon_Rise: Rise = value; break;

		case Falcon_ParamIndices::Falcon_PitchAttack: pitchAttack = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_PitchDecay: pitchDecay = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_PitchSustain: pitchSustain = value; break;
		case Falcon_ParamIndices::Falcon_PitchRelease: pitchRelease = Helpers::ScalarToEnvValue(value); break;
		case Falcon_ParamIndices::Falcon_PitchEnvAmt1: pitchEnvAmt1 = (value - .5f) * 2.0f * 36.0f; break;
		case Falcon_ParamIndices::Falcon_PitchEnvAmt2: pitchEnvAmt2 = (value - .5f) * 2.0f * 36.0f; break;

		case Falcon_ParamIndices::Falcon_VoiceMode: SetVoiceMode(Helpers::ParamToVoiceMode(value)); break;
		case Falcon_ParamIndices::Falcon_SlideTime: Slide = value; break;
		}
	}

	float Falcon::GetParam(int index) const
	{
		switch ((Falcon_ParamIndices)index)
		{
		case Falcon_ParamIndices::Falcon_Osc1Waveform:
		default:
			return osc1Waveform;
		case Falcon_ParamIndices::Falcon_Osc1RatioCoarse: return osc1RatioCoarse;
		case Falcon_ParamIndices::Falcon_Osc1RatioFine: return osc1RatioFine;
		case Falcon_ParamIndices::Falcon_Osc1Feedback: return osc1Feedback;
		case Falcon_ParamIndices::Falcon_Osc1FeedForward: return osc1FeedForward;

		case Falcon_ParamIndices::Falcon_Osc1Attack: return Helpers::EnvValueToScalar(osc1Attack);
		case Falcon_ParamIndices::Falcon_Osc1Decay: return Helpers::EnvValueToScalar(osc1Decay);
		case Falcon_ParamIndices::Falcon_Osc1Sustain: return osc1Sustain;
		case Falcon_ParamIndices::Falcon_Osc1Release: return Helpers::EnvValueToScalar(osc1Release);

		case Falcon_ParamIndices::Falcon_Osc2Waveform: return osc2Waveform;
		case Falcon_ParamIndices::Falcon_Osc2RatioCoarse: return osc2RatioCoarse;
		case Falcon_ParamIndices::Falcon_Osc2RatioFine: return osc2RatioFine;
		case Falcon_ParamIndices::Falcon_Osc2Feedback: return osc2Feedback;

		case Falcon_ParamIndices::Falcon_Osc2Attack: return Helpers::EnvValueToScalar(osc2Attack);
		case Falcon_ParamIndices::Falcon_Osc2Decay: return Helpers::EnvValueToScalar(osc2Decay);
		case Falcon_ParamIndices::Falcon_Osc2Sustain: return osc2Sustain;
		case Falcon_ParamIndices::Falcon_Osc2Release: return Helpers::EnvValueToScalar(osc2Release);

		case Falcon_ParamIndices::Falcon_MasterLevel: return masterLevel;

		case Falcon_ParamIndices::Falcon_VoicesUnisono: return Helpers::UnisonoToParam(VoicesUnisono);
		case Falcon_ParamIndices::Falcon_VoicesDetune: return VoicesDetune;
		case Falcon_ParamIndices::Falcon_VoicesPan: return VoicesPan;

		case Falcon_ParamIndices::Falcon_VibratoFreq: return Helpers::VibratoFreqToParam(VibratoFreq);
		case Falcon_ParamIndices::Falcon_VibratoAmount: return VibratoAmount;

		case Falcon_ParamIndices::Falcon_Rise: return Rise;


		case Falcon_ParamIndices::Falcon_PitchAttack: return Helpers::EnvValueToScalar(pitchAttack);
		case Falcon_ParamIndices::Falcon_PitchDecay: return Helpers::EnvValueToScalar(pitchDecay);
		case Falcon_ParamIndices::Falcon_PitchSustain: return pitchSustain;
		case Falcon_ParamIndices::Falcon_PitchRelease: return Helpers::EnvValueToScalar(pitchRelease);
		case Falcon_ParamIndices::Falcon_PitchEnvAmt1: return pitchEnvAmt1 / 36.0f / 2.0f + .5f;
		case Falcon_ParamIndices::Falcon_PitchEnvAmt2: return pitchEnvAmt2 / 36.0f / 2.0f + .5f;

		case Falcon_ParamIndices::Falcon_VoiceMode: return Helpers::VoiceModeToParam(GetVoiceMode());
		case Falcon_ParamIndices::Falcon_SlideTime: return Slide;
		}
	}

	Falcon::FalconVoice::FalconVoice(Falcon *falcon)
	{
		this->falcon = falcon;
	}

	SynthDevice *Falcon::FalconVoice::SynthDevice() const
	{
		return falcon;
	}

	void Falcon::FalconVoice::Run(double songPosition, float **outputs, int numSamples)
	{
		double vibratoFreq = falcon->VibratoFreq / Helpers::CurrentSampleRate;

		double osc1Feedback = falcon->osc1Feedback * falcon->osc1Feedback / 2.0;
		double osc2Feedback = falcon->osc2Feedback * falcon->osc2Feedback / 2.0;

		float osc1FeedForwardScalar = falcon->osc1FeedForward * falcon->osc1FeedForward;
		float masterLevelScalar = Helpers::VolumeToScalar(falcon->masterLevel);

		float leftPanScalar = Helpers::PanToScalarLeft(Pan);
		float rightPanScalar = Helpers::PanToScalarRight(Pan);


		double osc1RatioScalar = ratioScalar((double)falcon->osc1RatioCoarse, (double)falcon->osc1RatioFine);
		double osc2RatioScalar = ratioScalar((double)falcon->osc2RatioCoarse, (double)falcon->osc2RatioFine);

		for (int i = 0; i < numSamples; i++)
		{
			double baseNote = GetNote() + Detune + falcon->Rise * 24.0f;

			double osc1Input = osc1Phase / Helpers::CurrentSampleRate * 2.0 * 3.141592 + osc1Output * osc1Feedback;
			osc1Output = ((Helpers::FastSin(osc1Input) + Helpers::Square35(osc1Input) * (double)falcon->osc1Waveform)) * osc1Env.GetValue() * 13.25;

			double osc2Input = osc2Phase / Helpers::CurrentSampleRate * 2.0 * 3.141592 + osc2Output * osc2Feedback * 13.25 + osc1Output * osc1FeedForwardScalar;
			osc2Output = ((Helpers::FastSin(osc2Input) + Helpers::Square35(osc2Input) * (double)falcon->osc2Waveform)) * osc2Env.GetValue();

			float finalOutput = (float)osc2Output * masterLevelScalar;
			outputs[0][i] += finalOutput * leftPanScalar;
			outputs[1][i] += finalOutput * rightPanScalar;

			osc2Env.Next();
			if (osc2Env.State == EnvelopeState::EnvelopeState_Finished)
			{
				IsOn = false;
				break;
			}

			float pEnv = pitchEnv.GetValue();
			double freq1 = Helpers::NoteToFreq(baseNote + pEnv * falcon->pitchEnvAmt1 + Helpers::FastSin(vibratoPhase) * falcon->VibratoAmount);
			double freq2 = Helpers::NoteToFreq(baseNote + pEnv * falcon->pitchEnvAmt2 + Helpers::FastSin(vibratoPhase) * falcon->VibratoAmount);
			osc1Phase += freq1 * osc1RatioScalar;
			osc2Phase += freq2 * osc2RatioScalar;
			vibratoPhase += vibratoFreq;
			osc1Env.Next();
			pitchEnv.Next();
		}
	}

	void Falcon::FalconVoice::NoteOn(int note, int velocity, float detune, float pan)
	{
		Voice::NoteOn(note, velocity, detune, pan);
		osc1Phase = osc2Phase = (double)Helpers::RandFloat();
		osc1Env.Attack = falcon->osc1Attack;
		osc1Env.Decay = falcon->osc1Decay;
		osc1Env.Sustain = falcon->osc1Sustain;
		osc1Env.Release = falcon->osc1Release;
		osc1Env.Trigger();
		osc2Env.Attack = falcon->osc2Attack;
		osc2Env.Decay = falcon->osc2Decay;
		osc2Env.Sustain = falcon->osc2Sustain;
		osc2Env.Release = falcon->osc2Release;
		osc2Env.Trigger();

		pitchEnv.Attack = falcon->pitchAttack;
		pitchEnv.Decay = falcon->pitchDecay;
		pitchEnv.Sustain = falcon->pitchSustain;
		pitchEnv.Release = falcon->pitchRelease;
		pitchEnv.Trigger();

		osc1Output = osc2Output = 0.0;
	}

	void Falcon::FalconVoice::NoteOff()
	{
		osc1Env.Off();
		osc2Env.Off();
		pitchEnv.Off();
	}

	double Falcon::ratioScalar(double coarse, double fine)
	{
		double fineBase = (fine - .5) * 2.0;
		return 1.0 + floor(coarse * 32.99) + fineBase * fineBase * fineBase;
	}
}
