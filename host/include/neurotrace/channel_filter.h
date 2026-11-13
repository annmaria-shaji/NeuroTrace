#pragma once
#include "neurotrace/biquad.h"

namespace nt {

// EEG conditioning chain per channel: 0.5 Hz high-pass -> 45 Hz low-pass -> 50 Hz notch.
class ChannelFilter {
 public:
  explicit ChannelFilter(double fs, double mains = 50.0)
      : hp_(Biquad::highpass(fs, 0.5)), lp_(Biquad::lowpass(fs, 45.0)), notch_(Biquad::notch(fs, mains)) {}

  float process(float x) { return notch_.process(lp_.process(hp_.process(x))); }
  void reset() { hp_.reset(); lp_.reset(); notch_.reset(); }

 private:
  Biquad hp_, lp_, notch_;
};

}  // namespace nt
