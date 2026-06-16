//-------------------------------------------------------------------------------------------------------
// $Date: 2020/05/24 00:00:00 $
//
// Filename     : AllPassPhase.cpp
// Created by   : enummusic
// Description  : Crossover filter phase dispersion
//
// (c) 2020 enummusic
// VST SDK (c) 2005, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "AllPassPhase.h"

namespace {
float clampParameter(float value)
{
	if (value < 0.0f)
		return 0.0f;
	if (value > 1.0f)
		return 1.0f;
	return value;
}
}

//-----------------------------------------------------------------------------
AllPassPhaseProgram::AllPassPhaseProgram ()
{
	// default Program Values
	fFrequency = 0.5131f;
	fIterations = 0.5f;
	fQ = 0.5f;
	fMix = 1.0f;

	vst_strncpy (name, "Init", sizeof (name));
}

//-----------------------------------------------------------------------------
AllPassPhase::AllPassPhase (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	// init

	tempBufferL = 0;
	tempBufferR = 0;
	filterBufferL = 0;
	filterBufferR = 0;
	bufferSize = 0;

	programs = new AllPassPhaseProgram[numPrograms];
	fFrequency = 0.3675f;
	fQ = 0.5f;
	fIterations = 0.5f;
	fMix = 1.0f;

	if (programs)
		setProgram (0);

	setNumInputs (2);
	setNumOutputs (2);

	setUniqueID ('aphs');

	curIterations = getIterationCount();
	freq = knobToFrequency(fFrequency);
	q = fQ * sqrt(2.0f);
	setupFilters();
	lastfFreq = fFrequency;

	resume ();		// flush buffer
}

//------------------------------------------------------------------------
AllPassPhase::~AllPassPhase ()
{
	releaseBuffers();
	if (programs)
		delete[] programs;
}

//------------------------------------------------------------------------
void AllPassPhase::setProgram (long program)
{
	if (program < 0 || program >= kNumPrograms)
		return;

	AllPassPhaseProgram* ap = &programs[program];

	curProgram = program;
	setParameter (kFrequency, ap->fFrequency);
	setParameter (kQ, ap->fQ);
	setParameter (kIterations, ap->fIterations);
	setParameter (kMix, ap->fMix);
}

//------------------------------------------------------------------------
void AllPassPhase::setProgramName (char *name)
{
	vst_strncpy (programs[curProgram].name, name, sizeof (programs[curProgram].name));
}

