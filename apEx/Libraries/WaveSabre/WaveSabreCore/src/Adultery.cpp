#include <WaveSabreCore/Adultery.h>
#include <WaveSabreCore/Helpers.h>
#include <WaveSabreCore/GmDls.h>

#include <string.h>
#include <math.h>

typedef struct
{
	char tag[4];
	unsigned int size;
	short wChannels;
	int dwSamplesPerSec;
	int dwAvgBytesPerSec;
	short wBlockAlign;
} Fmt;

typedef struct
{
	char tag[4];
	unsigned int size;
	unsigned short unityNote;
	short fineTune;
	int gain;
	int attenuation;
	unsigned int fulOptions;
	unsigned int loopCount;
	unsigned int loopSize;
	unsigned int loopType;
	unsigned int loopStart;
	unsigned int loopLength;
} Wsmp;

namespace WaveSabreCore
{
	Adultery::Adultery()
		: SynthDevice((int)Adultery_ParamIndices::Adultery_NumParams)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new AdulteryVoice(this);

		sampleIndex = -1;

		ampAttack = 1.0f;
		ampDecay = 1.0f;
		ampSustain = 1.0f;
		ampRelease = 1.0f;

		sampleStart = 0.0f;
		reverse = false;
		loopMode = LoopMode::LoopMode_Repeat;
		loopBoundaryMode = LoopBoundaryMode::LoopBoundaryMode_FromSample;
		loopStart = 0.0f;
		loopLength = 1.0f;
		sampleLoopStart = 0;
		sampleLoopLength = 0;

		interpolationMode = InterpolationMode::InterpolationMode_Linear;

		sampleData = nullptr;
		sampleLength = 0;

		coarseTune = 0.5f;
		fineTune = 0.5f;

		filterType = StateVariableFilterType::StateVariableFilterType_Lowpass;
		filterFreq = 20000.0f - 20.0f;
		filterResonance = 1.0f;
		filterModAmt = .5f;

		modAttack = 1.0f;
		modDecay = 5.0f;
		modSustain = 1.0f;
		modRelease = 1.5f;

