// Second-order IIR sections (RBJ audio-EQ-cookbook formulas), Direct Form II Transposed.
#pragma once
#include <cmath>

namespace nt {

class Biquad {
 public:
  static Biquad lowpass(double fs, double fc, double q = 0.70710678) {
    const Prep p = prep(fs, fc, q);
    return Biquad((1 - p.c) / 2, 1 - p.c, (1 - p.c) / 2, 1 + p.alpha, -2 * p.c, 1 - p.alpha);
  }
  static Biquad highpass(double fs, double fc, double q = 0.70710678) {
    const Prep p = prep(fs, fc, q);
    return Biquad((1 + p.c) / 2, -(1 + p.c), (1 + p.c) / 2, 1 + p.alpha, -2 * p.c, 1 - p.alpha);
  }
  static Biquad notch(double fs, double f0, double q = 30.0) {
    const Prep p = prep(fs, f0, q);
    return Biquad(1, -2 * p.c, 1, 1 + p.alpha, -2 * p.c, 1 - p.alpha);
  }

  float process(float x) {
    const double y = b0_ * x + z1_;
    z1_ = b1_ * x - a1_ * y + z2_;
    z2_ = b2_ * x - a2_ * y;
    return static_cast<float>(y);
  }
  void reset() { z1_ = z2_ = 0; }

 private:
  struct Prep { double c, alpha; };
  static Prep prep(double fs, double f, double q) {
    const double w0 = 2.0 * M_PI * f / fs;
    return {std::cos(w0), std::sin(w0) / (2.0 * q)};
  }
  Biquad(double b0, double b1, double b2, double a0, double a1, double a2)
      : b0_(b0 / a0), b1_(b1 / a0), b2_(b2 / a0), a1_(a1 / a0), a2_(a2 / a0) {}

  double b0_, b1_, b2_, a1_, a2_;
  double z1_ = 0, z2_ = 0;
};

}  // namespace nt