//------------------------------------------------------------------------
void AllPassPhase::getProgramName (char *name)
{
	if (!strcmp (programs[curProgram].name, "Init"))
		sprintf (name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		vst_strncpy (name, programs[curProgram].name, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
bool AllPassPhase::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index >= 0 && index < kNumPrograms)
	{
		vst_strncpy (text, programs[index].name, kVstMaxProgNameLen);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void AllPassPhase::resume ()
{
	if (tempBufferL)
		memset (tempBufferL, 0, bufferSize * sizeof (float));
	if (tempBufferR)
		memset (tempBufferR, 0, bufferSize * sizeof (float));
	if (filterBufferL)
		memset (filterBufferL, 0, bufferSize * sizeof (float));
	if (filterBufferR)
		memset (filterBufferR, 0, bufferSize * sizeof (float));

	for (int i = 0; i < kMaxFilters; i++) {
		filterL[i].zeroBuffers();
		filterR[i].zeroBuffers();
	}

	samplesSinceSilence = deactivateAfterSamples;
	AudioEffectX::resume ();
}

void AllPassPhase::setSampleRate (float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);
	setupFilters();
}

//------------------------------------------------------------------------
void AllPassPhase::setParameter (VstInt32 index, float value)
{
	AllPassPhaseProgram* ap = &programs[curProgram];
	value = clampParameter(value);

	switch (index)
	{
		case kFrequency:
			fFrequency = ap->fFrequency = value;
			setupFilters();
			lastfFreq = fFrequency;
			break;
		case kQ:
			fQ = ap->fQ = value;
			setupFilters();
			break;
		case kIterations:
			fIterations = ap->fIterations = value;
			break;
		case kMix:
			fMix = ap->fMix = value;
			break;
	}
}

void AllPassPhase::setupFilters()
{
	freq = knobToFrequency(fFrequency);
	float sampleRate = getSampleRate();
	if (sampleRate <= 0.0f)
		sampleRate = 44100.0f;
	if (freq >= sampleRate * 0.5f)
		freq = (int)(sampleRate * 0.49f);

	q = fQ * sqrt(2.0f);
	if (q <= 0.005f)
		q = 0.005f;

	const bool resetFilterState = fabs(fFrequency - lastfFreq) > fFrequency / 10 && freq < 500;

	filterL[0].setup(freq, sampleRate, q);
	filterR[0].setup(freq, sampleRate, q);
	if (resetFilterState) {
		filterL[0].zeroBuffers();
		filterR[0].zeroBuffers();
	}

	for (int i = 1; i < getIterationCount(); i++) {
		filterL[i].copyCoefficientsFrom(filterL[0]);
		filterR[i].copyCoefficientsFrom(filterR[0]);

		if (resetFilterState) {
			filterL[i].zeroBuffers();
			filterR[i].zeroBuffers();
		}
	}
}

//------------------------------------------------------------------------
float AllPassPhase::getParameter (VstInt32 index)
{
	float v = 0;

	switch (index)
	{
		case kFrequency:
			v = fFrequency;
			break;
		case kQ:
			v = fQ;
			break;
		case kIterations:
			v = fIterations;
			break;
		case kMix:
			v = fMix;
			break;
	}
	return v;
}

//------------------------------------------------------------------------
void AllPassPhase::getParameterName (VstInt32 index, char *label)
{
	switch (index)
	{
		case kFrequency:
			vst_strncpy (label, "Frequency", kVstMaxParamStrLen);
			break;
		case kQ:
			vst_strncpy (label, "Q", kVstMaxParamStrLen);
			break;
		case kIterations:
			vst_strncpy (label, "Intensity", kVstMaxParamStrLen);
			break;
		case kMix:
			vst_strncpy(label, "Mix", kVstMaxParamStrLen);
			break;
	}
}

void AllPassPhase::ensureBufferSize(VstInt32 sampleFrames)
{
	if (sampleFrames <= bufferSize)
		return;

	releaseBuffers();

	tempBufferL = new float[sampleFrames] {};
	tempBufferR = new float[sampleFrames] {};
	filterBufferL = new float[sampleFrames] {};
	filterBufferR = new float[sampleFrames] {};
	bufferSize = sampleFrames;
}

void AllPassPhase::releaseBuffers()
{
	if (tempBufferL)
		delete[] tempBufferL;
	if (tempBufferR)
		delete[] tempBufferR;
	if (filterBufferL)
		delete[] filterBufferL;
	if (filterBufferR)
		delete[] filterBufferR;

	tempBufferL = 0;
	tempBufferR = 0;
	filterBufferL = 0;
	filterBufferR = 0;
	bufferSize = 0;
}

// https://www.musicdsp.org/en/latest/Other/260-exponential-curve-for.html
int AllPassPhase::knobToFrequency(float x)
{
	return floor(exp((16 + x * 100 * 1.20103)*log(1.059))*8.17742);
}

int AllPassPhase::getIterationCount() const
{
	const int iterations = (int)(fIterations * kMaxFilters);

	if (iterations < 0)
		return 0;
	if (iterations > kMaxFilters)
		return kMaxFilters;
	return iterations;
}

//------------------------------------------------------------------------
void AllPassPhase::getParameterDisplay (VstInt32 index, char *text)
{
	switch (index)
	{
		case kFrequency:
			int2string(knobToFrequency(fFrequency), text, 5);
			break;
		case kQ:
			float2string(fQ * sqrt(2.0f), text, 4);
			break;
		case kIterations:
			if (getIterationCount() == 0) {
				vst_strncpy(text, "BYPASS", kVstMaxParamStrLen);
			}
			else {
				int2string(getIterationCount(), text, 5);
			}
			break;
		case kMix:
			if (fMix == 0) {
				vst_strncpy(text, "0", kVstMaxParamStrLen);
			}
			else {
				int2string(fMix * 100, text, 3);
			}
			break;
	}
}

//------------------------------------------------------------------------
void AllPassPhase::getParameterLabel (VstInt32 index, char *label)
{
	switch (index)
	{
		case kFrequency:
			vst_strncpy (label, "Hz", kVstMaxParamStrLen);
			break;
		case kQ:
			vst_strncpy(label, " ", kVstMaxParamStrLen);
			break;
		case kIterations:
			vst_strncpy (label, "iterations", kVstMaxParamStrLen);
			break;
		case kMix:
			vst_strncpy(label, "%", kVstMaxParamStrLen);
			break;
	}
}

//------------------------------------------------------------------------
bool AllPassPhase::getEffectName (char* name)
{
	vst_strncpy (name, "AllPassPhase", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool AllPassPhase::getProductString (char* text)
{
	vst_strncpy (text, "AllPassPhase", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool AllPassPhase::getVendorString (char* text)
{
	vst_strncpy (text, "enummusic", kVstMaxVendorStrLen);
	return true;
}

//---------------------------------------------------------------------------
void AllPassPhase::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	if (sampleFrames <= 0)
		return;

	const int targetIterations = getIterationCount();

	if (targetIterations > curIterations) {
		if (curIterations > 0) {
			for (int i = curIterations; i < targetIterations; i++) {
				filterL[i].copyCoefficientsFrom(filterL[curIterations - 1]);
				filterR[i].copyCoefficientsFrom(filterR[curIterations - 1]);
			}
		}
		else {
			setupFilters();
		}
		curIterations = targetIterations;
	}
	else if (targetIterations < curIterations) {
		curIterations = targetIterations;
	}

	float* in1 = inputs[0];
	float* in2 = inputs[1];
	float* out1 = outputs[0];
	float* out2 = outputs[1];

	if (curIterations == 0 || fMix <= 0) {
		int samples = sampleFrames;
		while (--samples >= 0) {
			*out1++ = *in1++;
			*out2++ = *in2++;
		}
		return;
	}

	ensureBufferSize(sampleFrames);

	// copy input to temp array
	for (int i = 0; i < sampleFrames; i++) {
		tempBufferL[i] = in1[i];
		tempBufferR[i] = in2[i];
		// checks the whole buffer
		// if it sees anything that isn't silence, reset the silence counter
		// ensures the entire buffer is processed
		if (fabs(tempBufferL[i]) >= noiseFloor || fabs(tempBufferR[i]) >= noiseFloor) {
			samplesSinceSilence = 0;
		}
	}

	// filter the audio
	if (samplesSinceSilence < deactivateAfterSamples) {
		for (int i = 0; i < curIterations; i++) {
			filterL[i].processBlock(tempBufferL, filterBufferL, sampleFrames);
			filterR[i].processBlock(tempBufferR, filterBufferR, sampleFrames);
			int samples = sampleFrames;
			float *temp1 = tempBufferL;
			float *temp2 = tempBufferR;
			float *left = filterBufferL;
			float *right = filterBufferR;
			while (--samples >= 0) {
				*temp1++ = *left++;
				*temp2++ = *right++;
			}
		}
	}

	const float dryMix = 1.0f - fMix;
	int samples = sampleFrames;
	float *temp1 = tempBufferL;
	float *temp2 = tempBufferR;

	while (--samples >= 0) {

		// if it sees anything that isn't silence, reset the silence counter
		if (fabs(*temp1) >= noiseFloor || fabs(*temp2) >= noiseFloor) {
			samplesSinceSilence = 0;
		}
		else if (samplesSinceSilence < 32768) { // int overflow protection
			samplesSinceSilence++;
		}

		*out1 = (*temp1 * fMix + *in1 * dryMix);
		*out2 = (*temp2 * fMix + *in2 * dryMix);

		in1++;
		in2++;
		out1++;
		out2++;
		temp1++;
		temp2++;
	}

}
