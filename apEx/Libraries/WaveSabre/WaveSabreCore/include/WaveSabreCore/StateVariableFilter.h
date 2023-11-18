#ifndef __WAVESABRECORE_STATEVARIABLEFILTER_H__
#define __WAVESABRECORE_STATEVARIABLEFILTER_H__

namespace WaveSabreCore
{
	enum StateVariableFilterType
	{
		StateVariableFilterType_Lowpass,
		StateVariableFilterType_Highpass,
		StateVariableFilterType_Bandpass,
		StateVariableFilterType_Notch,
	};

	class StateVariableFilter
	{
	public:
		StateVariableFilter();

		float Next(float input);

		void SetType(StateVariableFilterType type);
		void SetFreq(float freq);
		void SetQ(float q);

	private:
		float run(float input);

		bool recalculate;

		StateVariableFilterType type;
		float freq;
		float q;

		float lastInput;
		float low, band;

		float f;
	};
}

#endif
