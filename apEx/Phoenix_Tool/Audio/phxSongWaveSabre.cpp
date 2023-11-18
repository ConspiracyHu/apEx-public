#include "BasePCH.h"
#include "phxSongWaveSabre.h"

#include "WaveSabrePlayerLib.h"

/*
#ifdef _DEBUG
#pragma comment(lib,"../libraries/WaveSabre/WaveSabreCore_debug.lib")
#pragma comment(lib,"../libraries/WaveSabre/WaveSabrePlayerLib_debug.lib")
#else
#pragma comment(lib,"../libraries/WaveSabre/WaveSabreCore_release.lib")
#pragma comment(lib,"../libraries/WaveSabre/WaveSabrePlayerLib_release.lib")
#endif
*/

CArray<WaveSabrePlayerLib::SongRenderer::DeviceId> usedDevices;

WaveSabreCore::Device *SongFactory( WaveSabrePlayerLib::SongRenderer::DeviceId id )
{
  usedDevices.AddUnique( id );

  switch ( id )
  {
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Falcon: return new WaveSabreCore::Falcon();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Slaughter: return new WaveSabreCore::Slaughter();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Thunder: return new WaveSabreCore::Thunder();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Scissor: return new WaveSabreCore::Scissor();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Leveller: return new WaveSabreCore::Leveller();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Crusher: return new WaveSabreCore::Crusher();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Echo: return new WaveSabreCore::Echo();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Smasher: return new WaveSabreCore::Smasher();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Chamber: return new WaveSabreCore::Chamber();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Twister: return new WaveSabreCore::Twister();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Cathedral: return new WaveSabreCore::Cathedral();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Adultery: return new WaveSabreCore::Adultery();
  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Specimen: return new WaveSabreCore::Specimen();
  }

  LOG_ERR( "Unknown WaveSabre device %d used in song!", id );
  return nullptr;
}

void CphxSongWaveSabre::OfflineRenderSong( CStreamReader & streamSong, CStreamWriter & streamOut, CArray<TU32> & aMarkers, SOUND_LOADCALLBACK pLoad, void * pUserData )
{
  TU8 * pData = new TU8[ (TS32) streamSong.GetLength() ];
  streamSong.SeekFromStart( 0 );
  streamSong.Read( pData, (TS32) streamSong.GetLength() );

  aMarkers.Flush();

  IGNOREFREEERRORS( true );

  WaveSabrePlayerLib::SongRenderer::Song song = { SongFactory, pData };
  usedDevices.Flush();

  WaveSabrePlayerLib::SongRenderer * songRenderer = new WaveSabrePlayerLib::SongRenderer( &song, 1 );

  int x = 0;
  int sampleRate = songRenderer->GetSampleRate();
  int bpm = songRenderer->GetTempo();
  while ( 1 )
  {
    int pos = x*1000 * 60 / bpm;
    if ( x*60.0 / bpm > songRenderer->GetLength() )
      break;
    aMarkers.Add( pos );
    x++;
  }

  double lengthInSec = songRenderer->GetLength();

  int sampleCount = 10 * 1024;
  WaveSabrePlayerLib::SongRenderer::Sample * buffer = new WaveSabrePlayerLib::SongRenderer::Sample[ sampleCount * 2 ];
  double secondsRendered = 0.0;
  while ( secondsRendered < lengthInSec )
  {
    songRenderer->RenderSamples( buffer, sampleCount * 2 );
    streamOut.Write( buffer, sampleCount * 2 * sizeof( short ) );
    secondsRendered += sampleCount / 44100.0;

    pLoad( pUserData, secondsRendered * 1000, lengthInSec * 1000 );
  }

  IGNOREFREEERRORS( false );

  devices = usedDevices;
  usedDevices.Flush();

  delete songRenderer;
  delete[] buffer;
  delete[] pData;

  LOG_NFO( GenerateHeaderContents().GetPointer() );
}

CString CphxSongWaveSabre::GenerateHeaderContents()
{
  CString result = R"(WaveSabreCore::Device *SongFactory( WaveSabrePlayerLib::SongRenderer::DeviceId id )
{
  switch ( id )
  {
  default: return nullptr;
)";

  for ( TS32 x = 0; x < devices.NumItems(); x++ )
  {
    switch ( devices[ x ] )
    {
    default: continue;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Falcon:    result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Falcon: return new WaveSabreCore::Falcon();\n";       break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Slaughter: result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Slaughter: return new WaveSabreCore::Slaughter();\n"; break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Thunder:   result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Thunder: return new WaveSabreCore::Thunder();\n";     break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Scissor:   result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Scissor: return new WaveSabreCore::Scissor();\n";     break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Leveller:  result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Leveller: return new WaveSabreCore::Leveller();\n";   break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Crusher:   result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Crusher: return new WaveSabreCore::Crusher();\n";     break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Echo:      result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Echo: return new WaveSabreCore::Echo();\n";           break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Smasher:   result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Smasher: return new WaveSabreCore::Smasher();\n";     break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Chamber:   result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Chamber: return new WaveSabreCore::Chamber();\n";     break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Twister:   result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Twister: return new WaveSabreCore::Twister();\n";     break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Cathedral: result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Cathedral: return new WaveSabreCore::Cathedral();\n"; break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Adultery:  result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Adultery: return new WaveSabreCore::Adultery();\n";   break;
    case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Specimen:  result += "  case WaveSabrePlayerLib::SongRenderer::DeviceId::DeviceId_Specimen: return new WaveSabreCore::Specimen();\n";   break;
    }
  }

  result += "  }\n}";

  return result;
}
