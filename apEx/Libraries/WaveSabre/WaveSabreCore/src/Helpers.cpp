#include <WaveSabreCore/Helpers.h>

//#include <inttypes.h>
#define _USE_MATH_DEFINES
#include <math.h>

namespace WaveSabreCore
{
	double Helpers::CurrentSampleRate = 44100.0;
	int Helpers::CurrentTempo = 120;
	int Helpers::RandomSeed = 1;

	static const int fastCosTabLog2Size = 10; // size = 1024
	static const int fastCosTabSize = (1 << fastCosTabLog2Size);
	double Helpers::fastCosTab[fastCosTabSize + 1];

	void Helpers::Init()
	{
		RandomSeed = 1;

		for (int i = 0; i < fastCosTabSize + 1; i++)
		{
			double phase = double(i) * ((M_PI * 2) / fastCosTabSize);
			fastCosTab[i] = cos(phase);
		}
	}

	float Helpers::RandFloat()
	{
		return (float)((RandomSeed *= 0x15a4e35) % 255) / 255.0f;
	}

	double Helpers::Pow(double x, double y)
	{
		return pow(x, y);
	}

	float Helpers::PowF(float x, float y)
	{
    return (float)pow( x, y );
	}

	double Helpers::FastCos(double x)
	{
		x = fabs(x); // cosine is symmetrical around 0, let's get rid of negative values

		// normalize range from 0..2PI to 1..2
		const auto phaseScale = 1.0 / (M_PI * 2);
		auto phase = 1.0 + x * phaseScale;

		auto phaseAsInt = *reinterpret_cast<unsigned long long *>(&phase);
		int exponent = (phaseAsInt >> 52) - 1023;

		const auto fractBits = 32 - fastCosTabLog2Size;
		const auto fractScale = 1 << fractBits;
		const auto fractMask = fractScale - 1;

		auto significand = (unsigned int)((phaseAsInt << exponent) >> (52 - 32));
		auto index = significand >> fractBits;
		int fract = significand & fractMask;

		auto left = fastCosTab[index];
		auto right = fastCosTab[index + 1];

		auto fractMix = fract * (1.0 / fractScale);
		return left + (right - left) * fractMix;
	}

	double Helpers::FastSin(double x)
	{
		return FastCos(x - M_PI_2);
	}

	double Helpers::Square135(double phase)
	{
		return FastSin(phase) +
			FastSin(phase * 3.0) / 3.0 +
			FastSin(phase * 5.0) / 5.0;
	}

	double Helpers::Square35(double phase)
	{
		return FastSin(phase * 3.0) / 3.0 +
			FastSin(phase * 5.0) / 5.0;
	}

	float Helpers::Mix(float v1, float v2, float mix)
	{
		return v1 * (1.0f - mix) + v2 * mix;
	}

	float Helpers::Clamp(float f, float min, float max)
	{
		if (f < min) return min;
		if (f > max) return max;
		return f;
	}

	double Helpers::NoteToFreq(double note)
	{
		return 440.0 * Pow(2.0, (note - 69.0) / 12.0);
	}

	float Helpers::DbToScalar(float db)
	{
		return PowF(2.0f, db / 6.0f);
	}

	float Helpers::EnvValueToScalar(float value)
	{
		return sqrtf((value - 1.0f) / 5000.0f);
	}

	float Helpers::ScalarToEnvValue(float scalar)
	{
		return scalar * scalar * 5000.0f + 1.0f;
	}

	float Helpers::VolumeToScalar(float volume)
	{
		float v = volume * .4f;
		return v * v;
	}

	float Helpers::ScalarToVolume(float scalar)
	{
		return sqrtf(scalar) / .4f;
	}

	bool Helpers::ParamToBoolean(float value)
	{
		return value >= .5f;
	}

	float Helpers::BooleanToParam(bool b)
	{
		return b ? 1.0f : 0.0f;
	}

	float Helpers::ParamToFrequency(float param)
	{
		return 20.0f + (20000.0f - 20.0f) * param * param;
	}

	float Helpers::FrequencyToParam(float freq)
	{
		return sqrtf((freq - 20.0f) / (20000.0f - 20.0f));
	}

	float Helpers::ParamToQ(float param)
	{
		if (param < .5f)
		{
			return param / .5f * (1.0f - .33f) + .33f;
		}
		else
		{
			return (param - .5f) / .5f * 11.0f + 1.0f;
		}
	}

	float Helpers::QToParam(float q)
	{
		if (q < 1.0f)
		{
			return (q - .33f) / (1.0f - .33f) * .5f;
		}
		else
		{
			return (q - 1.0f) / 11.0f * .5f + .5f;
		}
	}

	float Helpers::ParamToDb(float param, float range)
	{
		return (param * 2.0f - 1.0f) * range;
	}

	float Helpers::DbToParam(float db, float range)
	{
		return (db / range + 1.0f) / 2.0f;
	}

	float Helpers::ParamToResonance(float param)
	{
		return param * .99f + .01f;
	}

	float Helpers::ResonanceToParam(float resonance)
	{
		return (resonance - .01f) / .99f;
	}

	StateVariableFilterType Helpers::ParamToStateVariableFilterType(float param)
	{
		return (StateVariableFilterType)(int)(param * 3.0f);
	}

	float Helpers::StateVariableFilterTypeToParam(StateVariableFilterType type)
	{
		return (float)type / 3.0f;
	}

	int Helpers::ParamToUnisono(float param)
	{
		return (int)(param * 15.0f) + 1;
	}

	float Helpers::UnisonoToParam(int unisono)
	{
		return (float)(unisono - 1) / 15.0f;
	}

	double Helpers::ParamToVibratoFreq(float param)
	{
		return (Pow((double)param, 2.0) + .1) * 70.0;
	}

	float Helpers::VibratoFreqToParam(double vf)
	{
		double d = vf / 70.0 - .1;
		return d >= 0.0 ? (float)sqrt(d) : 0.0f;
	}

	float Helpers::PanToScalarLeft(float pan)
	{
		return sqrtf(1.0f - pan);
	}

	float Helpers::PanToScalarRight(float pan)
	{
		return sqrtf(pan);
	}

	Spread Helpers::ParamToSpread(float param)
	{
		return (Spread)(int)(param * 2.0f);
	}
	
	float Helpers::SpreadToParam(Spread spread)
	{
		return (float)spread / 2.0f;
	}

	VoiceMode Helpers::ParamToVoiceMode(float param)
	{
		return (VoiceMode)(int)(param * 1.0f);
	}

	float Helpers::VoiceModeToParam(VoiceMode voiceMode)
	{
		return (float)voiceMode / 1.0f;
	}
}