		masterLevel = 0.5f;
	}

	Adultery::~Adultery()
	{
		if (sampleData)
			delete [] sampleData;
	}

	void Adultery::SetParam(int index, float value)
	{
		switch ((Adultery_ParamIndices)index)
		{
		case Adultery_ParamIndices::Adultery_SampleIndex:
			sampleIndex = (int)value - 1;

			if (sampleData)
			{
				delete [] sampleData;
				sampleData = nullptr;
				sampleLength = 0;
			}

			if (sampleIndex >= 0)
			{
				auto gmDls = GmDls::Load();

				// Seek to wave pool chunk's data
				auto ptr = gmDls + GmDls::WaveListOffset;

				// Walk wave pool entries
				for (int i = 0; i <= sampleIndex; i++)
				{
					// Walk wave list
					auto waveListTag = *((unsigned int *)ptr); // Should be 'LIST'
					ptr += 4;
					auto waveListSize = *((unsigned int *)ptr);
					ptr += 4;

					// Skip entries until we've reached the index we're looking for
					if (i != sampleIndex)
					{
						ptr += waveListSize;
						continue;
					}

					// Walk wave entry
					auto wave = ptr;
					auto waveTag = *((unsigned int *)wave); // Should be 'wave'
					wave += 4;

					// Read fmt chunk
					Fmt fmt;
					memcpy(&fmt, wave, sizeof(Fmt));
					wave += fmt.size + 8; // size field doesn't account for tag or length fields

					// Read wsmp chunk
					Wsmp wsmp;
					memcpy(&wsmp, wave, sizeof(Wsmp));
					wave += wsmp.size + 8; // size field doesn't account for tag or length fields

					// Read data chunk
					auto dataChunkTag = *((unsigned int *)wave); // Should be 'data'
					wave += 4;
					auto dataChunkSize = *((unsigned int *)wave);
					wave += 4;

					// Data format is assumed to be mono 16-bit signed PCM
					sampleLength = dataChunkSize / 2;
					sampleData = new float[sampleLength];
					for (int j = 0; j < sampleLength; j++)
					{
						auto sample = *((short *)wave);
						wave += 2;
						sampleData[j] = (float)((double)sample / 32768.0);
					}

					if (wsmp.loopCount)
					{
						sampleLoopStart = wsmp.loopStart;
						sampleLoopLength = wsmp.loopLength;
					}
					else
					{
						sampleLoopStart = 0;
						sampleLoopLength = sampleLength;
					}
				}

				delete [] gmDls;
			}
			break;

		case Adultery_ParamIndices::Adultery_AmpAttack: ampAttack = Helpers::ScalarToEnvValue(value); break;
		case Adultery_ParamIndices::Adultery_AmpDecay: ampDecay = Helpers::ScalarToEnvValue(value); break;
		case Adultery_ParamIndices::Adultery_AmpSustain: ampSustain = value; break;
		case Adultery_ParamIndices::Adultery_AmpRelease: ampRelease = Helpers::ScalarToEnvValue(value); break;

		case Adultery_ParamIndices::Adultery_SampleStart: sampleStart = value; break;
		case Adultery_ParamIndices::Adultery_Reverse: reverse = Helpers::ParamToBoolean(value); break;
		case Adultery_ParamIndices::Adultery_LoopMode: loopMode = (LoopMode)(int)(value * (float)((int)LoopMode::LoopMode_NumLoopModes - 1)); break;
		case Adultery_ParamIndices::Adultery_LoopBoundaryMode: loopBoundaryMode = (LoopBoundaryMode)(int)(value * (float)((int)LoopBoundaryMode::LoopBoundaryMode_NumLoopBoundaryModes - 1)); break;
		case Adultery_ParamIndices::Adultery_LoopStart: loopStart = value; break;
		case Adultery_ParamIndices::Adultery_LoopLength: loopLength = value; break;

		case Adultery_ParamIndices::Adultery_InterpolationMode: interpolationMode = (InterpolationMode)(int)(value * (float)((int)InterpolationMode::InterpolationMode_NumInterpolationModes - 1)); break;

		case Adultery_ParamIndices::Adultery_CoarseTune: coarseTune = value; break;
		case Adultery_ParamIndices::Adultery_FineTune: fineTune = value; break;

		case Adultery_ParamIndices::Adultery_FilterType: filterType = Helpers::ParamToStateVariableFilterType(value); break;
		case Adultery_ParamIndices::Adultery_FilterFreq: filterFreq = Helpers::ParamToFrequency(value); break;
		case Adultery_ParamIndices::Adultery_FilterResonance: filterResonance = 1.0f - value; break;
		case Adultery_ParamIndices::Adultery_FilterModAmt: filterModAmt = value; break;

		case Adultery_ParamIndices::Adultery_ModAttack: modAttack = Helpers::ScalarToEnvValue(value); break;
		case Adultery_ParamIndices::Adultery_ModDecay: modDecay = Helpers::ScalarToEnvValue(value); break;
		case Adultery_ParamIndices::Adultery_ModSustain: modSustain = value; break;
		case Adultery_ParamIndices::Adultery_ModRelease: modRelease = Helpers::ScalarToEnvValue(value); break;

		case Adultery_ParamIndices::Adultery_VoicesUnisono: VoicesUnisono = Helpers::ParamToUnisono(value); break;
		case Adultery_ParamIndices::Adultery_VoicesDetune: VoicesDetune = value; break;
		case Adultery_ParamIndices::Adultery_VoicesPan: VoicesPan = value; break;

		case Adultery_ParamIndices::Adultery_VoiceMode: SetVoiceMode(Helpers::ParamToVoiceMode(value)); break;
		case Adultery_ParamIndices::Adultery_SlideTime: Slide = value; break;

		case Adultery_ParamIndices::Adultery_Master: masterLevel = value; break;
		}
	}

	float Adultery::GetParam(int index) const
	{
		switch ((Adultery_ParamIndices)index)
		{
		case Adultery_ParamIndices::Adultery_SampleIndex:
		default:
			return (float)(sampleIndex + 1);

		case Adultery_ParamIndices::Adultery_AmpAttack: return Helpers::EnvValueToScalar(ampAttack);
		case Adultery_ParamIndices::Adultery_AmpDecay: return Helpers::EnvValueToScalar(ampDecay);
		case Adultery_ParamIndices::Adultery_AmpSustain: return ampSustain;
		case Adultery_ParamIndices::Adultery_AmpRelease: return Helpers::EnvValueToScalar(ampRelease);

		case Adultery_ParamIndices::Adultery_SampleStart: return sampleStart;
		case Adultery_ParamIndices::Adultery_Reverse: return Helpers::BooleanToParam(reverse);
		case Adultery_ParamIndices::Adultery_LoopMode: return (float)loopMode / (float)((int)LoopMode::LoopMode_NumLoopModes - 1);
		case Adultery_ParamIndices::Adultery_LoopBoundaryMode: return (float)loopBoundaryMode / (float)((int)LoopBoundaryMode::LoopBoundaryMode_NumLoopBoundaryModes - 1);
		case Adultery_ParamIndices::Adultery_LoopStart: return loopStart;
		case Adultery_ParamIndices::Adultery_LoopLength: return loopLength;
		case Adultery_ParamIndices::Adultery_InterpolationMode: return (float)interpolationMode / (float)((int)InterpolationMode::InterpolationMode_NumInterpolationModes - 1);

		case Adultery_ParamIndices::Adultery_CoarseTune: return coarseTune;
		case Adultery_ParamIndices::Adultery_FineTune: return fineTune;

		case Adultery_ParamIndices::Adultery_FilterType: return Helpers::StateVariableFilterTypeToParam(filterType);
		case Adultery_ParamIndices::Adultery_FilterFreq: return Helpers::FrequencyToParam(filterFreq);
		case Adultery_ParamIndices::Adultery_FilterResonance: return 1.0f - filterResonance;
		case Adultery_ParamIndices::Adultery_FilterModAmt: return filterModAmt;

		case Adultery_ParamIndices::Adultery_ModAttack: return Helpers::EnvValueToScalar(modAttack);
		case Adultery_ParamIndices::Adultery_ModDecay: return Helpers::EnvValueToScalar(modDecay);
		case Adultery_ParamIndices::Adultery_ModSustain: return modSustain;
		case Adultery_ParamIndices::Adultery_ModRelease: return Helpers::EnvValueToScalar(modRelease);

		case Adultery_ParamIndices::Adultery_VoicesUnisono: return Helpers::UnisonoToParam(VoicesUnisono);
		case Adultery_ParamIndices::Adultery_VoicesDetune: return VoicesDetune;
		case Adultery_ParamIndices::Adultery_VoicesPan: return VoicesPan;
		
		case Adultery_ParamIndices::Adultery_VoiceMode: return Helpers::VoiceModeToParam(GetVoiceMode());
		case Adultery_ParamIndices::Adultery_SlideTime: return Slide;

		case Adultery_ParamIndices::Adultery_Master: return masterLevel;
		}
	}

	Adultery::AdulteryVoice::AdulteryVoice(Adultery *adultery)
	{
		this->adultery = adultery;
	}

	SynthDevice *Adultery::AdulteryVoice::SynthDevice() const
	{
		return adultery;
	}

	void Adultery::AdulteryVoice::Run(double songPosition, float **outputs, int numSamples)
	{
		filter.SetType(adultery->filterType);
		filter.SetQ(adultery->filterResonance);

		samplePlayer.SampleStart = adultery->sampleStart;
		samplePlayer.LoopStart = adultery->loopStart;
		samplePlayer.LoopLength = adultery->loopLength;
		samplePlayer.LoopMode = adultery->loopMode;
		samplePlayer.LoopBoundaryMode = adultery->loopBoundaryMode;
		samplePlayer.InterpolationMode = adultery->interpolationMode;
		samplePlayer.Reverse = adultery->reverse;

		samplePlayer.RunPrep();

		float amp = Helpers::VolumeToScalar(adultery->masterLevel);
		float panLeft = Helpers::PanToScalarLeft(Pan);
		float panRight = Helpers::PanToScalarRight(Pan);

		for (int i = 0; i < numSamples; i++)
		{
			calcPitch();

			filter.SetFreq(Helpers::Clamp(adultery->filterFreq + modEnv.GetValue() * (20000.0f - 20.0f) * (adultery->filterModAmt * 2.0f - 1.0f), 0.0f, 20000.0f - 20.0f));

			float sample = samplePlayer.Next();
			if (!samplePlayer.IsActive)
			{
				IsOn = false;
				break;
			}

			sample = filter.Next(sample) * ampEnv.GetValue() * velocity * amp;
			outputs[0][i] += sample * panLeft;
			outputs[1][i] += sample * panRight;

			modEnv.Next();
			ampEnv.Next();
			if (ampEnv.State == EnvelopeState::EnvelopeState_Finished)
			{
				IsOn = false;
				break;
			}
		}
	}

	void Adultery::AdulteryVoice::NoteOn(int note, int velocity, float detune, float pan)
	{
		Voice::NoteOn(note, velocity, detune, pan);

		ampEnv.Attack = adultery->ampAttack;
		ampEnv.Decay = adultery->ampDecay;
		ampEnv.Sustain = adultery->ampSustain;
		ampEnv.Release = adultery->ampRelease;
		ampEnv.Trigger();

		modEnv.Attack = adultery->modAttack;
		modEnv.Decay = adultery->modDecay;
		modEnv.Sustain = adultery->modSustain;
		modEnv.Release = adultery->modRelease;
		modEnv.Trigger();
		
		samplePlayer.SampleData = adultery->sampleData;
		samplePlayer.SampleLength = adultery->sampleLength;
		samplePlayer.SampleLoopStart = adultery->sampleLoopStart;
		samplePlayer.SampleLoopLength = adultery->sampleLoopLength;

		samplePlayer.SampleStart = adultery->sampleStart;
		samplePlayer.LoopStart = adultery->loopStart;
		samplePlayer.LoopLength = adultery->loopLength;
		samplePlayer.LoopMode = adultery->loopMode;
		samplePlayer.LoopBoundaryMode = adultery->loopBoundaryMode;
		samplePlayer.InterpolationMode = adultery->interpolationMode;
		samplePlayer.Reverse = adultery->reverse;

		calcPitch();
		samplePlayer.InitPos();

		this->velocity = (float)velocity / 128.0f;
	}

	void Adultery::AdulteryVoice::NoteOff()
	{
		ampEnv.Off();
		modEnv.Off();
	}

	double Adultery::AdulteryVoice::coarseDetune(float detune)
	{
		return floor((detune * 2.0f - 1.0f) * 12.0f);
	}

	void Adultery::AdulteryVoice::calcPitch()
	{
		samplePlayer.CalcPitch(GetNote() - 60 + Detune + adultery->fineTune * 2.0f - 1.0f + AdulteryVoice::coarseDetune(adultery->coarseTune));
	}
}
