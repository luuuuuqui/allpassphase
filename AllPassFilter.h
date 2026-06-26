#ifndef ALLPASSFILTER_H
#define ALLPASSFILTER_H

// code referenced:
// https://www.musicdsp.org/en/latest/Filters/197-rbj-audio-eq-cookbook.html
// https://www.musicdsp.org/en/latest/Filters/64-biquad-c-code.html
// https://www.musicdsp.org/en/latest/Filters/266-4th-order-linkwitz-riley-filters.html

class AllPassFilter {
  private:
	// coefficient calculations need to be in double
	// for filter stability at low frequencies
	struct filterCoefficients {
		double c0, c1, c2, c3, c4;
	} co;

	struct {
		double xm1 = 0.0;
		double xm2 = 0.0;
		double ym1 = 0.0, ym2 = 0.0;
	} temp;

	float freq = 100.0F;
	float q;

  public:
	void setup(float frequency, float sr, float resonance);
	void copyCoefficientsFrom(const AllPassFilter& filter);
	void zeroBuffers();
	void processBlock(const float* in, float* out, int numSamples);
	float getFreq() const { return freq; }
	float getQ() const { return q; }
};

#endif
