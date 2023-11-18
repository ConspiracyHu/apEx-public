#ifndef __WAVESABREPLAYERLIB_SONGRENDERER_H__
#define __WAVESABREPLAYERLIB_SONGRENDERER_H__

#include "CriticalSection.h"

#include <WaveSabreCore.h>

namespace WaveSabrePlayerLib
{
	class SongRenderer
	{
	public:
		enum DeviceId
		{
			DeviceId_Falcon,
			DeviceId_Slaughter,
			DeviceId_Thunder,
			DeviceId_Scissor,
			DeviceId_Leveller,
			DeviceId_Crusher,
			DeviceId_Echo,
			DeviceId_Smasher,
			DeviceId_Chamber,
			DeviceId_Twister,
			DeviceId_Cathedral,
			DeviceId_Adultery,
			DeviceId_Specimen
		};

		typedef WaveSabreCore::Device *(*DeviceFactory)(DeviceId);

		typedef struct {
			DeviceFactory factory;
			const unsigned char *blob;
		} Song;

		typedef short Sample;

		static const int NumChannels = 2;
		static const int BitsPerSample = 16;
		static const int BlockAlign = NumChannels * BitsPerSample / 8;

		SongRenderer(const SongRenderer::Song *song, int numRenderThreads);
		~SongRenderer();

		void RenderSamples(Sample *buffer, int numSamples);

		int GetTempo() const;
		int GetSampleRate() const;
		double GetLength() const;

	private:
		enum EventType
		{
			EventType_NoteOn,
			EventType_NoteOff,
		};

		typedef struct
		{
			int TimeStamp;
			EventType Type;
			int Note, Velocity;
		} Event;

		class Devices
		{
		public:
			int numDevices;
			WaveSabreCore::Device **devices;
		};

		class MidiLane
		{
		public:
			int numEvents;
			Event *events;
		};

		class Track
		{
		public:
			typedef struct
			{
				int SendingTrackIndex;
				int ReceivingChannelIndex;
				float Volume;
			} Receive;

			Track(SongRenderer *songRenderer, DeviceFactory factory);
			~Track();
			
			void Run(int numSamples);

		private:
			static const int numBuffers = 4;
		public:
			float *Buffers[numBuffers];

			int NumReceives;
			Receive *Receives;

		private:
			class Automation
			{
			public:
				Automation(SongRenderer *songRenderer, WaveSabreCore::Device *device);
				~Automation();

				void Run(int numSamples);

			private:
				typedef struct
				{
					int TimeStamp;
					float Value;
				} Point;

				WaveSabreCore::Device *device;
				int paramId;

				int numPoints;
				Point *points;

				int samplePos;
				int pointIndex;
			};

			SongRenderer *songRenderer;

			float volume;

			int numDevices;
			int *devicesIndicies;

			int midiLaneId;

			int numAutomations;
			Automation **automations;

			int lastSamplePos;
			int accumEventTimestamp;
			int eventIndex;
		};

		enum TrackRenderState : unsigned int
		{
			TrackRenderState_Idle,
			TrackRenderState_Rendering,
			TrackRenderState_Finished,
		};

		typedef struct
		{
			SongRenderer *songRenderer;
			int renderThreadIndex;
		} RenderThreadData;

		static DWORD WINAPI renderThreadProc(LPVOID lpParameter);

		bool renderThreadWork(int renderThreadIndex);

		// TODO: Templatize? Might actually be bigger..
		unsigned char readByte();
		int readInt();
		float readFloat();
		double readDouble();

		const unsigned char *songBlobPtr;
		int songDataIndex;

		int bpm;
		int sampleRate;
		double length;
	
		int numDevices;
		WaveSabreCore::Device **devices;

		int numMidiLanes;
		MidiLane **midiLanes;

		int numTracks;
		Track **tracks;
		TrackRenderState *trackRenderStates;

		int numRenderThreads;
		HANDLE *additionalRenderThreads;

		bool renderThreadShutdown;
		int renderThreadNumFloatSamples;
		unsigned int renderThreadsRunning;
		HANDLE *renderThreadStartEvents;
		HANDLE renderDoneEvent;
	};
}

#endif
