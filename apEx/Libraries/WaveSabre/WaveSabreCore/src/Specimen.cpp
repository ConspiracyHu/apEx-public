#include <WaveSabreCore/Specimen.h>
#include <WaveSabreCore/Helpers.h>

#include <math.h>

namespace WaveSabreCore
{
	HACMDRIVERID Specimen::driverId = NULL;

	Specimen::Specimen()
		: SynthDevice(0)
	{
		for (int i = 0; i < maxVoices; i++) voices[i] = new SpecimenVoice(this);

		chunkData = nullptr;

		waveFormatData = nullptr;
		compressedSize = uncompressedSize = 0;
		compressedData = nullptr;
		sampleData = nullptr;

		sampleLength = 0;

		ampAttack = 1.0f;
		ampDecay = 1.0f;
		ampSustain = 1.0f;
		ampRelease = 1.0f;

		sampleStart = 0.0f;
		reverse = false;
		loopMode = LoopMode::LoopMode_Disabled;
		loopBoundaryMode = LoopBoundaryMode::LoopBoundaryMode_Manual;
		loopStart = 0.0f;
		loopLength = 1.0f;
		sampleLoopStart = 0;
		sampleLoopLength = 0;

		interpolationMode = InterpolationMode::InterpolationMode_Linear;

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

	Specimen::~Specimen()
	{
		if (chunkData) delete [] chunkData;
		if (waveFormatData) delete [] waveFormatData;
		if (compressedData) delete [] compressedData;
		if (sampleData) delete [] sampleData;
	}

	void Specimen::SetParam(int index, float value)
	{
		switch ((Specimen_ParamIndices)index)
		{
		case Specimen_ParamIndices::Specimen_AmpAttack: ampAttack = Helpers::ScalarToEnvValue(value); break;
		case Specimen_ParamIndices::Specimen_AmpDecay: ampDecay = Helpers::ScalarToEnvValue(value); break;
		case Specimen_ParamIndices::Specimen_AmpSustain: ampSustain = value; break;
		case Specimen_ParamIndices::Specimen_AmpRelease: ampRelease = Helpers::ScalarToEnvValue(value); break;

		case Specimen_ParamIndices::Specimen_SampleStart: sampleStart = value; break;
		case Specimen_ParamIndices::Specimen_Reverse: reverse = Helpers::ParamToBoolean(value); break;
		case Specimen_ParamIndices::Specimen_LoopMode: loopMode = (LoopMode)(int)(value * (float)((int)LoopMode::LoopMode_NumLoopModes - 1)); break;
		case Specimen_ParamIndices::Specimen_LoopStart: loopStart = value; break;
		case Specimen_ParamIndices::Specimen_LoopLength: loopLength = value; break;

		case Specimen_ParamIndices::Specimen_InterpolationMode: interpolationMode = (InterpolationMode)(int)(value * (float)((int)InterpolationMode::InterpolationMode_NumInterpolationModes - 1)); break;

		case Specimen_ParamIndices::Specimen_CoarseTune: coarseTune = value; break;
		case Specimen_ParamIndices::Specimen_FineTune: fineTune = value; break;

		case Specimen_ParamIndices::Specimen_FilterType: filterType = Helpers::ParamToStateVariableFilterType(value); break;
		case Specimen_ParamIndices::Specimen_FilterFreq: filterFreq = Helpers::ParamToFrequency(value); break;
		case Specimen_ParamIndices::Specimen_FilterResonance: filterResonance = 1.0f - value; break;
		case Specimen_ParamIndices::Specimen_FilterModAmt: filterModAmt = value; break;

		case Specimen_ParamIndices::Specimen_ModAttack: modAttack = Helpers::ScalarToEnvValue(value); break;
		case Specimen_ParamIndices::Specimen_ModDecay: modDecay = Helpers::ScalarToEnvValue(value); break;
		case Specimen_ParamIndices::Specimen_ModSustain: modSustain = value; break;
		case Specimen_ParamIndices::Specimen_ModRelease: modRelease = Helpers::ScalarToEnvValue(value); break;

		case Specimen_ParamIndices::Specimen_VoicesUnisono: VoicesUnisono = Helpers::ParamToUnisono(value); break;
		case Specimen_ParamIndices::Specimen_VoicesDetune: VoicesDetune = value; break;
		case Specimen_ParamIndices::Specimen_VoicesPan: VoicesPan = value; break;

		case Specimen_ParamIndices::Specimen_Master: masterLevel = value; break;

		case Specimen_ParamIndices::Specimen_VoiceMode: SetVoiceMode(Helpers::ParamToVoiceMode(value)); break;
		case Specimen_ParamIndices::Specimen_SlideTime: Slide = value; break;
		}
	}

	float Specimen::GetParam(int index) const
	{
		switch ((Specimen_ParamIndices)index)
		{
		case Specimen_ParamIndices::Specimen_AmpAttack: default: return Helpers::EnvValueToScalar(ampAttack);
		case Specimen_ParamIndices::Specimen_AmpDecay: return Helpers::EnvValueToScalar(ampDecay);
		case Specimen_ParamIndices::Specimen_AmpSustain: return ampSustain;
		case Specimen_ParamIndices::Specimen_AmpRelease: return Helpers::EnvValueToScalar(ampRelease);

		case Specimen_ParamIndices::Specimen_SampleStart: return sampleStart;
		case Specimen_ParamIndices::Specimen_Reverse: return Helpers::BooleanToParam(reverse);
		case Specimen_ParamIndices::Specimen_LoopMode: return (float)loopMode / (float)((int)LoopMode::LoopMode_NumLoopModes - 1);
		case Specimen_ParamIndices::Specimen_LoopStart: return loopStart;
		case Specimen_ParamIndices::Specimen_LoopLength: return loopLength;
		case Specimen_ParamIndices::Specimen_InterpolationMode: return (float)interpolationMode / (float)((int)InterpolationMode::InterpolationMode_NumInterpolationModes - 1);

		case Specimen_ParamIndices::Specimen_CoarseTune: return coarseTune;
		case Specimen_ParamIndices::Specimen_FineTune: return fineTune;

		case Specimen_ParamIndices::Specimen_FilterType: return Helpers::StateVariableFilterTypeToParam(filterType);
		case Specimen_ParamIndices::Specimen_FilterFreq: return Helpers::FrequencyToParam(filterFreq);
		case Specimen_ParamIndices::Specimen_FilterResonance: return 1.0f - filterResonance;
		case Specimen_ParamIndices::Specimen_FilterModAmt: return filterModAmt;

		case Specimen_ParamIndices::Specimen_ModAttack: return Helpers::EnvValueToScalar(modAttack);
		case Specimen_ParamIndices::Specimen_ModDecay: return Helpers::EnvValueToScalar(modDecay);
		case Specimen_ParamIndices::Specimen_ModSustain: return modSustain;
		case Specimen_ParamIndices::Specimen_ModRelease: return Helpers::EnvValueToScalar(modRelease);

		case Specimen_ParamIndices::Specimen_VoicesUnisono: return Helpers::UnisonoToParam(VoicesUnisono);
		case Specimen_ParamIndices::Specimen_VoicesDetune: return VoicesDetune;
		case Specimen_ParamIndices::Specimen_VoicesPan: return VoicesPan;

		case Specimen_ParamIndices::Specimen_Master: return masterLevel;

		case Specimen_ParamIndices::Specimen_VoiceMode: return Helpers::VoiceModeToParam(GetVoiceMode());
		case Specimen_ParamIndices::Specimen_SlideTime: return Slide;
		}
	}

	typedef struct
	{
		int CompressedSize;
		int UncompressedSize;
	} ChunkHeader;

	void Specimen::SetChunk(void *data, int size)
	{
		if (!size) return;

		// Read header
		auto headerPtr = (ChunkHeader *)data;
		auto headerSize = sizeof(ChunkHeader);

		// Read wave format
		auto waveFormatPtr = (WAVEFORMATEX *)((char *)data + headerSize);
		auto waveFormatSize = sizeof(WAVEFORMATEX) + waveFormatPtr->cbSize;

		// Read compressed data and load sample
		auto compressedDataPtr = (char *)waveFormatPtr + waveFormatSize;
		auto compressedDataSize = headerPtr->CompressedSize;
		LoadSample(compressedDataPtr, headerPtr->CompressedSize, headerPtr->UncompressedSize, waveFormatPtr);

		// Read params
		//  The rest of the data from the start of the params until the end of the chunk
		//  should be interpreted as a bunch of float params, minus a single int at the
		//  end, which should match the size of the chunk.
		auto paramDataPtr = compressedDataPtr + compressedDataSize;
		// This may be different than our internal numParams value if this chunk was
		//  saved with an earlier version of the plug for example. It's important we
		//  don't read past the chunk data, so we set as many parameters as the
		//  chunk contains, not the amount of parameters we have available. The
		//  remaining parameters will retain their default values in that case, which
		//  if we've done our job right, shouldn't change the sound with respect to
		//  the parameters we read here.
		auto numChunkParams = (int)((size - sizeof(int) - (paramDataPtr - (char *)data)) / sizeof(float));
		for (int i = 0; i < numChunkParams; i++)
			SetParam(i, ((float *)paramDataPtr)[i]);
	}

	int Specimen::GetChunk(void **data)
	{
		if (!compressedData) return 0;

		// Figure out size of chunk
		//  The names here are meant to be symmetric with those in SetChunk for clarity
		auto headerSize = sizeof(ChunkHeader);
		auto waveFormatSize = sizeof(WAVEFORMATEX) + ((WAVEFORMATEX *)waveFormatData)->cbSize;
		auto compressedDataSize = compressedSize;
		auto paramSize = (int)Specimen_ParamIndices::Specimen_NumParams * sizeof(float);
		auto chunkSizeSize = sizeof(int);
		auto size = headerSize + waveFormatSize + compressedSize + paramSize + chunkSizeSize;

		// (Re)allocate chunk data
		if (chunkData) delete [] chunkData;
		chunkData = new char[size];

		// Write header
		ChunkHeader header;
		header.CompressedSize = compressedSize;
		header.UncompressedSize = uncompressedSize;
		memcpy(chunkData, &header, sizeof(ChunkHeader));

		// Write wave format
		auto waveFormatPtr = (char *)chunkData + headerSize;
		memcpy(waveFormatPtr, waveFormatData, waveFormatSize);

		// Write compressed data
		auto compressedDataPtr = waveFormatPtr + waveFormatSize;
		memcpy(compressedDataPtr, compressedData, compressedDataSize);

		// Write params
		auto paramDataPtr = (float *)(compressedDataPtr + compressedDataSize);
		for (int i = 0; i < (int)Specimen_ParamIndices::Specimen_NumParams; i++)
			paramDataPtr[i] = GetParam(i);

		// Write final chunk size
		auto chunkSizePtr = (int *)((char *)paramDataPtr + paramSize);
		*chunkSizePtr = size;

		*data = chunkData;
		return size;
	}

	void Specimen::LoadSample(char *data, int compressedSize, int uncompressedSize, WAVEFORMATEX *waveFormat)
	{
		this->compressedSize = compressedSize;
		this->uncompressedSize = uncompressedSize;

		if (waveFormatData) delete [] waveFormatData;
		waveFormatData = new char[sizeof(WAVEFORMATEX) + waveFormat->cbSize];
		memcpy(waveFormatData, waveFormat, sizeof(WAVEFORMATEX) + waveFormat->cbSize);
		if (compressedData) delete [] compressedData;
		compressedData = new char[compressedSize];
		memcpy(compressedData, data, compressedSize);

		acmDriverEnum(driverEnumCallback, NULL, NULL);
		HACMDRIVER driver = NULL;
		acmDriverOpen(&driver, driverId, 0);

		WAVEFORMATEX dstWaveFormat =
		{
			WAVE_FORMAT_PCM,
			1,
			waveFormat->nSamplesPerSec,
			waveFormat->nSamplesPerSec * 2,
			sizeof(short),
			sizeof(short) * 8,
			0
		};

		HACMSTREAM stream = NULL;
		acmStreamOpen(&stream, driver, waveFormat, &dstWaveFormat, NULL, NULL, NULL, ACM_STREAMOPENF_NONREALTIME);

		ACMSTREAMHEADER streamHeader;
		memset(&streamHeader, 0, sizeof(ACMSTREAMHEADER));
		streamHeader.cbStruct = sizeof(ACMSTREAMHEADER);
		streamHeader.pbSrc = (LPBYTE)compressedData;
		streamHeader.cbSrcLength = compressedSize;
		auto uncompressedData = new short[uncompressedSize * 2];
		streamHeader.pbDst = (LPBYTE)uncompressedData;
		streamHeader.cbDstLength = uncompressedSize * 2;
		acmStreamPrepareHeader(stream, &streamHeader, 0);

		acmStreamConvert(stream, &streamHeader, 0);
		
		acmStreamClose(stream, 0);
		acmDriverClose(driver, 0);

		sampleLength = streamHeader.cbDstLengthUsed / sizeof(short);
		if (sampleData) delete [] sampleData;
		sampleData = new float[sampleLength];
		for (int i = 0; i < sampleLength; i++) sampleData[i] = (float)((double)uncompressedData[i] / 32768.0);

		sampleLoopStart = 0;
		sampleLoopLength = sampleLength;

		delete [] uncompressedData;
	}

	Specimen::SpecimenVoice::SpecimenVoice(Specimen *specimen)
	{
		this->specimen = specimen;
	}

	SynthDevice *Specimen::SpecimenVoice::SynthDevice() const
	{
		return specimen;
	}

	void Specimen::SpecimenVoice::Run(double songPosition, float **outputs, int numSamples)
	{
		filter.SetType(specimen->filterType);
		filter.SetQ(specimen->filterResonance);

		samplePlayer.SampleStart = specimen->sampleStart;
		samplePlayer.LoopStart = specimen->loopStart;
		samplePlayer.LoopLength = specimen->loopLength;
		samplePlayer.LoopMode = specimen->loopMode;
		samplePlayer.LoopBoundaryMode = specimen->loopBoundaryMode;
		samplePlayer.InterpolationMode = specimen->interpolationMode;
		samplePlayer.Reverse = specimen->reverse;

		samplePlayer.RunPrep();

		float amp = Helpers::VolumeToScalar(specimen->masterLevel);
		float panLeft = Helpers::PanToScalarLeft(Pan);
		float panRight = Helpers::PanToScalarRight(Pan);

		for (int i = 0; i < numSamples; i++)
		{
			calcPitch();

			filter.SetFreq(Helpers::Clamp(specimen->filterFreq + modEnv.GetValue() * (20000.0f - 20.0f) * (specimen->filterModAmt * 2.0f - 1.0f), 0.0f, 20000.0f - 20.0f));

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

	void Specimen::SpecimenVoice::NoteOn(int note, int velocity, float detune, float pan)
	{
		Voice::NoteOn(note, velocity, detune, pan);

		ampEnv.Attack = specimen->ampAttack;
		ampEnv.Decay = specimen->ampDecay;
		ampEnv.Sustain = specimen->ampSustain;
		ampEnv.Release = specimen->ampRelease;
		ampEnv.Trigger();

		modEnv.Attack = specimen->modAttack;
		modEnv.Decay = specimen->modDecay;
		modEnv.Sustain = specimen->modSustain;
		modEnv.Release = specimen->modRelease;
		modEnv.Trigger();

		samplePlayer.SampleData = specimen->sampleData;
		samplePlayer.SampleLength = specimen->sampleLength;
		samplePlayer.SampleLoopStart = specimen->sampleLoopStart;
		samplePlayer.SampleLoopLength = specimen->sampleLoopLength;

		samplePlayer.SampleStart = specimen->sampleStart;
		samplePlayer.LoopStart = specimen->loopStart;
		samplePlayer.LoopLength = specimen->loopLength;
		samplePlayer.LoopMode = specimen->loopMode;
		samplePlayer.LoopBoundaryMode = specimen->loopBoundaryMode;
		samplePlayer.InterpolationMode = specimen->interpolationMode;
		samplePlayer.Reverse = specimen->reverse;

		calcPitch();
		samplePlayer.InitPos();

		this->velocity = (float)velocity / 128.0f;
	}
	
	void Specimen::SpecimenVoice::NoteOff()
	{
		ampEnv.Off();
		modEnv.Off();
	}

	double Specimen::SpecimenVoice::coarseDetune(float detune)
	{
		return floor((detune * 2.0f - 1.0f) * 12.0f);
	}

	void Specimen::SpecimenVoice::calcPitch()
	{
		samplePlayer.CalcPitch(GetNote() - 60 + Detune + specimen->fineTune * 2.0f - 1.0f + SpecimenVoice::coarseDetune(specimen->coarseTune));
	}

	BOOL __stdcall Specimen::driverEnumCallback(HACMDRIVERID driverId, DWORD dwInstance, DWORD fdwSupport)
	{
		if (Specimen::driverId) return 1;

		HACMDRIVER driver = NULL;
		acmDriverOpen(&driver, driverId, 0);

		int waveFormatSize = 0;
		acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, &waveFormatSize);
		auto waveFormat = (WAVEFORMATEX *)(new char[waveFormatSize]);
		memset(waveFormat, 0, waveFormatSize);
		ACMFORMATDETAILS formatDetails;
		memset(&formatDetails, 0, sizeof(formatDetails));
		formatDetails.cbStruct = sizeof(formatDetails);
		formatDetails.pwfx = waveFormat;
		formatDetails.cbwfx = waveFormatSize;
		formatDetails.dwFormatTag = WAVE_FORMAT_UNKNOWN;
		acmFormatEnum(driver, &formatDetails, formatEnumCallback, NULL, NULL);

		delete [] (char *)waveFormat;

		acmDriverClose(driver, 0);

		return 1;
	}

	BOOL __stdcall Specimen::formatEnumCallback(HACMDRIVERID driverId, LPACMFORMATDETAILS formatDetails, DWORD dwInstance, DWORD fdwSupport)
	{
		if (formatDetails->pwfx->wFormatTag == WAVE_FORMAT_GSM610 &&
			formatDetails->pwfx->nChannels == 1 &&
			formatDetails->pwfx->nSamplesPerSec == SampleRate)
		{
			Specimen::driverId = driverId;
		}
		return 1;
	}
}
