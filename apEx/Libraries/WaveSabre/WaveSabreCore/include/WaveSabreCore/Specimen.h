#ifndef __WAVESABRECORE_SPECIMEN_H__
#define __WAVESABRECORE_SPECIMEN_H__

#include "SynthDevice.h"
#include "Envelope.h"
#include "StateVariableFilter.h"
#include "SamplePlayer.h"

#include <Windows.h>
#include <mmreg.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <MSAcm.h>

namespace WaveSabreCore
{
	class Specimen : public SynthDevice
	{
	public:
		enum Specimen_ParamIndices
		{
			Specimen_AmpAttack,
			Specimen_AmpDecay,
			Specimen_AmpSustain,
			Specimen_AmpRelease,

			Specimen_SampleStart,
			Specimen_LoopMode,
			Specimen_LoopStart,
			Specimen_LoopLength,
			Specimen_Reverse,

			Specimen_InterpolationMode,

			Specimen_CoarseTune,
      Specimen_FineTune,

			Specimen_FilterType,
			Specimen_FilterFreq,
			Specimen_FilterResonance,
			Specimen_FilterModAmt,

			Specimen_ModAttack,
			Specimen_ModDecay,
			Specimen_ModSustain,
			Specimen_ModRelease,

			Specimen_VoicesUnisono,
			Specimen_VoicesDetune,
			Specimen_VoicesPan,

			Specimen_Master,

			Specimen_VoiceMode,
			Specimen_SlideTime,

			Specimen_NumParams,
		};

		static const int SampleRate = 44100;

		Specimen();
		virtual ~Specimen();

		virtual void SetParam(int index, float value);
		virtual float GetParam(int index) const;

		virtual void SetChunk(void *data, int size);
		virtual int GetChunk(void **data);

		void LoadSample(char *data, int compressedSize, int uncompressedSize, WAVEFORMATEX *waveFormat);

	private:
		class SpecimenVoice : public Voice
		{
		public:
			SpecimenVoice(Specimen *thunder);
			virtual WaveSabreCore::SynthDevice *SynthDevice() const;

			virtual void Run(double songPosition, float **outputs, int numSamples);

			virtual void NoteOn(int note, int velocity, float detune, float pan);
			virtual void NoteOff();

		private:
			double coarseDetune(float detune);
			void calcPitch();

			Specimen *specimen;

			Envelope ampEnv, modEnv;

			SamplePlayer samplePlayer;

			StateVariableFilter filter;

			float velocity;
		};

		static BOOL __stdcall driverEnumCallback(HACMDRIVERID driverId, DWORD dwInstance, DWORD fdwSupport);
		static BOOL __stdcall formatEnumCallback(HACMDRIVERID driverId, LPACMFORMATDETAILS formatDetails, DWORD dwInstance, DWORD fdwSupport);

		static HACMDRIVERID driverId;

		char *chunkData;

		char *waveFormatData;
		int compressedSize, uncompressedSize;

		char *compressedData;
		float *sampleData;

		float ampAttack, ampDecay, ampSustain, ampRelease;
		float sampleStart;
		bool reverse;
		LoopMode loopMode;
		LoopBoundaryMode loopBoundaryMode;
		float loopStart, loopLength;

		InterpolationMode interpolationMode;

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
